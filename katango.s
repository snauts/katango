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
.incbin "tiles.chr"
.incbin "sprites.chr"

.segment "ZEROPAGE"

.import sp

.segment "BSS"

.segment "OAM"
_oam_buffer:	.res 256

.segment "RODATA"

.segment "SAMPLE"

.segment "CODE"

.import _irq_handler
.import _game_startup
.import _setup_palette
.export _oam_buffer

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
	lda	#>_oam_buffer
	sta	OAMDMA

	stx	PPUSCROLL
	stx	PPUSCROLL
	lda	#%10001000
	sta	PPUCTRL

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
	jsr	_setup_palette
	jsr	wait_vblank

	ldx	#$20
	stx	sp + 1

	lda	#%0000000
	sta	PPUSCROLL
	sta	PPUSCROLL
	lda	#%10001000
	sta	PPUCTRL

	jsr	_game_startup
:	jmp	:-

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
	sta	_oam_buffer, X
	inx
	inx
	inx
	inx
	bne	:-
	rts
