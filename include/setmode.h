#ifndef _SETMODE_H
#define _SETMODE_H

#include "vmm.h"

/* 64-bit page * entry bits*/
#define PDE64_PRESENT   1
#define PDE64_RW        (1U << 1)
#define PDE64_USER      (1U << 2)
#define PDE64_ACCESSED  (1U << 5)
#define PDE64_DIRTY     (1U << 6)
#define PDE64_HP        (1U << 7)
#define PDE64_G         (1U << 8)

/* CR0 bits */
#define CR0_PE          1u
#define CR0_MP          (1U << 1)
#define CR0_EM          (1U << 2)
#define CR0_TS          (1U << 3)    
#define CR0_ET          (1U << 4)
#define CR0_NE          (1U << 5)
#define CR0_WP          (1U << 16)
#define CR0_AM          (1U << 18)
#define CR0_NW          (1U << 29)
#define CR0_CD          (1U << 30)
#define CR0_PG          (1U << 31)

#define CR4_PAE         (1U << 5)

#define EFER_SCE 1
#define EFER_LME (1U << 8)
#define EFER_LMA (1U << 10)
#define EFER_NXE (1U << 11)

#define SET_LONGMODE 0x208

int run_long_mode(X86CPU *vcpu);
int run_protected_mode(X86CPU *vcpu);

#endif