#include "include/setmode.h"

static void setup_protected_mode(struct kvm_sregs *sregs)
{
	struct kvm_segment seg = {
		.base = 0,
		.limit = 0xffffffff,
		.selector = 1 << 3,
		.present = 1,
		.type = 11, /* Code: execute, read, accessed */
		.dpl = 0,
		.db = 1,
		.s = 1, /* Code/data */
		.l = 0,
		.g = 1, /* 4KB granularity */
	};

	sregs->cr0 |= CR0_PE; /* enter protected mode */

	sregs->cs = seg;

	seg.type = 3; /* Data: read/write, accessed */
	seg.selector = 2 << 3;
	sregs->ds = sregs->es = sregs->fs = sregs->gs = sregs->ss = seg;
}

extern const unsigned char guest32[], guest32_end[];

int run_protected_mode(X86CPU *vcpu)
{
	struct kvm_sregs sregs;
	struct kvm_regs regs;

    if (ioctl(vcpu->vcpu_fd, KVM_GET_SREGS, &sregs) < 0) {
		perror("KVM_GET_SREGS");
		goto err;
	}

	setup_protected_mode(&sregs);

    if (ioctl(vcpu->vcpu_fd, KVM_SET_SREGS, &sregs) < 0) {
		perror("KVM_SET_SREGS");
		goto err;
	}

	memset(&regs, 0, sizeof(regs));
	/* Clear all FLAGS bits, except bit 1 which is always set. */
	regs.rflags = 2;
	regs.rip = 0;

	if (ioctl(vcpu->vcpu_fd, KVM_SET_REGS, &regs) < 0) {
		perror("KVM_SET_REGS");
		goto err;
	}

    memset(vcpu->mem->ram, 0, vcpu->mem->memory_size);
	memcpy(vcpu->mem->ram, guest32, guest32_end-guest32);

    return 0;

err:
    return -1;
}

static void setup_long_mode(X86CPU *vcpu, struct kvm_sregs *sregs)
{
    uint64_t p4_table = 0x2000;
    uint64_t *p4 = (void *)(vcpu->mem->ram + p4_table);

    uint64_t p3_table = 0x3000;
    uint64_t *p3 = (void *)(vcpu->mem->ram + p3_table);

    uint64_t p2_table = 0x4000;
    uint64_t *p2 = (void *)(vcpu->mem->ram + p2_table);

    p4[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | p3_table;
    p3[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | p2_table;

    // 每一页大小为2MB, 填满p2table的每一项，映射前1G的内存
    uint64_t page_size = 0x200000;
    for (int i = 0; i < 512; i++) {
        uint64_t addr = i * page_size;
        p2[i] = PDE64_PRESENT | PDE64_RW | PDE64_USER | PDE64_HP | addr;
    }

    sregs->cr3 = p4_table;
    sregs->cr4 = CR4_PAE;
    sregs->cr0 = CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM |CR0_PG;
    sregs->efer = EFER_LME | EFER_LMA;

    //setup_64bit_code_segment(sregs);
}

int run_long_mode(X86CPU *vcpu)
{
    struct kvm_sregs sregs;
    struct kvm_regs  regs;

    if (ioctl(vcpu->vcpu_fd, KVM_GET_SREGS, &sregs) < 0) {
        perror("KVM_GET_SREGS");
        goto err;
    }

    setup_long_mode(vcpu, &sregs);

    if (ioctl(vcpu->vcpu_fd, KVM_SET_SREGS, &sregs) < 0) {
        perror("KVM_SET_REGS");
        goto err;
    }

    memset(&regs, 0, sizeof(regs));
    regs.rflags = 2;
    regs.rip = 0x3e0;
    regs.rsp = 2 << 20;

    if (ioctl(vcpu->vcpu_fd, KVM_SET_REGS, &regs) < 0) {
        perror("KVM_SET_REGS");
        goto err;
    }

    size_t guest32_size = guest32_end - guest32;
    /*
        本来思路是在payload.o中设置payload32和payload64两个段对应 32bit代码和64bit代码，
        但是由于payload.ld链接时没有自动重定位main64.img.o中的地址（应该加上32bitcode占用的0x3e0），
        而且对链接脚本不太熟，解决不了这个问题。

        所有最后决定直接将链接好的64bit binary直接读入0x3e0开始的内存中。
    */
    //memcpy(&vcpu->mem->ram[guest32_size], guest64, guest64_end - guest64);
    int fd = open("build/obj/main64.img", O_RDONLY);
    if (fd == -1) {
        perror("open img:");
        goto err;
    }

    int datasize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    int rc = read(fd, &vcpu->mem->ram[guest32_size], datasize);

    return 0;

err:
    return -1;
}