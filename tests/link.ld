/* tests/link.ld */
OUTPUT_ARCH("riscv")
ENTRY(_start)

/* Define the memory layout matching our simulator (1MB RAM at 0x0) */
MEMORY {
    ram (rwx) : ORIGIN = 0x00000000, LENGTH = 1M
}

SECTIONS {
    /* Code segment */
    .text : {
        *(.text.init) /* Ensure start.S goes first */
        *(.text*)
    } > ram

    /* Read-only data segment (strings, constants) */
    .rodata : {
        *(.rodata*)
    } > ram

    /* Initialized data segment */
    .data : {
        *(.data*)
    } > ram

    /* Uninitialized data segment */
    .bss : {
        *(.bss*)
    } > ram
}
