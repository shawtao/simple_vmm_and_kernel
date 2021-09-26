OUTPUT_FORMAT(binary)
SECTIONS
{
        .start.text : { *(.start.text) }
        .text : { *(.text*) }
        .rodata : { *(.rodata) }
        .data : { *(.data) }
        .bss  : { *(.bss) }
}
