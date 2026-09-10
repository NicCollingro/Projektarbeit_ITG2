fsub:
	pop H              ; Ruecksprungadresse
	pop B              ; Mb
	pop A              ; Ma
	sub                ; A = Ma - Mb
	pop E              ;
	ldi B 0
	cmp
	jz %sub_zero       ; if M == 0: alles 0

sub_up:                    ; while M < 8 and E > 0: M += M; E -= 16
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
	mov A C            ; Exponent ist 0, Mantisse bleibt klein
sub_done:
	push E             ; Exponent
	push A             ; Mantisse
	push H
	ret
sub_zero:
	ldi A 0
	push A
	push A
	push H
	ret