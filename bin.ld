ENTRY(_main)

SECTIONS {
    . = 0x800000;

    .text : {
        *(.text._main)
        *(.text*)
    }

    .rodata : { *(.rodata*) }

    .data : { *(.data*) }

    .bss : {
        *(COMMON)
        *(.bss*)
    }
}
