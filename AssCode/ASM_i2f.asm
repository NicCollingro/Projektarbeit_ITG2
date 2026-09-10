i2f:
	pop H
	pop A
	ldi B 0
	cmp
	jz %i2f_zero
	mov F A
	ldi A 112
	mov E A
i2f_dn:
	mov A F
	ldi B 16
	cmp
	jc %i2f_up
	mov A F
	ldi B 1
	shr
	mov F A
	mov A E
	ldi B 16
	add
	mov E A
	jmp %i2f_dn
i2f_up:
	mov A F
	ldi B 8
	cmp
	jnc %i2f_done
	mov A E
	ldi B 0
	cmp
	jz %i2f_done
	mov A F
	mov B A
	add
	mov F A
	mov A E
	ldi B 16
	sub
	mov E A
	jmp %i2f_up
i2f_done:
	mov A E
	mov B F
	or
	push A
	push H
	ret
i2f_zero:
	ldi A 0
	push A
	push H
	ret
