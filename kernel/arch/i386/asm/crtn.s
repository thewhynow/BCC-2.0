.section .init
    /* gcc will automatically insert here */
    popl %ebp
    ret

.section .fini
    /* gcc will automatically insert here */
    popl %ebp
    ret
