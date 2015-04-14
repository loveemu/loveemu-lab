.LOROM

; Cartridge header should be copied from the original ROM
.SNESHEADER
 ID    "A42U"
 NAME  "goemon 3             "
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
	.dw	$0002
.DEFINE PARAM_SONG_PRELOAD = $808002
	.dw	$0000
.DEFINE PARAM_SONG_TYPE = $808004
	.dw	$0000
.DEFINE PARAM_RESERVED_2 = $808006
	.dw	$0000

.DEFINE FrameCount = $7E0042

; setup routine derived from the original setup ($80/8000)
Start:
	clc
	xce
	sei
	cld

	; set stack pointer
	rep	#$30
	ldx	#$01af
	txs

	; zero memory ($0000-$1FFF)
	rep	#$30
	phb
	lda	#$0000
	sta	$000000.l
	lda	#$1ffd
	ldx	#$0001
	txy
	iny
	mvn	$00,$00
	plb

	; set direct page 0
	lda	#$0000
	tcd

	; clear sound area of main RAM
	jsl	$84bab6

	; transfer SPC program
	ldx	#$88ad
	jsl	$84bbb0

	sei
	cld

	; set stack pointer, again
	rep	#$30
	ldx	#$01af
	txs

	; set direct page 0, again
	lda	#$0000
	tcd

	; clear framecount
	lda #0
	sta FrameCount

	; setup interrupt
	rep	#$20
	jsl	$808230
	cli

loc_WaitDspInit:
	wai

	; Konami driver unsets the FLG mute bit after the initialization.
	; However, it seems to be done with a little delay.
	; Moreover, the SPC driver will never notify the unmute timing.

	; Therefore, we need to wait a little, before sending a request.
	; The mute continues for about 20 frames, but we wait for 30 frames for safe.
	lda	FrameCount
	cmp	#30
	bcc	loc_WaitDspInit

	; Another bug note:
	; The driver does not initialize the GAIN register.
	; First notes in some songs might sound odd a little.

loc_PlaySound:
	lda	PARAM_SONG_TYPE
	beq	loc_PlayBGM
	bpl	loc_PlaySFX
	bmi	loc_PlayVoice

loc_PlayBGM:
	lda	PARAM_SONG_PRELOAD
	bpl	loc_SkipPreload
	and	#$7fff

	; request BGM playback (preload)
	asl	a
	asl	a
	tay
	jsl	$808950

loc_SkipPreload
	lda	PARAM_SONG
	bmi	loc_PlayKonamiLogo

	; request BGM playback
	asl	a
	asl	a
	tay
	jsl	$808950

	bra	loc_EnterMainLoop

loc_PlayKonamiLogo:
	lda	#$0000
	jsl	$84c6d5

	bra	loc_EnterMainLoop

loc_PlaySFX:
	; request SFX playback
	lda	PARAM_SONG
	jsl	$84c594

	bra loc_EnterMainLoop

loc_PlayVoice:
	; request sampled SFX playback
	lda	PARAM_SONG
	asl	a
	asl	a
	jsl	$8089bd

loc_EnterMainLoop:
	lda	#0
	sta	FrameCount

loc_MainLoop:
	wai
	bra	loc_MainLoop

VBlank:
	; NMI ($80/80F5)
	rep	#$38
	pha
	phx
	phy
	phd
	phb

	; clear NMI flag
	lda	$4210

	; increment framecount
	lda FrameCount
	inc a
	sta FrameCount

	; dispatch sampled SFXs
	jsl	$84c7dd

	; dispatch sound requests
	jsl	$84c523

	rep	#$30
	plb
	pld
	ply
	plx
	pla
	rti

EmptyHandler:
	RTI

.ENDS
