ENTRY(_start)

SECTIONS
{
    . = 0x1000; /* Starting address for kernel code */

    .text :
    {
        *(.text)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }
}
