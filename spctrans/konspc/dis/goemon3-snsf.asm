.LOROM

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

.ORG $7E00
.SECTION "SETUP" SEMIFREE

.DEFINE REQUESTCODE1 = $00FE00
	.dw	$0008
.DEFINE REQUESTCODE2 = $00FE02
	.dw	$0000

Start:
	; RESET ($80/FF90)
;	CLC
;	XCE
;	SEI

	; setup start ($80/8000)
	CLC
	XCE
	SEI
	CLD

	; set stack pointer
	REP	#$30
	LDX	#$01AF
	TXS

	; zero memory ($0000-$1FFF)
	REP	#$30
	PHB
	LDA	#$0000
	STA	$000000.L
	LDA	#$1FFD
	LDX	#$0001
	TXY
	INY
	MVN	$00,$00
	PLB

	; zero memory ($7E2000-$7EFFFE)
;	REP	#$30
;	PHB
;	LDA	#$0000
;	STA	$7E2000.L
;	LDA	#$DFFC
;	LDX	#$2001
;	TXY
;	INY
;	MVN	$7E,$7E
;	PLB

	; zero memory ($7F0000-$7FFFFE)
;	REP	#$30
;	PHB
;	LDA	#$0000
;	STA	$7F0000.L
;	LDA	#$FFFC
;	LDX	#$0001
;	TXY
;	INY
;	MVN	$7F,$7F
;	PLB

	; direct page 0
;	LDA	#$0000
;	TCD

;	PEA	$8100
;	PLB
;	PLB

;	SEP	#$20
;	LDA	$4210
;	STZ	$4200

;	LDA	#$80
;	STA	$2100

;	REP	#$20
;	JSL	$808710

;	REP	#$10
;	LDY	#$7FFF
;	LDX	#$0000
;	JSL	$80BF4B

	JSL	$84BAB6	; clear RAM

;	JSL	$808404

	LDX	#$88AD
	JSL	$84BBB0	; transfer SPC program

	SEI
	CLD

	; set stack pointer, again
	REP	#$30
	LDX	#$01AF
	TXS

	; direct page 0
	LDA	#$0000
	TCD

;	PEA	$8100
;	PLB
;	PLB

;	JSL	$84C579	; sync $2140

;	STZ	$1FF4
;	JSL	$84C5BB	; checks $1FF4
;	JSL	$808302
;	JSL	$808710
;	JSL	$808828

;	REP	#$20
;	LDA	#$7000
;	STA	$50
;	LDA	#$7400
;	STA	$52
;	LDA	#$19D2
;	STA	$19D0
;	LDA	#$2C00
;	STA	$4A
;	STZ	$1770
;	STZ	$1778
;	STZ	$177C
;	STZ	$1784
;	STZ	$178C
;	STZ	$1790
;	STZ	$1798
;	STZ	$17A0
;	STZ	$17A4
;	STZ	$17AC
	JSL	$808230

	CLI

	LDY	REQUESTCODE1.L
	JSL	$808950

_LP:
	WAI
;	LDA	#$0000
;	TCD
;	LDA	$4C
;	BNE	loc_80F3
;	LDA	$86
;	ADC	$42
;	STA	$86
;
;loc_80F3:
	BRA	_LP

VBlank:
	; NMI ($80/80F5)
	REP	#$38
	PHA
	PHX
	PHY
	PHD
	PHB
;	LDA	#$0000
;	TCD
;	SEP	#$20
;	LDA	#$81
;	PHA
;	PLB
;	REP	#$20
	LDA	$4210
;	LDA	#$0001
;	LDY	$44
;	STA	$44
;	BEQ	loc_813E
;	JSR	$824B
;	JSR	$81A0
;	JSL	$808344
;	LDA	$1E36
;	BNE	loc_813B
;	LDA	$001D9C
;	PHA
;	LDA	$001D9E
;	PHA
;	JSL	$84C7DD
;	PLA
;	STA	$001D9E
;	PLA
;	STA	$001D9C
;
;loc_813B:
;	BRL	loc_8198
;
;loc_813E:
;	SEP	#$20
;	STZ	$420C
;	REP	#$20
;	JSR	$824B
;	JSR	$81CC
;	JSL	$809493
;	JSL	$809570
;	JSL	$80936E
;	JSL	$8092F6
;	JSR	$81A0
;	JSL	$809784
;	JSR	$82C8
;	JSL	$808344
;	JSL	$848B6C	; run if $1D60 != 0
;	INC	$1E36
;	JSL	$84C7DD	; run if $1E00 != 0
;	JSL	$808622
;	JSL	$808458
;	STZ	$1E36
;	JSL	$80B0A2
;	JSL	$80C316
;	JSL	$8097D5
;	JSL	$84BB83	; call subroutine specified by $0100 or $1F00
;	INC	$1E36
	JSL	$84C523
;	STZ	$44
;
;loc_8198:
	REP	#$30
	PLB
	PLD
	PLY
	PLX
	PLA
	RTI

EmptyHandler:
	RTI

.ENDS
