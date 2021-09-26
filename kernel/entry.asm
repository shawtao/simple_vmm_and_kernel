global _start
extern main

section .start.text
bits32
_start:
    jmp _entry
ALIGN 8

_entry:
    xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	xor edi, edi
	xor esi, esi
	xor ebp, ebp
	xor esp, esp
    ;初始化栈
    mov esp, stack_top
    call main

section .bss
; reserved memory for paging
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096
; reserved memory for stack
stack_bottom:
    resb 4096 * 4
stack_top:






