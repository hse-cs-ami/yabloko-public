    .global trapret

alltraps:
    # Build trap frame.
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    pushal

    mov $0x10, %ax
    mov %ax, %ds
    
    # Call trap(tf), where tf=%esp
    pushl %esp
    call trap
    add $4, %esp
    // execution falls through to trapret

trapret:
    popal
    popl %gs
    popl %fs
    popl %es
    popl %ds
    addl $0x8, %esp  # trapno and errcode
    iret

.macro handler i
vector\i :
    .if (!(\i == 8 || (\i >= 10 && \i <= 14) || \i == 17))
    pushl $0
    .endif
    pushl $\i
    jmp alltraps
.endm

.altmacro

.macro irq_insertX number
    .section .text
    handler \number

    .section .rodata
    .long vector\number
.endm

.section .rodata
.global default_handlers
default_handlers:
.set i,0
.rept 256
    irq_insertX %i
    .set i, i+1
.endr
