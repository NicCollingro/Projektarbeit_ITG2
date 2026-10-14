.text
	ldi A 92
	push A
	ldi A 88
	push A
	call %splitter
	call %fdiv
	call %pack
	pop A
	stx A %a
	ldx A %a
	push A
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
fdiv:
	pop H
	pop F
	pop B
	pop D
	pop A
	sub
	ldi B 112
	add
	ldi B 64
	sub
	mov E A
	mov A D
	ldi B 16
	mlo
	mov B F
	ldi C 0
float_divloop:
	cmp
	jc %div_ready
	sub
	push A
	mov A C
	inc
	mov C A
	pop A
	jmp %float_divloop
div_ready:
	mov A C
	ldi B 0
	cmp
	jz %div_zero
div_dn:
	ldi B 16
	cmp
	jc %div_up
	ldi B 1
	shr
	mov C A
	mov A E
	ldi B 16
	add
	mov E A
	mov A C
	jmp %div_dn
div_up:
	ldi B 8
	cmp
	jnc %div_done
	mov C A
	mov A E
	ldi B 0
	cmp
	jz %div_denorm
	ldi B 16
	sub
	mov E A
	mov A C
	mov B A
	add
	mov C A
	jmp %div_up
div_denorm:
	mov A C
div_done:
	push E
	push A
	push H
	ret
div_zero:
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
	a = 0
