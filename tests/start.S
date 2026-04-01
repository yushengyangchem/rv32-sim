/* tests/start.S */
.section .text.init
.global _start

_start:
    /* * Initialize the stack pointer (sp/x2). 
     * Our simulator has 1MB of RAM, which is 0x100000 in hex.
     * The stack grows downwards from the top of memory.
     */
    li sp, 0x00100000

    /* Jump to the C main function */
    call main

_halt:
    /* * If main returns, we trap the CPU to halt execution.
     * Our simulator is configured to halt when it fetches 0x00000000.
     */
    .word 0x00000000
