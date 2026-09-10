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
