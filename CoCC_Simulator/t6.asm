.text
	ldi A 3
	push A
	call %i2f
	pop A
	stx A %f
	ldx A %f
	push A
	call %f2i
	pop A
	rout A
	ldi A 91
	push A
	call %f2i
	pop A
	stx A %n
	ldx A %n
	rout A
	ldi A 76
	push A
	call %f2i
	pop A
	stx A %n
	ldx A %n
	rout A
	hlt
f2i:
	pop H
	pop A
	mov D A
	ldi B 15
	and
	mov F A
	mov A D
	ldi B 240
	and
	mov E A
f2i_loop:
	mov A E
	ldi B 112
	cmp
	jz %f2i_done
	jc %f2i_down
	mov A F
	mov B A
	add
	mov F A
	mov A E
	ldi B 16
	sub
	mov E A
	jmp %f2i_loop
f2i_down:
	mov A F
	ldi B 1
	shr
	mov F A
	mov A E
	ldi B 16
	add
	mov E A
	jmp %f2i_loop
f2i_done:
	push F
	push H
	ret
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
.data
	f = 0
	n = 0
