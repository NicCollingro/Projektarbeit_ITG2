.text
	ldi A 0
	stx A %i
flp11:
	ldx A %i
	push A
	ldi A 3
	pop B
	cmp
	ldi C 1
	jnz %bo16
	ldi C 0
bo16:
	jnc %bo26
	ldi C 0
bo26:
	mov A C
	ldi B 0
	cmp
	jz %flpe1
	jmp %flp31
flp21:
	ldx A %i
	push A
	ldi A 1
	pop B
	add
	stx A %i
	jmp %flp11
flp31:
	ldx A %i
	push A
	ldx A %i
	mov C A
	pop A
	ldi B %w000
	add
	sta C 
	jmp %flp21
flpe1:
	ldi A 1
	ldi B %w000
	add
	lda C
	mov A C
	rout A
	ldi A 76
	push A
	call %f2i
	pop A
	ldi B %w000
	add
	lda C
	mov A C
	rout A
	ldi A 90
	stx A %f
	ldx A %f
	push A
	call %f2i
	pop A
	ldi B %w000
	add
	lda C
	mov A C
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
.data
	f = 0
	i = 0
	w000 = 0
	w001 = 0
	w002 = 0
