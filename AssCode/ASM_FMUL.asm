fmul:
    pop H; store ret Addr
    pop F
    pop E
    pop D
    pop A
    ldi B 112
    sub
    mov B E
    add
    push A; store new exponent
    mov A F
    mov B D
    mlo
    ldi B 0
	cmp
	jz %mul_zero
mul_norm:
    ldi B 16
    cmp
    jc %retcall_mul
    ldi B 1
    shr
    mov C A; store mantissa
    pop A;
    ldi B 16
    add
    push A;
    mov A C;
    jmp %mul_norm

retcall_mul:
    push A;
    push H
    ret

mul_zero:
    pop A
    ldi A 0
    push A
    push A
    push H
    ret
