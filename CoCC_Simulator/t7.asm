.text
	ldi A 60
	stx A %a
	ldi A 76
	stx A %b
	ldx A %a
	push A
	ldx A %b
	pop B
	cmp
	ldi C 1
	jnz %bo17
	ldi C 0
bo17:
	jnc %bo27
	ldi C 0
bo27:
	mov A C
	ldi B 0
	cmp
	jz %else1
	ldi A 1
	rout A
	jmp %cont1
else1:
	ldi A 0
	rout A
cont1:
	ldx A %a
	push A
	ldx A %b
	mov B A
	pop A
	cmp
	ldi C 1
	jnz %bo117
	ldi C 0
bo117:
	jnc %bo217
	ldi C 0
bo217:
	mov A C
	ldi B 0
	cmp
	jz %else2
	ldi A 1
	rout A
	jmp %cont2
else2:
	ldi A 0
	rout A
cont2:
	ldx A %a
	push A
	ldi A 60
	pop B
	cmp
	ldi C 1
	jz %bo127
	ldi C 0
bo127:
	mov A C
	ldi B 0
	cmp
	jz %else3
	ldi A 1
	rout A
	jmp %cont3
else3:
	ldi A 0
	rout A
cont3:
	ldx A %a
	push A
	ldx A %b
	pop B
	cmp
	ldi C 1
	jnz %bo137
	ldi C 0
bo137:
	jnc %bo237
	ldi C 0
bo237:
	push C
	ldx A %b
	push A
	ldi A 78
	pop B
	cmp
	ldi C 1
	jnz %bo140
	ldi C 0
bo140:
	jnc %bo240
	ldi C 0
bo240:
	push C
	ldi B 0
	ldi C 1
	pop A
	cmp
	jnz %kon141
	ldi C 0
kon141:
	pop A
	cmp
	jnz %kon241
	ldi C 0
kon241:
	mov A C
	ldi B 0
	cmp
	jz %else4
	ldi A 1
	rout A
	jmp %cont4
else4:
	ldi A 0
	rout A
cont4:
	hlt
.data
	a = 0
	b = 0
