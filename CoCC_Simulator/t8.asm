.text
	ldi A 40
	stx A %x
	ldi A 0
	stx A %i
flp11:
	ldx A %i
	push A
	ldi A 8
	pop B
	cmp
	ldi C 1
	jnz %bo18
	ldi C 0
bo18:
	jnc %bo28
	ldi C 0
bo28:
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
	ldx A %s
	push A
	ldx A %x
	push A
	call %splitter
	call %align
	call %fadd
	call %pack
	pop A
	stx A %s
	jmp %flp21
flpe1:
	ldx A %s
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
fadd:
	pop H
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
pack:
	pop H
	pop A
	pop B
	or
	push A
	push H
	ret
.data
	i = 0
	s = 0
	x = 0
