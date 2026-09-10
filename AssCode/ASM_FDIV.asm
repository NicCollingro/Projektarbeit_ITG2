fdiv:
	pop H              ; Ruecksprungadresse
	pop F              ; Mb
	pop B              ; Eb
	pop D              ; Ma
	pop A              ; Ea
	sub                ; A = Ea - Eb
	ldi B 112
	add                ; + Bias
	ldi B 64
	sub                ; -4 Schritte, weil Mantisse x16 skaliert wird
	mov E A            ; Exponent ins Register, nicht auf den Stack
	mov A D            ; Dividend  = Ma
	ldi B 16
	mlo                ; Ma * 16
	mov B F            ; Divisor   = Mb
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
	mov A C            ; A = Quotient = Mantisse
	ldi B 0
	cmp
	jz %div_zero       ; if M == 0: alles 0

div_dn:                    ; while M >= 16: M >>= 1; E += 16
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

div_up:                    ; while M < 8 and E > 0: M += M; E -= 16
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
	mov A C            ; Exponent ist 0
div_done:
	push E             ; Exponent
	push A             ; Mantisse
	push H
	ret
div_zero:
	ldi A 0
	push A
	push A
	push H
	ret