global print_cpuid

global print_cpu_brand_string

SERIAL_PORT equ 0x3f8

section .text
bits32

print_cpuid:
    mov eax, 0
    cpuid
    push ecx
    push edx
    push ebx

    mov cl, 3
    .next_dword:
        pop eax
        mov bl, 4
        .print_register:
            call print_char
            shr eax, 8
            dec bl
            jnz .print_register
        dec cl
        jnz .next_dword

    ret

print_cpu_brand_string:
    mov al, '"'
    call print_char
    .next_function:
        mov eax, [cpuid_function]
        cpuid
        push edx
        push ecx
        push ebx
        push eax

    mov cl, 4
    .next_dword:
        pop eax
        mov bl, 4
        .print_register:
            call print_char
            shr eax, 8
            dec bl
            jnz .print_register
        dec cl
        jnz .next_dword

    inc dword[cpuid_function]
    cmp dword[cpuid_function], 0x80000004
    jle .next_function

    mov al, '"'
    call print_char
    ret

print_new_line:
    push dx
    push ax
    mov dx, SERIAL_PORT
    mov al, `\n`
    out dx, al
    pop ax
    pop dx
    ret

print_char:
    push dx
    mov dx, SERIAL_PORT
    out dx, al
    pop dx
    ret

print_str:
    push dx
    push ax
    mov dx, SERIAL_PORT
    .print_next_char:
        lodsb               ; load byte pointed to by SI into AL and SI++
        cmp al, 0
        je .printstr_done
        out dx, al
        jmp .print_next_char
    .printstr_done:
        pop ax
        pop dx
        ret

section .data
    cpuid_function      dd  0x80000002


