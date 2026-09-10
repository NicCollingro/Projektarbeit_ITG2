splitter:
    pop H ; store ret addr in H
    pop A ; pop float_b
    mov D A; store float_b in F
    pop A; pop float_a
    mov F A; store float_a in F
    ;let the split beginn
    ldi B 240
    and
    mov B A; 
    push A; push a_upper nibble
    mov A F;
    sub
    push A; push a_lower nibble
    mov A D;
    ldi B 240;
    and
    mov B A;
    push A; push b_upper nibble
    mov A D;
    sub;
    push A; push b_lower nibble
    push H; push ret addr
    ret