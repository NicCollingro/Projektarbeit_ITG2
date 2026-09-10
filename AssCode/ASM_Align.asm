align:
    pop H

    pop F ; b lower
    pop E ; b upper
    pop D ; a lower
    pop C ; a upper

compare:
    mov A C
    mov B E
    cmp
    ldi B 1
    jz %cmp_retcall
    jc %a_smaller_b
    jmp %b_smaller_a

a_smaller_b:
    mov A D
    shr
    mov D A
    mov A C
    ldi B 16
    add
    mov C A
    jmp %compare

b_smaller_a:
    mov A F
    shr
    mov F A
    mov A E
    ldi B 16
    add
    mov E A
    jmp %compare

cmp_retcall:
    push E
    push D
    push F
    push H
    ret