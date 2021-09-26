#ifndef _VMM_H
#define _VMM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <cpuid.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <linux/kvm.h>
#include "serial.h"

#define KVM_API_VERSION 12
#define FD_INVALID -1
#define VCPU_ID 0
#define DUMP_REGISTER 0x207 

#define DPRINTF(fmt, ...) \
    do { fprintf(stderr, fmt, ## __VA_ARGS__);} while (0)

enum error_type
{   
    OK = 1,
    IOCTL_ERROR = -1,
    FD_ERROR = -2,
    MAX_ERROR = -3,
};

typedef struct KVMHost_t {
    int kvm_fd;
    struct kvm_msr_list msr_list_tmp;
    struct kvm_msr_list *msr_list;
} KVMHost;

typedef struct KVMSlot_t {
    uint64_t start_addr;
    uint64_t memory_size;
    void *ram;
    int slot;
    int flags;
} KVMSlot;

typedef struct CPUState {
    int kvm_fd;
    int vcpu_fd;
    int protected_mode;
    KVMSlot *mem;
    struct kvm_run *kvm_run;
} X86CPU;

typedef struct Vmm_t {
    KVMHost *kvm;
    KVMSlot *mem;
    X86CPU *cpu;
    int vm_fd;
} Vmm;


static inline KVMHost *KVMHost_alloc(void) 
{
    KVMHost *self = (KVMHost *)calloc(1, sizeof(KVMHost));
    if (!self) {
        DPRINTF("error: KVMHost alloc failed!\n");
        return NULL;
    }

    self->kvm_fd = FD_INVALID;

    return self;
}

static inline void KVMHost_free(KVMHost *self) 
{
    if (self) {
        if (self->kvm_fd != -1)
            close(self->kvm_fd);
        free(self);
    }
}

static inline Vmm *Vmm_alloc(void) 
{
    Vmm *self = (Vmm *)calloc(1, sizeof(Vmm));
    if (!self) {
        DPRINTF("error: Vmm alloc failed!\n");
        return NULL;
    }

    self->vm_fd = FD_INVALID;

    return self;
}

static inline void Vmm_free(Vmm *self) 
{
    if (self) {
        if (self->vm_fd != -1)
            close(self->vm_fd);
        free(self);
    }
}

static inline void X86CPU_free(X86CPU *self) 
{   
    if (self) {
        unsigned long mmap_size;

        mmap_size = ioctl(self->kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
        munmap(self->kvm_run, mmap_size);

        if (self->vcpu_fd != -1)
            close(self->vcpu_fd);
        free(self);
    }
}

static inline void KVMSlot_free(KVMSlot *self)
{   
    if (self) {
        if (self->ram)
            munmap((void*)self->ram, self->memory_size);
        free(self);
    }
}

int mem_init(Vmm *vm, uint64_t mem_size, uint64_t start_addr, char *file);
int kvm_cpu_init(Vmm *vm, int protected_mode);
void kvm_start_vcpu(X86CPU *cpu);
int kvm_set_cpuid(const Vmm *vm);


Vmm *VM_create();


#endif


