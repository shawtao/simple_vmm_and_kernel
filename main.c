#include "include/vmm.h"

#define RAM_SIZE 128000000  
#define START_ADDR 0x0000

int main(int argc, char **argv) {
    Vmm *vm = VM_create();
    if (!vm)
        return -1;
    
    if (mem_init(vm, RAM_SIZE, START_ADDR, argv[1]) < 0)
        return -1;
    
    if (kvm_cpu_init(vm,1) < 0)
        return -1;
    
    if (kvm_set_cpuid(vm) < 0)
        return -1;

    kvm_start_vcpu(vm->cpu);

    X86CPU_free(vm->cpu);
    KVMSlot_free(vm->mem);
    KVMHost_free(vm->kvm);
    Vmm_free(vm);

    return 0;
}