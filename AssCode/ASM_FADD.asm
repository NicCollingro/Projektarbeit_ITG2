fadd:
    pop H; ret adrr
    pop B
    pop A
    
    add

    ldi B 16
    cmp
    jc %retcall_add
    ldi B 1
    shr
    mov C A
    pop A
    ldi B 16
    add
    push A
    mov A C

retcall_add:
    push A
    push H
    ret

