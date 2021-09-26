#include "include/vmm.h"
#include "include/setmode.h"

static int kvm_put_sreg(X86CPU *cpu)
{
    struct kvm_sregs sregs;
    if (ioctl(cpu->vcpu_fd, KVM_GET_SREGS, &sregs) < 0) {
        DPRINTF("error: KVM_GET_SREGS failed!\n");
        return -1;
    }
    sregs.cs.base = 0;
    sregs.cs.selector = 0;
    if (ioctl(cpu->vcpu_fd, KVM_SET_SREGS, &sregs) < 0) {
        DPRINTF("error: KVM_SET_SREGS failed!\n");
        return -1;
    }
    return 0;
}

static int kvm_getput_regs(X86CPU *cpu, int set)
{
    if(set) {
        struct kvm_regs regs = {
            .rip = cpu->mem->start_addr,
            .rflags = 0x2,
        };
        if (ioctl(cpu->vcpu_fd, KVM_SET_REGS, &regs) < 0) {
            fprintf(stderr, "KVM_SET_REGS failed\n");
            return -1;
        }
    }

    return 0;
}

static void kvm_segment_format(char *buffer, size_t size, const struct kvm_segment *s)
{
	snprintf(buffer, size
		,"%016llx +%08x sel:%04x t:%02x"
		" p:%u dpl:%u db:%u s:%u l:%u g:%u a:%u"
		,(unsigned long long)s->base
		,(unsigned)s->limit
		,(unsigned)s->selector
		,(unsigned)s->type
		,(unsigned)s->present
		,(unsigned)s->dpl
		,(unsigned)s->db
		,(unsigned)s->s
		,(unsigned)s->l
		,(unsigned)s->g
		,(unsigned)s->avl
	);
}

static void kvm_dtable_format(char *buffer, size_t size, const struct kvm_dtable *dt)
{
	snprintf(buffer, size
		,"base:%016llx limit:%08x"
		,dt->base
		,dt->limit
	);
}

static void dump_vcpu_regs(const struct kvm_regs *r)
{
	printf("rax:%016llx rbx:%016llx\n", r->rax, r->rbx);
	printf("rcx:%016llx rdx:%016llx\n", r->rcx, r->rdx);
	printf("rsi:%016llx rdi:%016llx\n", r->rsi, r->rdi);
	printf("rsp:%016llx rbp:%016llx\n", r->rsp, r->rbp);
	printf("r8 :%016llx r9 :%016llx\n", r->r8, r->r9);
	printf("r10:%016llx r11:%016llx\n", r->r10, r->r11);
	printf("r12:%016llx r13:%016llx\n", r->r12, r->r13);
	printf("r14:%016llx r15:%016llx\n", r->r14, r->r15);
	printf("rip:%016llx rflags:%016llx\n", r->rip, r->rflags);
}

static void dump_vcpu_sregs(const struct kvm_sregs *r)
{
	char buffer[256];

	kvm_segment_format(buffer, sizeof(buffer), &r->cs);
	printf("cs : %s\n", buffer);
	kvm_segment_format(buffer, sizeof(buffer), &r->ds);
	printf("ds : %s\n", buffer);
	kvm_segment_format(buffer, sizeof(buffer), &r->es);
	printf("es : %s\n", buffer);
	kvm_segment_format(buffer, sizeof(buffer), &r->fs);
	printf("fs : %s\n", buffer);
	kvm_segment_format(buffer, sizeof(buffer), &r->gs);
	printf("gs : %s\n", buffer);
	kvm_segment_format(buffer, sizeof(buffer), &r->ss);
	printf("ss : %s\n", buffer);
	kvm_segment_format(buffer, sizeof(buffer), &r->tr);
	printf("tr : %s\n", buffer);
	kvm_segment_format(buffer, sizeof(buffer), &r->ldt);
	printf("ldt: %s\n", buffer);

	kvm_dtable_format(buffer, sizeof(buffer), &r->gdt);
	printf("gdt: %s\n", buffer);
	kvm_dtable_format(buffer, sizeof(buffer), &r->idt);
	printf("idt: %s\n", buffer);

	printf("cr0:%016llx cr2:%016llx\n", r->cr0, r->cr2);
	printf("cr3:%016llx cr4:%016llx\n", r->cr3, r->cr4);
	printf("cr8:%016llx\n", r->cr8);

	printf("efer:%016llx apic_base:%016llx\n", r->efer, r->apic_base);
// r->interrupt_bitmap
}


int kvm_dump_register(const X86CPU *cpu)
{
    int ret = 0;
    struct kvm_regs kvm_regs;
    struct kvm_sregs kvm_sregs;

    memset(&kvm_regs, 0, sizeof(kvm_regs));
    ret = ioctl(cpu->vcpu_fd, KVM_GET_REGS, &kvm_regs);
    if (ret < 0) {
        DPRINTF("dump register error: KVM_GET_REGS failed!\n");
        goto err;
    }
    dump_vcpu_regs(&kvm_regs);

    memset(&kvm_sregs, 0, sizeof(kvm_sregs));
    ret = ioctl(cpu->vcpu_fd, KVM_GET_SREGS, &kvm_sregs);
    if (ret < 0) {
        DPRINTF("dump register error: KVM_GET_REGS failed!\n");
        goto err;
    }
    dump_vcpu_sregs(&kvm_sregs);

    return ret;

err:
    ret = -1;
    return ret;
}

int kvm_set_cpuid(const Vmm *vm)
{   
    struct kvm_cpuid2 *cpuid; 
    int nent = 100;
    int ret = 0;
    unsigned long size = sizeof(*cpuid) + nent * sizeof(*cpuid->entries);

    cpuid = (struct kvm_cpuid2*)malloc(size);
    if (cpuid < 0) {
        DPRINTF("error: alloc struct kvm_cpuid2 failed!\n");
        goto alloc_fail;
    }

    bzero(cpuid, size);
    cpuid->nent = nent;

    ret = ioctl(vm->kvm->kvm_fd, KVM_GET_SUPPORTED_CPUID, cpuid);
    if (ret < 0) {
        DPRINTF("error: KVM_GET_SUPPORTED_CPUID failed!\n");
        goto ioctl_err;
    }

    for (int i = 0; i < cpuid->nent; i++) {
        if (cpuid->entries[i].function == 0x80000002)
            __get_cpuid(0x80000002, &cpuid->entries[i].eax, &cpuid->entries[i].ebx,
                           &cpuid->entries[i].ecx, &cpuid->entries[i].edx);
        if (cpuid->entries[i].function == 0x80000003)
            __get_cpuid(0x80000003, &cpuid->entries[i].eax, &cpuid->entries[i].ebx,
                           &cpuid->entries[i].ecx, &cpuid->entries[i].edx);
        if (cpuid->entries[i].function == 0x80000004)
            __get_cpuid(0x80000004, &cpuid->entries[i].eax, &cpuid->entries[i].ebx,
                           &cpuid->entries[i].ecx, &cpuid->entries[i].edx);  
    }

    ret = ioctl(vm->cpu->vcpu_fd, KVM_SET_CPUID2, cpuid);
    if (ret < 0) {
        DPRINTF("error: KVM_SET_CPUID2!\n");
        goto ioctl_err;
    }

    return 0;

ioctl_err:
    free(cpuid);
alloc_fail:
    ret = -1;
    return ret;
}

static void kvm_handle_io(struct kvm_run *run, X86CPU *cpu)
{
    if (run->io.direction == KVM_EXIT_IO_OUT) {
        switch (run->io.port) {
            case SERIAL_PORT:
                putchar(*(((char *)run) + run->io.data_offset));
                break;
            case DUMP_REGISTER:
                kvm_dump_register(cpu);
                break;
            case SET_LONGMODE:
                printf("\nEntering 64bit kernel............\n");
                if(run_long_mode(cpu) < 0)
                    exit(1);
                /*uint8_t memval = 0;
                memcpy(&memval, &cpu->mem->ram[0x48e], 1);
                printf("memory at 0x48e is %c\n", (unsigned long long)memval);
                memcpy(&memval, &cpu->mem->ram[0x48f], 1);
                printf("memory at 0x48f is %c\n", (unsigned long long)memval);*/
                break;
            default:
                printf("Port: 0x%x\n", run->io.port);
                DPRINTF("error: unhandled KVM_EXIT_IO\n");
        }
    } else {
        /* KVM_EXIT_IO_IN */
        switch (run->io.port) {
            case SERIAL_PORT:
                *(((char *)run) + run->io.data_offset) = getche();
                break;
            default:
                printf("Port: 0x%x\n", run->io.port);
                DPRINTF("error: unhandled KVM_EXIT_IO\n");    
        }
    }
}           


int KVMHost_init(KVMHost *self)
{
    static const char kvm_name[] = "/dev/kvm";
    int ioctl_ret = 0;

    self->kvm_fd = open(kvm_name, O_RDWR);
    if (self->kvm_fd < 0) {
        perror("open kvm");
        return FD_ERROR;
    }

    if (ioctl(self->kvm_fd, KVM_GET_API_VERSION, 0) != KVM_API_VERSION) {
        DPRINTF("error: kvm version not supported\n");
        return IOCTL_ERROR;
    }

    memset(&self->msr_list_tmp, 0, sizeof(self->msr_list_tmp));
    self->msr_list = &self->msr_list_tmp;
    ioctl_ret = ioctl(self->kvm_fd, KVM_GET_MSR_INDEX_LIST, &self->msr_list_tmp);

    // if any MSR's, then ioctl will fail with E2BIG
    if (ioctl_ret < 0 && self->msr_list_tmp.nmsrs > 0) {
        // allocate enough memory for the full MSR list,
		// now that kvm has told us how many there are
        size_t msr_list_size = sizeof(*self->msr_list) + 
                    sizeof(self->msr_list->indices[0]) * self->msr_list_tmp.nmsrs;

        self->msr_list = calloc(1, msr_list_size);
        self->msr_list->nmsrs = self->msr_list_tmp.nmsrs;

        ioctl_ret = ioctl(self->kvm_fd, KVM_GET_MSR_INDEX_LIST, self->msr_list);
        if (ioctl_ret < 0) {
            DPRINTF("error: failed to get MSR!\n");
            return IOCTL_ERROR;
        }
    }

    return OK;
}

static int kvm_set_user_memory_region(const Vmm *vm, const KVMSlot *slot)
{
    int ret = 0;
    struct kvm_userspace_memory_region mem;
    mem.flags = slot->flags;
    mem.slot = slot->slot;
    mem.guest_phys_addr = slot->start_addr;
    mem.userspace_addr = (unsigned long)slot->ram;
    mem.memory_size = slot->memory_size;

    ret = ioctl(vm->vm_fd, KVM_SET_USER_MEMORY_REGION, &mem);
    return ret;
} 

static int rom_add_file(uint64_t ram_start, uint64_t ram_size, char *file)
{
    int fd;
    int ret = 0;

    fd = open(file, O_RDONLY);
    if (fd == -1) {
        DPRINTF("error: could not open option rom '%s'\n", file);
        goto err;
    }

    int datasize = lseek(fd, 0, SEEK_END);
    if (datasize == -1){
        DPRINTF("error: rom: file %-20s: get size error\n", file);
        goto err;
    }
    if (datasize > ram_size) {
        DPRINTF("error: rom: file %-20s: datasize=%d > ramsize=%zd\n");
        goto err;
    }

    lseek(fd, 0, SEEK_SET);

    int rc = read(fd, (void*)ram_start, datasize);
    if (rc != datasize) {
        DPRINTF("error: rom: file %-20s read error: rc=%d (expect %zd)",
                    file, rc, datasize);
        goto err;
    }

    return ret;

err:
    if (fd != -1)
        close(fd);
    ret = -1;
    return ret;
}

static int kvm_cpu_exec(X86CPU *cpu)
{   
    int ret = 0;
    struct kvm_run *run = cpu->kvm_run;
    
    if (cpu->protected_mode) {
        ret = run_protected_mode(cpu);
        if (ret < 0)
            return ret;
    } else {
        ret = kvm_put_sreg(cpu);
        if (ret < 0)
            return ret;

        ret = kvm_getput_regs(cpu, 1);
        if (ret < 0)
            return ret;
    }

    while (1) {
        ret = ioctl(cpu->vcpu_fd, KVM_RUN, NULL);
        if (ret < 0) {
            DPRINTF("error: KVM_RUN failed %s!\n",
                        strerror(-ret));
            ret = -1;
            break;
        }
        switch (run->exit_reason) {
            case KVM_EXIT_HLT:
                puts("KVM_EXIT_HLT");
                return 0;
            case KVM_EXIT_IO:
                kvm_handle_io(run, cpu);
                break;
            case KVM_EXIT_FAIL_ENTRY:
                DPRINTF("KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx",
                     (unsigned long long)run->fail_entry.hardware_entry_failure_reason);
                break;
            case KVM_EXIT_INTERNAL_ERROR:
                DPRINTF("KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x", run->internal.suberror);
                break;
            default:
                DPRINTF("exit_reason = 0x%x", run->exit_reason);
        }
    }

    return ret;
}

static int kvm_init_vcpu(Vmm *vm, X86CPU *cpu)
{
    int ret = 0;
    unsigned long mmap_size;

    cpu->vcpu_fd = ioctl(vm->vm_fd, KVM_CREATE_VCPU, VCPU_ID);
    if (cpu->vcpu_fd < 0) {
        DPRINTF("error: kvm_create_vcpu failed\n");
        ret = -1;
        goto err;
    }

    mmap_size = ioctl(vm->kvm->kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
    if (mmap_size < 0) {
        DPRINTF("error: KVM_GET_VCPU_MMAP_SIZE failed!\n");
        ret = -1;
        goto err;
    }

    cpu->kvm_run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                            cpu->vcpu_fd, 0);
    if (cpu->kvm_run == MAP_FAILED) {
        DPRINTF("error: mmap vcpu state failed!\n");
        ret = -1;
        goto err;
    }

    return ret;

err:
    if (cpu->vcpu_fd >= 0) {
        close(cpu->vcpu_fd);
    }
    return ret;
}

static void *kvm_cpu_thread_fn(void *arg)
{
    int ret = 0;
    X86CPU *cpu = arg;

    ret = kvm_cpu_exec(cpu);
    if (ret < 0) {
        DPRINTF("error: kvm_cpu_exec failed!\n");
        exit(0);
    }
}

void kvm_start_vcpu(X86CPU *cpu)
{
    pthread_t vcpu_thread;
    if (pthread_create(&(vcpu_thread), (const pthread_attr_t *)NULL,
                            kvm_cpu_thread_fn, cpu) != 0) {
        DPRINTF("error: can not create kvm cpu thread!\n");
        exit(1);
    
    }
    pthread_join(vcpu_thread, NULL);
}

int kvm_cpu_init(Vmm *vm, int protected_mode)
{   
    X86CPU *cpu = malloc(sizeof(X86CPU));
    if (!cpu) {
        DPRINTF("error: X86CPU alloc failed!\n");
        return -1;
    }

    cpu->vcpu_fd = FD_INVALID;
    cpu->mem = vm->mem;
    cpu->kvm_fd = vm->kvm->kvm_fd;

    if (kvm_init_vcpu(vm, cpu) < 0) {
        DPRINTF("error: kvm_init_vcpu failed!\n");
        return -1;
    }

    if (protected_mode)
        cpu->protected_mode = 1;

    vm->cpu = cpu;
    
    return 0;
}

int mem_init(Vmm *vm, uint64_t mem_size, uint64_t start_addr, char *file)
{
    KVMSlot *slot = malloc(sizeof(KVMSlot));
    if (!slot) {
        DPRINTF("error: alloc KVMSlot failed!\n");
        goto alloc_fail;
    }

    slot->memory_size = mem_size;
    slot->slot = 0;
    slot->start_addr = start_addr;
    slot->ram = mmap(NULL, slot->memory_size, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS | MAP_NORESERVE,
                            -1, 0);
    if ((void *)slot->ram == MAP_FAILED) {
        DPRINTF("error: mmap vm ram failed!\n");
        goto mem_init_fail;
    }

    madvise(slot->ram, mem_size, MADV_MERGEABLE);

    //set vm's memory region
    if (kvm_set_user_memory_region(vm, slot) < 0) {
        DPRINTF("error: set memory_region failed!\n");
        goto mem_init_fail;
    }
    //load binary to vm's ram
    /*if (rom_add_file((uint64_t)slot->ram, slot->memory_size, file) < 0) {
        DPRINTF("error: load rom file failed\n");
        goto mem_init_fail;
    }*/

    vm->mem = slot;
    return 0;

mem_init_fail:
    free(slot);
alloc_fail:
    return -1;
}

Vmm *VM_create() 
{   
    int ret = 0;
    KVMHost *host = KVMHost_alloc();
    if (!host) 
        goto host_alloc_fail;
    
    ret = KVMHost_init(host);
    if (ret != OK) 
       goto create_fail;
    
    do {
        ret = ioctl(host->kvm_fd, KVM_CREATE_VM, 0);
    } while (ret == -EINTR);
    if (ret < 0) {
        DPRINTF("ioctl(KVM_CREATE_VM) failed: %s\n",strerror(-ret));
        goto create_fail;
    }

    int vm_fd = ret;
    ret = ioctl(vm_fd, KVM_SET_TSS_ADDR, 0xfffbd000);
    if (ret < 0) {
        DPRINTF("error: KVM_SET_TSS_ADDR failed!\n");
        goto create_fail;
    }

    Vmm *vm = Vmm_alloc();
    if (!vm)
        goto create_fail;

    vm->kvm = host;
    vm->vm_fd = vm_fd;
    return vm;
create_fail:
    KVMHost_free(host);
host_alloc_fail:
    DPRINTF("vm create fail!\n");
    return NULL;
}