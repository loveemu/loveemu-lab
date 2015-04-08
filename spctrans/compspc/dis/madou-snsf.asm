.LOROM

; Cartridge header should be copied from the original ROM
.SNESHEADER
 ID    "ADYJ"
 NAME  "MADOMONOGATARI       "
 LOROM
 FASTROM
 CARTRIDGETYPE $02
 ROMSIZE $0B
 SRAMSIZE $03
 COUNTRY $00
 LICENSEECODE $33
 VERSION $00
.ENDSNES

.MEMORYMAP
 SLOTSIZE $8000
 DEFAULTSLOT 0
 SLOT 0 $8000
.ENDME

.ROMBANKSIZE $8000
.ROMBANKS 1

.SNESNATIVEVECTOR               ; Define Native Mode interrupt vector table
  COP EmptyHandler
  BRK EmptyHandler
  ABORT EmptyHandler
  NMI VBlank
  IRQ EmptyHandler
.ENDNATIVEVECTOR

.SNESEMUVECTOR                  ; Define Emulation Mode interrupt vector table
  COP EmptyHandler
  ABORT EmptyHandler
  NMI EmptyHandler
  RESET Start
  IRQBRK EmptyHandler
.ENDEMUVECTOR


.BANK 0 SLOT 0

.ORG $0000
.SECTION "SETUP" SEMIFREE

.EMPTYFILL $FF

.DEFINE PARAM_SONG = $808000
	.dw	$0000
.DEFINE PARAM_SONG_2 = $808002
	.dw	$0000
.DEFINE PARAM_RESERVED_1 = $808004
	.dw	$0000
.DEFINE PARAM_RESERVED_2 = $808006
	.dw	$0000

; setup routine derived from the original setup ($80/8000)
Start:
	sei
	clc
	xce

;	sep	#$20
;	rep	#$10
;	stz	$420B
;	stz	$420C
;	lda	#$8F
;	sta	$2100
;	lda	#$00
;	sta	$4200

	; zero memory ($0000-$1FFF)
	rep	#$30
	ldx	#$0000
	ldy	#$0001
	stz	$00
	lda	#$1ffd
	mvn	$00,$00

;	ldx #$0000
;	ldy #$0001
;	lda #$0000
;	sta $7E0000
;	lda #$FFFE
;	mvn $7E,$7E
;	ldx #$0000
;	ldy #$0001
;	lda #$0000
;	sta $7F0000
;	lda #$FFFE
;	mvn $7F,$7F

	sep	#$20
	phk
	plb
	rep	#$20

	; set direct page 0
	lda	#$0000
	tcd

	; set stack pointer
	sep	#$20
	ldx	#$1fff
	txs

	phk
	plb
	rep	#$20
	lda	#$cf6a
	sta	$01
	stz	$62
	stz	$5e
	stz	$60
	stz	$420b

	; setup interrupt
	sep	#$20
	lda	#$81
	sta	$0c84
	lda	#$81
	sta	$4200
	cli

;	sep	#$20
;	jsr	$8d2f
;	ldx	#$1fff
;	txs

;	phk
;	plb
;	rep	#$20
;	lda	#$cf6a
;	sta	$01
;	stz	$62
;	stz	$5e
;	stz	$60
;	stz	$420b
;	sep	#$20
;	lda	#$01
;	sta	$0d00
;	lda	#$81
;	sta	$0c84
;	lda	#$81
;	sta	$4200
;	cli
;	jsl	$02ede3
;	jsr	$8b92
;	jsr	$8b92
;	lda	$0c5b
;	inc	a
;	sta	$0c5b
;	cmp	#$0f
;	bne	$8088
;	jsl	$02eef9

	; transfer SPC program
	jsr	$e6ad

	lda	#$01
	jsl	$00e74f
	lda	#$02
	jsl	$00e74f
	lda	#$03
	jsl	$00e74f

	; set stereo
	jsl	$00ecc9
	lda	#$01
	jsl	$00ecb8

	lda	#$f1
	jsl	$00e87f

	lda	#$01
	sta	$5d

loc_PlaySound:
	rep #$20
	lda	PARAM_SONG_2
	bmi	loc_PlaySFX

loc_PlayBGMPreload:
	beq	loc_PlayBGM
;	wai
;	wai
;	wai
;	wai

	sep #$20
	jsl	$80ec44

loc_PlayBGM:
	lda	PARAM_SONG
	sep #$20
	jsl	$80ebfa
;	jsl	$80ec44

	bra loc_MainLoop

loc_PlaySFX:
	and	#$7f
	beq	loc_PlayVoice

	lda	PARAM_SONG
	sep #$20
	jsl	$80e891

	bra loc_MainLoop

loc_PlayVoice:
	lda	PARAM_SONG
	sep #$20
	jsl	$80ebb0

loc_MainLoop:
	wai
	bra	loc_MainLoop

VBlank:
	; NMI ($80/8864)
	rep	#$30
	phb
	pha
	phx
	phy
	phd

	; clear NMI flag
	phk
	plb
	lda	$4210

	; disable interrupt
	lda	#$01
	sta	$4200

	; increment framecount
;	inc $77

	lda	$5d
	beq	loc_SkipSoundReq

	; dispatch sound requests
	sep	#$10
	jsr	$e8da
	rep	#$10

loc_SkipSoundReq:
	; restore interrupt
	lda	$0c84
	sta	$4200

	rep	#$20
	pld
	ply
	plx
	pla
	plb
	sep	#$20
	rti

EmptyHandler:
	RTI

.ENDS
