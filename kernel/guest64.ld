OUTPUT_FORMAT(binary)
SECTIONS
{       . = 0x3e0;
        guest64 = .;
        .start.text : { *(.start.text) }
        .text : { *(.text*) }
        .rodata : { *(.rodata) }
        .data : { *(.data) }
        .bss  : { *(.bss) }
        guest64_end = .;
}