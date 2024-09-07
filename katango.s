.segment "HEADER"

.byte 'N', 'E', 'S', $1A
.byte $02, $01, $01, $00
.byte $00, $00, $00, $00
.byte $00, $00, $00, $00

.segment "VECTORS"
.word nmi
.word rst
.word irq

.segment "TILES"
;; .incbin "katango.chr"

.segment "ZEROPAGE"

.segment "BSS"

.segment "OAM"
oam_buffer:	.res 256

.segment "RODATA"
palette:
.byte $0F, $03, $13, $23
.byte $0F, $0F, $0F, $0F
.byte $0F, $08, $18, $28
.byte $0F, $09, $19, $29

.byte $0F, $06, $16, $30
.byte $0F, $27, $28, $10
.byte $0F, $06, $16, $37
.byte $0F, $06, $16, $17

.segment "SAMPLE"

.segment "CODE"

.import _irq_handler
.import _game_startup

PPUCTRL		= $2000
PPUMASK		= $2001
PPUSTATUS	= $2002
OAMADDR		= $2003
OAMDATA		= $2004
PPUSCROLL	= $2005
PPUADDR		= $2006
PPUDATA		= $2007

DMC_FREQ	= $4010
OAMDMA		= $4014
SND_CHN		= $4015
JOY1		= $4016
JOY2		= $4017

nmi:
	pha
	txa
	pha
	tya
	pha

	jsr	_irq_handler

	ldx	#%00000000
	stx	PPUMASK

	stx	OAMADDR
	lda	#>oam_buffer
	sta	OAMDMA

	lda	#%00011110
	sta	PPUMASK

	pla
	tay
	pla
	tax
	pla
	rti

irq:
	rti

rst:
	;; Setup stack and stuff
	sei
	cld
	ldx	#$40
	stx	JOY2
	ldx	#$ff
	txs
	inx
	stx	PPUCTRL
	stx	PPUMASK
	stx	DMC_FREQ
	lda	#%00001111
	sta	SND_CHN

	;; Wait for PPU to stabilize
	jsr	wait_vblank
	jsr	clear_memory
	jsr	wait_vblank
	jsr	setup_pallete
	jsr	wait_vblank

	lda	#%10000000
	sta	PPUCTRL

	jsr	_game_startup

wait_vblank:
	bit	PPUSTATUS
	bpl	wait_vblank
	rts

clear_memory:
	lda	#0
	ldx	#0
:
	sta	$0000, X
	sta	$0200, X
	sta	$0300, X
	sta	$0400, X
	sta	$0500, X
	sta	$0600, X
	sta	$0700, X
	inx
	bne	:-

	jsr	hide_all_sprites
	rts

hide_all_sprites:
	lda	#255
	ldx	#0
:
	sta	oam_buffer, X
	inx
	inx
	inx
	inx
	bne	:-
	rts

setup_pallete:
	lda	#$3F
	sta	PPUADDR
	lda	#$00
	sta	PPUADDR

	ldx	#0
:
	lda	palette, x
	sta	PPUDATA
	inx
	cpx	#32
	bcc	:-

	rts
