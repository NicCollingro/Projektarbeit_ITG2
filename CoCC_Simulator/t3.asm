.text
	ldi A 76
	stx A %a
	ldi A 56
	stx A %b
	ldx A %a
	push A
	ldx A %b
	push A
	call %splitter
	call %align
	call %fsub
	call %pack
	call %f2i
	pop A
	rout A
	ldi A 124
	stx A %a
	ldi A 123
	stx A %b
	ldx A %a
	push A
	ldx A %b
	push A
	call %splitter
	call %align
	call %fsub
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
align:
	pop H
	pop F
	pop E
	pop D
	pop C
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
	ldi B 1
	shr
	mov D A
	mov A C
	ldi B 16
	add
	mov C A
	jmp %compare
b_smaller_a:
	mov A F
	ldi B 1
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
fsub:
	pop H
	pop B
	pop A
	sub
	pop E
	ldi B 0
	cmp
	jz %sub_zero
sub_up:
	ldi B 8
	cmp
	jnc %sub_done
	mov C A
	mov A E
	ldi B 0
	cmp
	jz %sub_denorm
	ldi B 16
	sub
	mov E A
	mov A C
	mov B A
	add
	mov C A
	jmp %sub_up
sub_denorm:
	mov A C
sub_done:
	push E
	push A
	push H
	ret
sub_zero:
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
	b = 0
