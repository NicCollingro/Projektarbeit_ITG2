.text
	ldi A 93
	stx A %f
	ldx A %f
	push A
	ldi A 4
	push A
	call %i2f
	call %splitter
	call %fmul
	call %pack
	call %f2i
	pop A
	rout A
	ldi A 13
	stx A %f
	ldx A %f
	push A
	ldi A 128
	push A
	call %i2f
	call %splitter
	call %fmul
	call %pack
	call %f2i
	pop A
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
splitter:
	pop H
	pop A
	mov D A
	pop A
	mov F A
	ldi B 240
	and
	mov B A
	push A
	mov A F
	sub
	push A
	mov A D
	ldi B 240
	and
	mov B A
	push A
	mov A D
	sub
	push A
	push H
	ret
fmul:
	pop H
	pop F
	pop E
	pop D
	pop A
	ldi B 112
	sub
	mov B E
	add
	push A
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
	mov C A
	pop A
	ldi B 16
	add
	push A
	mov A C
	jmp %mul_norm
retcall_mul:
	push A
	push H
	ret
mul_zero:
	pop A
	ldi A 0
	push A
	push A
	push H
	ret
pack:
	pop H
	pop A
	pop B
	or
	push A
	push H
	ret
.data
	f = 0
