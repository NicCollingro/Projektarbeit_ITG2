.text
	ldi A 10
	stx A %a
	ldi A 3
	stx A %b
	ldx A %a
	rout A
	ldx A %b
	rout A
	ldx A %a
	push A
	ldx A %b
	mov B A
	pop A
	ldi C 0
divlop11:
	cmp
	jc %divend11
	sub
	push A
	mov A C
	inc
	mov C A
	pop A
	jmp %divlop11
divend11:
	mov A C
	rout A
	ldx A %a
	push A
	ldx A %b
	mov B A
	pop A
	ldi C 0
modlop15:
	cmp
	jc %modend15
	sub
	push A
	mov A C
	inc
	mov C A
	pop A
	jmp %modlop15
modend15:
	rout A
	ldi A 0
	stx A %s
	ldi A 1
	stx A %i
flp11:
	ldx A %i
	push A
	ldi A 10
	pop B
	cmp
	ldi C 1
	jnc %bo124
	ldi C 0
bo124:
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
	ldx A %i
	pop B
	add
	stx A %s
	jmp %flp21
flpe1:
	ldx A %s
	rout A
	ldi A 0
	stx A %i
flp12:
	ldx A %i
	push A
	ldi A 5
	pop B
	cmp
	ldi C 1
	jnz %bo143
	ldi C 0
bo143:
	jnc %bo243
	ldi C 0
bo243:
	mov A C
	ldi B 0
	cmp
	jz %flpe2
	jmp %flp32
flp22:
	ldx A %i
	push A
	ldi A 1
	pop B
	add
	stx A %i
	jmp %flp12
flp32:
	ldx A %i
	push A
	ldx A %i
	push A
	ldx A %i
	pop B
	mlo
	mov C A
	pop A
	ldi B %w000
	add
	sta C 
	jmp %flp22
flpe2:
	ldi A 0
	stx A %s
	ldi A 0
	stx A %i
flp13:
	ldx A %i
	push A
	ldi A 5
	pop B
	cmp
	ldi C 1
	jnz %bo163
	ldi C 0
bo163:
	jnc %bo263
	ldi C 0
bo263:
	mov A C
	ldi B 0
	cmp
	jz %flpe3
	jmp %flp33
flp23:
	ldx A %i
	push A
	ldi A 1
	pop B
	add
	stx A %i
	jmp %flp13
flp33:
	ldx A %s
	push A
	ldx A %i
	ldi B %w000
	add
	lda C
	mov A C
	pop B
	add
	stx A %s
	jmp %flp23
flpe3:
	ldx A %s
	rout A
	ldi A 2
	ldi B %w000
	add
	lda C
	mov A C
	rout A
	hlt
.data
	a = 0
	b = 0
	i = 0
	s = 0
	w000 = 0
	w001 = 0
	w002 = 0
	w003 = 0
	w004 = 0
