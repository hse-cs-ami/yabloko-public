    // swtch(void** oldstack, void* newstack)
    .global swtch
swtch:
    mov 4(%esp), %eax // eax holds "oldstack"
    mov 8(%esp), %ecx

    push %ebx
    push %ebp
    push %esi
    push %edi

    mov %esp, (%eax) // save stack ptr to "oldstack"
    mov %ecx, %esp   // use "newstack" as stack ptr

    pop %edi
    pop %esi
    pop %ebp
    pop %ebx

    ret
