; Madara 2
; note number := $80-$df?
; note := <note number> <velocity $00-$ff?>
; Wild Dance of Fear Bass = around $487e
; track pointers are at $2f+x?
; and they are loaded from $0120+x?

08ca: 20        clrp
08cb: cd cf     mov   x,#$cf
08cd: bd        mov   sp,x
08ce: e8 30     mov   a,#$30
08d0: c5 f1 00  mov   $00f1,a
08d3: e8 00     mov   a,#$00
08d5: cd 00     mov   x,#$00
08d7: af        mov   (x)+,a
08d8: c8 f0     cmp   x,#$f0
08da: d0 fb     bne   $08d7
08dc: 8d 00     mov   y,#$00
08de: 8f 00 04  mov   $04,#$00
08e1: 8f 01 05  mov   $05,#$01
08e4: d7 04     mov   ($04)+y,a
08e6: ad 8f     cmp   y,#$8f
08e8: d0 05     bne   $08ef
08ea: 78 01 05  cmp   $05,#$01
08ed: f0 07     beq   $08f6
08ef: fc        inc   y
08f0: d0 f2     bne   $08e4
08f2: ab 05     inc   $05
08f4: 2f ee     bra   $08e4
08f6: 8d 03     mov   y,#$03
08f8: 8f 00 04  mov   $04,#$00
08fb: 8f 02 05  mov   $05,#$02
08fe: d7 04     mov   ($04)+y,a
0900: ad c4     cmp   y,#$c4
0902: d0 05     bne   $0909
0904: 78 03 05  cmp   $05,#$03
0907: f0 07     beq   $0910
0909: fc        inc   y
090a: d0 f2     bne   $08fe
090c: ab 05     inc   $05
090e: 2f ee     bra   $08fe
0910: e8 00     mov   a,#$00
0912: 3f a6 11  call  $11a6
0915: a2 14     set5  $14
0917: 8d 71     mov   y,#$71
0919: e8 00     mov   a,#$00
091b: cc f2 00  mov   $00f2,y
091e: c5 f3 00  mov   $00f3,a
0921: dc        dec   y
0922: cc f2 00  mov   $00f2,y
0925: c5 f3 00  mov   $00f3,a
0928: dd        mov   a,y
0929: 80        setc
092a: a8 0f     sbc   a,#$0f
092c: fd        mov   y,a
092d: 10 ea     bpl   $0919
092f: e8 ff     mov   a,#$ff
0931: c4 13     mov   $13,a
0933: 8d 5c     mov   y,#$5c
0935: cc f2 00  mov   $00f2,y
0938: c5 f3 00  mov   $00f3,a
093b: e8 7f     mov   a,#$7f
093d: 8d 0c     mov   y,#$0c
093f: cc f2 00  mov   $00f2,y
0942: c5 f3 00  mov   $00f3,a
0945: 8d 1c     mov   y,#$1c
0947: cc f2 00  mov   $00f2,y
094a: c5 f3 00  mov   $00f3,a
094d: e8 50     mov   a,#$50
094f: 8d 5d     mov   y,#$5d
0951: cc f2 00  mov   $00f2,y
0954: c5 f3 00  mov   $00f3,a
0957: e8 20     mov   a,#$20
0959: c5 fa 00  mov   $00fa,a
095c: 1c        asl   a
095d: c5 fb 00  mov   $00fb,a
0960: e8 03     mov   a,#$03
0962: c5 f1 00  mov   $00f1,a
0965: e5 fd 00  mov   a,$00fd
0968: f0 fb     beq   $0965
096a: ab 25     inc   $25
096c: 03 25 07  bbs0  $25,$0976
096f: e5 fe 00  mov   a,$00fe
0972: f0 fb     beq   $096f
0974: c4 2b     mov   $2b,a
0976: 8d 0a     mov   y,#$0a
0978: ad 05     cmp   y,#$05
097a: f0 07     beq   $0983
097c: b0 08     bcs   $0986
097e: 69 1a 19  cmp   ($19),($1a)
0981: d0 11     bne   $0994
0983: e3 19 0e  bbs7  $19,$0994
0986: f6 6c 15  mov   a,$156c+y
0989: c5 f2 00  mov   $00f2,a
098c: f6 76 15  mov   a,$1576+y
098f: 5d        mov   x,a
0990: e6        mov   a,(x)
0991: c5 f3 00  mov   $00f3,a
0994: fe e2     dbnz  y,$0978
0996: cb 12     mov   $12,y
0998: cb 13     mov   $13,y
099a: cb 1d     mov   $1d,y
099c: e4 23     mov   a,$23
099e: 60        clrc
099f: 88 40     adc   a,#$40
09a1: c4 23     mov   $23,a
09a3: 90 07     bcc   $09ac
09a5: 69 1a 19  cmp   ($19),($1a)
09a8: f0 02     beq   $09ac
09aa: ab 19     inc   $19
09ac: 13 28 0d  bbc0  $28,$09bc
09af: 12 28     clr0  $28
09b1: e4 27     mov   a,$27
09b3: f0 07     beq   $09bc
09b5: c4 0c     mov   $0c,a
09b7: 3f 05 1a  call  $1a05
09ba: 2f 05     bra   $09c1
09bc: 3f 69 19  call  $1969
09bf: f0 09     beq   $09ca
09c1: 0e 13 00  tset1 $0013
09c4: 4e 1b 00  tclr1 $001b
09c7: 0e 1d 00  tset1 $001d
09ca: e4 1e     mov   a,$1e
09cc: f0 1a     beq   $09e8
09ce: cd 3f     mov   x,#$3f
09d0: ab 1f     inc   $1f
09d2: e5 04 02  mov   a,$0204
09d5: 2e 1f 10  cbne  $1f,$09e8
09d8: 8f 00 1f  mov   $1f,#$00
09db: 8b 1e     dec   $1e
09dd: d0 09     bne   $09e8
09df: 3f e3 19  call  $19e3
09e2: 0e 13 00  tset1 $0013
09e5: 4e 1b 00  tclr1 $001b
09e8: 8f 01 1c  mov   $1c,#$01
09eb: cd 00     mov   x,#$00
09ed: f4 d0     mov   a,$d0+x
09ef: f0 6b     beq   $0a5c
09f1: e4 1c     mov   a,$1c
09f3: 24 1d     and   a,$1d
09f5: d0 65     bne   $0a5c
09f7: f5 a5 02  mov   a,$02a5+x
09fa: 5c        lsr   a
09fb: b0 0e     bcs   $0a0b
09fd: 7d        mov   a,x
09fe: 5c        lsr   a
09ff: 28 01     and   a,#$01
0a01: c4 04     mov   $04,a
0a03: e4 25     mov   a,$25
0a05: 28 01     and   a,#$01
0a07: 44 04     eor   a,$04
0a09: d0 51     bne   $0a5c
0a0b: d8 22     mov   $22,x
0a0d: f5 33 15  mov   a,$1533+x
0a10: c4 24     mov   $24,a
0a12: f5 a5 02  mov   a,$02a5+x
0a15: c4 21     mov   $21,a
0a17: f5 b5 02  mov   a,$02b5+x
0a1a: c4 29     mov   $29,a
0a1c: f5 b6 02  mov   a,$02b6+x
0a1f: c4 2a     mov   $2a,a
0a21: 8d 01     mov   y,#$01
0a23: 03 21 02  bbs0  $21,$0a28
0a26: eb 2b     mov   y,$2b
0a28: f4 bf     mov   a,$bf+x
0a2a: c4 04     mov   $04,a
0a2c: 8f 00 05  mov   $05,#$00
0a2f: f5 05 02  mov   a,$0205+x
0a32: cf        mul   ya
0a33: 7a 04     addw  ya,$04
0a35: d4 bf     mov   $bf+x,a
0a37: ad 00     cmp   y,#$00
0a39: f0 0f     beq   $0a4a
0a3b: dc        dec   y
0a3c: f0 04     beq   $0a42
0a3e: 1a 29     decw  $29
0a40: fe fc     dbnz  y,$0a3e
0a42: 3f fb 0a  call  $0afb
0a45: 3f 02 0c  call  $0c02
0a48: 2f 03     bra   $0a4d
0a4a: 3f 65 0a  call  $0a65
0a4d: e4 21     mov   a,$21
0a4f: d5 a5 02  mov   $02a5+x,a
0a52: e4 29     mov   a,$29
0a54: d5 b5 02  mov   $02b5+x,a
0a57: e4 2a     mov   a,$2a
0a59: d5 b6 02  mov   $02b6+x,a
0a5c: 3d        inc   x
0a5d: 3d        inc   x
0a5e: 0b 1c     asl   $1c
0a60: d0 8b     bne   $09ed
0a62: 5f 65 09  jmp   $0965

0a65: 52 20     clr2  $20
0a67: f5 26 03  mov   a,$0326+x
0a6a: c4 0b     mov   $0b,a
0a6c: f5 25 03  mov   a,$0325+x
0a6f: c4 0a     mov   $0a,a
0a71: f4 7f     mov   a,$7f+x
0a73: f0 4b     beq   $0ac0
0a75: f4 80     mov   a,$80+x
0a77: d0 47     bne   $0ac0
0a79: 42 20     set2  $20
0a7b: f5 35 03  mov   a,$0335+x
0a7e: c4 04     mov   $04,a
0a80: f5 36 03  mov   a,$0336+x
0a83: c4 05     mov   $05,a
0a85: 30 18     bmi   $0a9f
0a87: f4 bf     mov   a,$bf+x
0a89: eb 04     mov   y,$04
0a8b: cf        mul   ya
0a8c: cb 06     mov   $06,y
0a8e: 8f 00 07  mov   $07,#$00
0a91: f4 bf     mov   a,$bf+x
0a93: eb 05     mov   y,$05
0a95: cf        mul   ya
0a96: 60        clrc
0a97: 7a 06     addw  ya,$06
0a99: 7a 0a     addw  ya,$0a
0a9b: da 0a     movw  $0a,ya
0a9d: 2f 21     bra   $0ac0
0a9f: 58 ff 04  eor   $04,#$ff
0aa2: 58 ff 05  eor   $05,#$ff
0aa5: 3a 04     incw  $04
0aa7: f4 bf     mov   a,$bf+x
0aa9: eb 04     mov   y,$04
0aab: cf        mul   ya
0aac: cb 06     mov   $06,y
0aae: 8f 00 07  mov   $07,#$00
0ab1: f4 bf     mov   a,$bf+x
0ab3: eb 05     mov   y,$05
0ab5: cf        mul   ya
0ab6: 7a 06     addw  ya,$06
0ab8: da 04     movw  $04,ya
0aba: ba 0a     movw  ya,$0a
0abc: 9a 04     subw  ya,$04
0abe: da 0a     movw  $0a,ya
0ac0: f5 56 03  mov   a,$0356+x
0ac3: 15 55 03  or    a,$0355+x
0ac6: f0 1e     beq   $0ae6
0ac8: f5 45 02  mov   a,$0245+x
0acb: de 9f 18  cbne  $9f+x,$0ae6
0ace: f5 36 02  mov   a,$0236+x
0ad1: 60        clrc
0ad2: 95 35 02  adc   a,$0235+x
0ad5: d5 36 02  mov   $0236+x,a
0ad8: 60        clrc
0ad9: 94 a0     adc   a,$a0+x
0adb: 3f c6 13  call  $13c6
0ade: ba 08     movw  ya,$08
0ae0: 7a 0a     addw  ya,$0a
0ae2: da 0a     movw  $0a,ya
0ae4: 42 20     set2  $20
0ae6: 3f ef 13  call  $13ef
0ae9: ba 04     movw  ya,$04
0aeb: f0 05     beq   $0af2
0aed: 7a 0a     addw  ya,$0a
0aef: 5f 3a 14  jmp   $143a

0af2: 53 20 05  bbc2  $20,$0afa
0af5: ba 0a     movw  ya,$0a
0af7: 5f 3a 14  jmp   $143a

0afa: 6f        ret

0afb: 1a 29     decw  $29
0afd: 30 02     bmi   $0b01
0aff: d0 03     bne   $0b04
0b01: 5f bd 0c  jmp   $0cbd

0b04: f4 50     mov   a,$50+x
0b06: f0 13     beq   $0b1b
0b08: 9b 50     dec   $50+x
0b0a: d0 46     bne   $0b52
0b0c: f5 a5 03  mov   a,$03a5+x
0b0f: 15 a6 03  or    a,$03a6+x
0b12: d0 07     bne   $0b1b
0b14: f5 66 02  mov   a,$0266+x
0b17: 68 7f     cmp   a,#$7f
0b19: 90 34     bcc   $0b4f
0b1b: f5 b6 03  mov   a,$03b6+x
0b1e: 75 16 02  cmp   a,$0216+x
0b21: b0 2c     bcs   $0b4f
0b23: f5 a5 03  mov   a,$03a5+x
0b26: 15 a6 03  or    a,$03a6+x
0b29: f0 27     beq   $0b52
0b2b: f5 66 02  mov   a,$0266+x
0b2e: 68 7f     cmp   a,#$7f
0b30: b0 08     bcs   $0b3a
0b32: 8d 00     mov   y,#$00
0b34: e8 01     mov   a,#$01
0b36: 5a 29     cmpw  ya,$29
0b38: f0 15     beq   $0b4f
0b3a: f5 b5 03  mov   a,$03b5+x
0b3d: 60        clrc
0b3e: 95 a5 03  adc   a,$03a5+x
0b41: d5 b5 03  mov   $03b5+x,a
0b44: f5 b6 03  mov   a,$03b6+x
0b47: 95 a6 03  adc   a,$03a6+x
0b4a: d5 b6 03  mov   $03b6+x,a
0b4d: 2f 03     bra   $0b52
0b4f: 09 1c 13  or    ($13),($1c)
0b52: 52 20     clr2  $20
0b54: e8 00     mov   a,#$00
0b56: fd        mov   y,a
0b57: da 08     movw  $08,ya
0b59: f4 7f     mov   a,$7f+x
0b5b: f0 2f     beq   $0b8c
0b5d: f4 80     mov   a,$80+x
0b5f: f0 04     beq   $0b65
0b61: 9b 80     dec   $80+x
0b63: 2f 27     bra   $0b8c
0b65: 9b 7f     dec   $7f+x
0b67: d0 0e     bne   $0b77
0b69: f5 46 03  mov   a,$0346+x
0b6c: d5 26 03  mov   $0326+x,a
0b6f: f5 45 03  mov   a,$0345+x
0b72: d5 25 03  mov   $0325+x,a
0b75: 2f 13     bra   $0b8a
0b77: f5 25 03  mov   a,$0325+x
0b7a: 60        clrc
0b7b: 95 35 03  adc   a,$0335+x
0b7e: d5 25 03  mov   $0325+x,a
0b81: f5 26 03  mov   a,$0326+x
0b84: 95 36 03  adc   a,$0336+x
0b87: d5 26 03  mov   $0326+x,a
0b8a: 42 20     set2  $20
0b8c: f5 55 02  mov   a,$0255+x
0b8f: f0 58     beq   $0be9
0b91: f4 9f     mov   a,$9f+x
0b93: 75 45 02  cmp   a,$0245+x
0b96: f0 04     beq   $0b9c
0b98: bb 9f     inc   $9f+x
0b9a: 2f 4d     bra   $0be9
0b9c: f4 cf     mov   a,$cf+x
0b9e: 75 46 02  cmp   a,$0246+x
0ba1: d0 0d     bne   $0bb0
0ba3: f5 55 02  mov   a,$0255+x
0ba6: d5 56 03  mov   $0356+x,a
0ba9: e8 00     mov   a,#$00
0bab: d5 55 03  mov   $0355+x,a
0bae: 2f 27     bra   $0bd7
0bb0: 68 00     cmp   a,#$00
0bb2: d0 0e     bne   $0bc2
0bb4: f5 66 03  mov   a,$0366+x
0bb7: d5 56 03  mov   $0356+x,a
0bba: f5 65 03  mov   a,$0365+x
0bbd: d5 55 03  mov   $0355+x,a
0bc0: 2f 13     bra   $0bd5
0bc2: f5 55 03  mov   a,$0355+x
0bc5: 60        clrc
0bc6: 95 65 03  adc   a,$0365+x
0bc9: d5 55 03  mov   $0355+x,a
0bcc: f5 56 03  mov   a,$0356+x
0bcf: 95 66 03  adc   a,$0366+x
0bd2: d5 56 03  mov   $0356+x,a
0bd5: bb cf     inc   $cf+x
0bd7: e8 00     mov   a,#$00
0bd9: d5 36 02  mov   $0236+x,a
0bdc: f4 a0     mov   a,$a0+x
0bde: 60        clrc
0bdf: 95 c0 00  adc   a,$00c0+x
0be2: d4 a0     mov   $a0+x,a
0be4: 3f c6 13  call  $13c6
0be7: 42 20     set2  $20
0be9: 3f ef 13  call  $13ef
0bec: ba 04     movw  ya,$04
0bee: d0 03     bne   $0bf3
0bf0: 53 20 0e  bbc2  $20,$0c01
0bf3: f5 26 03  mov   a,$0326+x
0bf6: fd        mov   y,a
0bf7: f5 25 03  mov   a,$0325+x
0bfa: 7a 04     addw  ya,$04
0bfc: 7a 08     addw  ya,$08
0bfe: 5f 3a 14  jmp   $143a

0c01: 6f        ret

0c02: b3 21 03  bbc5  $21,$0c08
0c05: b2 21     clr5  $21
0c07: 6f        ret

0c08: f4 3f     mov   a,$3f+x
0c0a: f0 1e     beq   $0c2a
0c0c: 9b 3f     dec   $3f+x
0c0e: d0 04     bne   $0c14
0c10: f4 40     mov   a,$40+x
0c12: 2f 10     bra   $0c24
0c14: f5 10 01  mov   a,$0110+x
0c17: 60        clrc
0c18: 95 00 01  adc   a,$0100+x
0c1b: d5 10 01  mov   $0110+x,a
0c1e: f5 11 01  mov   a,$0111+x
0c21: 95 01 01  adc   a,$0101+x
0c24: d5 11 01  mov   $0111+x,a
0c27: d5 05 02  mov   $0205+x,a
0c2a: f4 7f     mov   a,$7f+x
0c2c: d0 35     bne   $0c63
0c2e: e7 2f     mov   a,($2f+x)
0c30: 68 f3     cmp   a,#$f3
0c32: d0 2f     bne   $0c63
0c34: 3f 87 0e  call  $0e87
0c37: 3f 85 0e  call  $0e85
0c3a: d4 80     mov   $80+x,a
0c3c: 3f 85 0e  call  $0e85
0c3f: d4 7f     mov   $7f+x,a
0c41: 3f 85 0e  call  $0e85
0c44: 60        clrc
0c45: 95 56 02  adc   a,$0256+x
0c48: d5 46 03  mov   $0346+x,a
0c4b: e8 00     mov   a,#$00
0c4d: d5 45 03  mov   $0345+x,a
0c50: f4 7f     mov   a,$7f+x
0c52: f0 0f     beq   $0c63
0c54: 3f 87 0e  call  $0e87
0c57: 3f 85 0e  call  $0e85
0c5a: d5 35 03  mov   $0335+x,a
0c5d: 3f 85 0e  call  $0e85
0c60: d5 36 03  mov   $0336+x,a
0c63: f4 70     mov   a,$70+x
0c65: f0 24     beq   $0c8b
0c67: 9b 70     dec   $70+x
0c69: d0 0d     bne   $0c78
0c6b: f5 15 02  mov   a,$0215+x
0c6e: d5 e6 02  mov   $02e6+x,a
0c71: e8 00     mov   a,#$00
0c73: d5 e5 02  mov   $02e5+x,a
0c76: 2f 13     bra   $0c8b
0c78: f5 e5 02  mov   a,$02e5+x
0c7b: 60        clrc
0c7c: 95 f5 02  adc   a,$02f5+x
0c7f: d5 e5 02  mov   $02e5+x,a
0c82: f5 e6 02  mov   a,$02e6+x
0c85: 95 f6 02  adc   a,$02f6+x
0c88: d5 e6 02  mov   $02e6+x,a
0c8b: f5 16 02  mov   a,$0216+x
0c8e: f0 05     beq   $0c95
0c90: e8 00     mov   a,#$00
0c92: 3f a5 14  call  $14a5
0c95: f4 6f     mov   a,$6f+x
0c97: f0 23     beq   $0cbc
0c99: 9b 6f     dec   $6f+x
0c9b: f0 14     beq   $0cb1
0c9d: f5 05 03  mov   a,$0305+x
0ca0: 60        clrc
0ca1: 95 15 03  adc   a,$0315+x
0ca4: d5 05 03  mov   $0305+x,a
0ca7: f5 06 03  mov   a,$0306+x
0caa: 95 16 03  adc   a,$0316+x
0cad: d5 06 03  mov   $0306+x,a
0cb0: 6f        ret

0cb1: f5 26 02  mov   a,$0226+x
0cb4: d5 06 03  mov   $0306+x,a
0cb7: e8 00     mov   a,#$00
0cb9: d5 05 03  mov   $0305+x,a
0cbc: 6f        ret

0cbd: 3f 85 0e  call  $0e85
0cc0: c4 08     mov   $08,a
0cc2: 28 7f     and   a,#$7f
0cc4: 68 60     cmp   a,#$60
0cc6: 90 03     bcc   $0ccb
0cc8: 5f 5b 0e  jmp   $0e5b

0ccb: 33 21 03  bbc1  $21,$0cd1
0cce: 09 1c 13  or    ($13),($1c)
0cd1: f3 08 05  bbc7  $08,$0cd9
0cd4: f5 65 02  mov   a,$0265+x
0cd7: 2f 03     bra   $0cdc
0cd9: 3f 85 0e  call  $0e85
0cdc: d5 65 02  mov   $0265+x,a
0cdf: 8d 00     mov   y,#$00
0ce1: 7a 29     addw  ya,$29
0ce3: da 29     movw  $29,ya
0ce5: f0 02     beq   $0ce9
0ce7: 10 18     bpl   $0d01
0ce9: 3f 85 0e  call  $0e85
0cec: fd        mov   y,a
0ced: 10 03     bpl   $0cf2
0cef: f5 66 02  mov   a,$0266+x
0cf2: d5 66 02  mov   $0266+x,a
0cf5: d5 75 02  mov   $0275+x,a
0cf8: dd        mov   a,y
0cf9: 30 03     bmi   $0cfe
0cfb: 3f 87 0e  call  $0e87
0cfe: 5f bd 0c  jmp   $0cbd

0d01: f5 75 02  mov   a,$0275+x
0d04: 68 7f     cmp   a,#$7f
0d06: f0 06     beq   $0d0e
0d08: 09 1c 12  or    ($12),($1c)
0d0b: 09 1c 1b  or    ($1b),($1c)
0d0e: 3f 85 0e  call  $0e85
0d11: fd        mov   y,a
0d12: 10 03     bpl   $0d17
0d14: f5 66 02  mov   a,$0266+x
0d17: d5 66 02  mov   $0266+x,a
0d1a: d5 75 02  mov   $0275+x,a
0d1d: dd        mov   a,y
0d1e: 30 03     bmi   $0d23
0d20: 3f 85 0e  call  $0e85
0d23: 28 7f     and   a,#$7f
0d25: d5 96 02  mov   $0296+x,a
0d28: d5 16 02  mov   $0216+x,a
0d2b: d0 08     bne   $0d35
0d2d: e4 1c     mov   a,$1c
0d2f: 4e 12 00  tclr1 $0012
0d32: 4e 1b 00  tclr1 $001b
0d35: 33 21 0d  bbc1  $21,$0d45
0d38: e4 08     mov   a,$08
0d3a: 28 7f     and   a,#$7f
0d3c: d5 a6 02  mov   $02a6+x,a
0d3f: 3f 32 0f  call  $0f32
0d42: 8f 3c 08  mov   $08,#$3c
0d45: f5 65 02  mov   a,$0265+x
0d48: fd        mov   y,a
0d49: f5 66 02  mov   a,$0266+x
0d4c: 68 7f     cmp   a,#$7f
0d4e: d0 05     bne   $0d55
0d50: f5 65 02  mov   a,$0265+x
0d53: 2f 07     bra   $0d5c
0d55: 1c        asl   a
0d56: cf        mul   ya
0d57: dd        mov   a,y
0d58: d0 02     bne   $0d5c
0d5a: e8 01     mov   a,#$01
0d5c: d4 50     mov   $50+x,a
0d5e: f5 25 03  mov   a,$0325+x
0d61: c4 0c     mov   $0c,a
0d63: f5 26 03  mov   a,$0326+x
0d66: c4 0d     mov   $0d,a
0d68: f5 51 01  mov   a,$0151+x
0d6b: c4 05     mov   $05,a
0d6d: f5 50 01  mov   a,$0150+x
0d70: c4 04     mov   $04,a
0d72: f5 61 01  mov   a,$0161+x
0d75: c4 07     mov   $07,a
0d77: f5 60 01  mov   a,$0160+x
0d7a: c4 06     mov   $06,a
0d7c: f5 96 03  mov   a,$0396+x
0d7f: c4 0b     mov   $0b,a
0d81: f5 95 03  mov   a,$0395+x
0d84: c4 0a     mov   $0a,a
0d86: e4 08     mov   a,$08
0d88: 60        clrc
0d89: 95 56 02  adc   a,$0256+x
0d8c: fd        mov   y,a
0d8d: e8 00     mov   a,#$00
0d8f: 7a 04     addw  ya,$04
0d91: 7a 06     addw  ya,$06
0d93: 7a 0a     addw  ya,$0a
0d95: d5 25 03  mov   $0325+x,a
0d98: dd        mov   a,y
0d99: 28 7f     and   a,#$7f
0d9b: d5 26 03  mov   $0326+x,a
0d9e: e8 00     mov   a,#$00
0da0: d4 a0     mov   $a0+x,a
0da2: d4 9f     mov   $9f+x,a
0da4: d4 cf     mov   $cf+x,a
0da6: d5 55 03  mov   $0355+x,a
0da9: d5 56 03  mov   $0356+x,a
0dac: d5 b6 03  mov   $03b6+x,a
0daf: d5 b5 03  mov   $03b5+x,a
0db2: f4 8f     mov   a,$8f+x
0db4: d4 7f     mov   $7f+x,a
0db6: f0 33     beq   $0deb
0db8: f4 90     mov   a,$90+x
0dba: d4 80     mov   $80+x,a
0dbc: 83 21 15  bbs4  $21,$0dd4
0dbf: f5 25 03  mov   a,$0325+x
0dc2: d5 45 03  mov   $0345+x,a
0dc5: f5 26 03  mov   a,$0326+x
0dc8: d5 46 03  mov   $0346+x,a
0dcb: 80        setc
0dcc: b5 06 02  sbc   a,$0206+x
0dcf: d5 26 03  mov   $0326+x,a
0dd2: 2f 17     bra   $0deb
0dd4: f5 26 03  mov   a,$0326+x
0dd7: c4 05     mov   $05,a
0dd9: f5 25 03  mov   a,$0325+x
0ddc: c4 04     mov   $04,a
0dde: e4 0d     mov   a,$0d
0de0: d5 26 03  mov   $0326+x,a
0de3: e4 0c     mov   a,$0c
0de5: d5 25 03  mov   $0325+x,a
0de8: 3f 69 13  call  $1369
0deb: f5 26 03  mov   a,$0326+x
0dee: fd        mov   y,a
0def: f5 25 03  mov   a,$0325+x
0df2: 5f 3a 14  jmp   $143a

; vcmd dispatch table
0df5: dw $0e8e  ; 
0df7: dw $0e93  ; 
0df9: dw $0eba  ; 
0dfb: dw $0ede  ; 
0dfd: dw $0ef8  ; 
0dff: dw $0f88  ; 
0e01: dw $0fa8  ; 
0e03: dw $0fce  ; 
0e05: dw $0fea  ; 
0e07: dw $102f  ; 
0e09: dw $0ff7  ; 
0e0b: dw $1014  ; 
0e0d: dw $10af  ; 
0e0f: dw $10b9  ; 
0e11: dw $10de  ; 
0e13: dw $10e4  ; 
0e15: dw $10ed  ; 
0e17: dw $10fb  ; 
0e19: dw $1116  ; 
0e1b: dw $1121  ; 
0e1d: dw $1133  ; 
0e1f: dw $1155  ; 
0e21: dw $115e  ; 
0e23: dw $1180  ; 
0e25: dw $11fd  ; 
0e27: dw $120f  ; 
0e29: dw $1246  ; 
0e2b: dw $1261  ; 
0e2d: dw $127c  ; 
0e2f: dw $12e0  ; 
0e31: dw $12e5  ; 
0e33: dw $12f1  ; 
0e35: dw $12ff  ; 
0e37: dw $131b  ; 

; vcmd lengths
0e39: 00 00 01
0e3c: 02 01
0e3e: 01
0e3f: 03 03 00
0e42: 03 00 03
0e45: 01
0e46: 02 01
0e48: 03 01 02
0e4b: 01
0e4c: 03 01 03
0e4f: 03 03 00
0e52: 00
0e53: 02 01
0e55: 03 01 01
0e58: 02 02
0e5a: 00

0e5b: e4 08     mov   a,$08
0e5d: 8f de 04  mov   $04,#$de
0e60: 68 e0     cmp   a,#$e0
0e62: b0 0c     bcs   $0e70
0e64: 8f 60 04  mov   $04,#$60
0e67: 68 62     cmp   a,#$62
0e69: 90 05     bcc   $0e70
0e6b: a2 21     set5  $21
0e6d: 5f e3 19  jmp   $19e3

0e70: 80        setc
0e71: a4 04     sbc   a,$04
0e73: 1c        asl   a
0e74: fd        mov   y,a
0e75: f6 f6 0d  mov   a,$0df6+y
0e78: 2d        push  a
0e79: f6 f5 0d  mov   a,$0df5+y
0e7c: 2d        push  a
0e7d: dd        mov   a,y
0e7e: 5c        lsr   a
0e7f: fd        mov   y,a
0e80: f6 39 0e  mov   a,$0e39+y
0e83: f0 08     beq   $0e8d
0e85: e7 2f     mov   a,($2f+x)
0e87: bb 2f     inc   $2f+x
0e89: d0 02     bne   $0e8d
0e8b: bb 30     inc   $30+x
0e8d: 6f        ret

0e8e: 22 21     set1  $21
0e90: 5f bd 0c  jmp   $0cbd

0e93: 32 21     clr1  $21
0e95: 5f bd 0c  jmp   $0cbd

0e98: fd        mov   y,a
0e99: f5 66 02  mov   a,$0266+x
0e9c: d5 75 02  mov   $0275+x,a
0e9f: dd        mov   a,y
0ea0: d5 66 02  mov   $0266+x,a
0ea3: 5f bd 0c  jmp   $0cbd

0ea6: fd        mov   y,a
0ea7: f5 66 02  mov   a,$0266+x
0eaa: d5 75 02  mov   $0275+x,a
0ead: dd        mov   a,y
0eae: d5 66 02  mov   $0266+x,a
0eb1: 3f 85 0e  call  $0e85
0eb4: d5 96 02  mov   $0296+x,a
0eb7: 5f bd 0c  jmp   $0cbd

0eba: d5 65 02  mov   $0265+x,a
0ebd: 09 1c 13  or    ($13),($1c)
0ec0: e8 00     mov   a,#$00
0ec2: d5 66 02  mov   $0266+x,a
0ec5: d5 75 02  mov   $0275+x,a
0ec8: d4 50     mov   $50+x,a
0eca: d5 16 02  mov   $0216+x,a
0ecd: f5 65 02  mov   a,$0265+x
0ed0: 8d 00     mov   y,#$00
0ed2: 7a 29     addw  ya,$29
0ed4: da 29     movw  $29,ya
0ed6: f0 03     beq   $0edb
0ed8: 30 01     bmi   $0edb
0eda: 6f        ret

0edb: 5f bd 0c  jmp   $0cbd

0ede: d5 65 02  mov   $0265+x,a
0ee1: 2d        push  a
0ee2: 3f 85 0e  call  $0e85
0ee5: d5 66 02  mov   $0266+x,a
0ee8: d5 75 02  mov   $0275+x,a
0eeb: ee        pop   y
0eec: 1c        asl   a
0eed: cf        mul   ya
0eee: dd        mov   a,y
0eef: d0 02     bne   $0ef3
0ef1: e8 01     mov   a,#$01
0ef3: d4 50     mov   $50+x,a
0ef5: 5f cd 0e  jmp   $0ecd

0ef8: 09 1c 13  or    ($13),($1c)
0efb: d5 a6 02  mov   $02a6+x,a
0efe: 68 20     cmp   a,#$20
0f00: b0 04     bcs   $0f06
0f02: 68 19     cmp   a,#$19
0f04: b0 11     bcs   $0f17
0f06: e8 aa     mov   a,#$aa
0f08: c4 04     mov   $04,a
0f0a: e8 06     mov   a,#$06
0f0c: c4 05     mov   $05,a
0f0e: f5 a6 02  mov   a,$02a6+x
0f11: 3f 3d 0f  call  $0f3d
0f14: 5f bd 0c  jmp   $0cbd

0f17: e5 26 00  mov   a,$0026
0f1a: 1c        asl   a
0f1b: fd        mov   y,a
0f1c: f6 98 06  mov   a,$0698+y
0f1f: c4 04     mov   $04,a
0f21: f6 99 06  mov   a,$0699+y
0f24: c4 05     mov   $05,a
0f26: f5 a6 02  mov   a,$02a6+x
0f29: 80        setc
0f2a: a8 19     sbc   a,#$19
0f2c: 3f 3d 0f  call  $0f3d
0f2f: 5f bd 0c  jmp   $0cbd

0f32: e8 aa     mov   a,#$aa
0f34: c4 04     mov   $04,a
0f36: e8 07     mov   a,#$07
0f38: c4 05     mov   $05,a
0f3a: f5 a6 02  mov   a,$02a6+x
0f3d: 8d 08     mov   y,#$08
0f3f: cf        mul   ya
0f40: 7a 04     addw  ya,$04
0f42: da 04     movw  $04,ya
0f44: 8d 00     mov   y,#$00
0f46: e8 04     mov   a,#$04
0f48: 04 24     or    a,$24
0f4a: c4 06     mov   $06,a
0f4c: c5 f2 00  mov   $00f2,a
0f4f: f7 04     mov   a,($04)+y
0f51: c5 f3 00  mov   $00f3,a
0f54: fc        inc   y
0f55: f7 04     mov   a,($04)+y
0f57: d5 85 02  mov   $0285+x,a
0f5a: fc        inc   y
0f5b: f7 04     mov   a,($04)+y
0f5d: d5 86 02  mov   $0286+x,a
0f60: 8f 03 07  mov   $07,#$03
0f63: fc        inc   y
0f64: ab 06     inc   $06
0f66: e4 06     mov   a,$06
0f68: c5 f2 00  mov   $00f2,a
0f6b: f7 04     mov   a,($04)+y
0f6d: c5 f3 00  mov   $00f3,a
0f70: 6e 07 f0  dbnz  $07,$0f63
0f73: fc        inc   y
0f74: 63 21 0a  bbs3  $21,$0f81
0f77: f7 04     mov   a,($04)+y
0f79: d5 06 03  mov   $0306+x,a
0f7c: e8 00     mov   a,#$00
0f7e: d5 05 03  mov   $0305+x,a
0f81: fc        inc   y
0f82: f7 04     mov   a,($04)+y
0f84: d5 95 02  mov   $0295+x,a
0f87: 6f        ret

0f88: 68 15     cmp   a,#$15
0f8a: f0 12     beq   $0f9e
0f8c: 68 16     cmp   a,#$16
0f8e: f0 13     beq   $0fa3
0f90: d5 06 03  mov   $0306+x,a
0f93: e8 00     mov   a,#$00
0f95: d5 05 03  mov   $0305+x,a
0f98: d5 6f 00  mov   $006f+x,a
0f9b: 5f bd 0c  jmp   $0cbd

0f9e: 62 21     set3  $21
0fa0: 5f bd 0c  jmp   $0cbd

0fa3: 72 21     clr3  $21
0fa5: 5f bd 0c  jmp   $0cbd

0fa8: d5 45 02  mov   $0245+x,a
0fab: 3f 85 0e  call  $0e85
0fae: d5 c0 00  mov   $00c0+x,a
0fb1: fd        mov   y,a
0fb2: f5 05 02  mov   a,$0205+x
0fb5: cf        mul   ya
0fb6: dd        mov   a,y
0fb7: d5 35 02  mov   $0235+x,a
0fba: 3f 85 0e  call  $0e85
0fbd: d5 56 03  mov   $0356+x,a
0fc0: d5 55 02  mov   $0255+x,a
0fc3: e8 00     mov   a,#$00
0fc5: d5 55 03  mov   $0355+x,a
0fc8: d5 46 02  mov   $0246+x,a
0fcb: 5f bd 0c  jmp   $0cbd

0fce: d5 76 02  mov   $0276+x,a
0fd1: 3f 85 0e  call  $0e85
0fd4: d5 86 03  mov   $0386+x,a
0fd7: 3f 85 0e  call  $0e85
0fda: d5 85 03  mov   $0385+x,a
0fdd: e8 00     mov   a,#$00
0fdf: d4 60     mov   $60+x,a
0fe1: d5 75 03  mov   $0375+x,a
0fe4: d5 76 03  mov   $0376+x,a
0fe7: 5f bd 0c  jmp   $0cbd

0fea: f4 2f     mov   a,$2f+x
0fec: d5 70 01  mov   $0170+x,a
0fef: f4 30     mov   a,$30+x
0ff1: d5 71 01  mov   $0171+x,a
0ff4: 5f bd 0c  jmp   $0cbd

0ff7: f4 2f     mov   a,$2f+x
0ff9: d5 80 01  mov   $0180+x,a
0ffc: f4 30     mov   a,$30+x
0ffe: d5 81 01  mov   $0181+x,a
1001: e8 00     mov   a,#$00
1003: d4 5f     mov   $5f+x,a
1005: d5 40 01  mov   $0140+x,a
1008: d5 41 01  mov   $0141+x,a
100b: d5 60 01  mov   $0160+x,a
100e: d5 61 01  mov   $0161+x,a
1011: 5f bd 0c  jmp   $0cbd

1014: fd        mov   y,a
1015: 7d        mov   a,x
1016: 60        clrc
1017: 88 10     adc   a,#$10
1019: 5d        mov   x,a
101a: dd        mov   a,y
101b: 68 00     cmp   a,#$00
101d: f0 32     beq   $1051
101f: bb 4f     inc   $4f+x
1021: de 4f 2d  cbne  $4f+x,$1051
1024: f8 22     mov   x,$22
1026: 3f 87 0e  call  $0e87
1029: 3f 87 0e  call  $0e87
102c: 5f bd 0c  jmp   $0cbd

102f: 68 00     cmp   a,#$00
1031: f0 1e     beq   $1051
1033: bb 4f     inc   $4f+x
1035: de 4f 19  cbne  $4f+x,$1051
1038: 3f 87 0e  call  $0e87
103b: 3f 87 0e  call  $0e87
103e: e8 00     mov   a,#$00
1040: d4 4f     mov   $4f+x,a
1042: d5 30 01  mov   $0130+x,a
1045: d5 31 01  mov   $0131+x,a
1048: d5 50 01  mov   $0150+x,a
104b: d5 51 01  mov   $0151+x,a
104e: 5f bd 0c  jmp   $0cbd

1051: f5 30 01  mov   a,$0130+x
1054: c4 04     mov   $04,a
1056: f5 31 01  mov   a,$0131+x
1059: c4 05     mov   $05,a
105b: 4d        push  x
105c: f8 22     mov   x,$22
105e: 3f 85 0e  call  $0e85
1061: ce        pop   x
1062: 8d 00     mov   y,#$00
1064: 08 00     or    a,#$00
1066: f0 0c     beq   $1074
1068: 10 01     bpl   $106b
106a: dc        dec   y
106b: 7a 04     addw  ya,$04
106d: d5 30 01  mov   $0130+x,a
1070: dd        mov   a,y
1071: d5 31 01  mov   $0131+x,a
1074: 4d        push  x
1075: f8 22     mov   x,$22
1077: 3f 85 0e  call  $0e85
107a: ce        pop   x
107b: 68 00     cmp   a,#$00
107d: f0 1d     beq   $109c
107f: 8d 00     mov   y,#$00
1081: 1c        asl   a
1082: 90 01     bcc   $1085
1084: dc        dec   y
1085: cb 04     mov   $04,y
1087: 1c        asl   a
1088: 2b 04     rol   $04
108a: 1c        asl   a
108b: 2b 04     rol   $04
108d: 60        clrc
108e: 95 50 01  adc   a,$0150+x
1091: d5 50 01  mov   $0150+x,a
1094: e4 04     mov   a,$04
1096: 95 51 01  adc   a,$0151+x
1099: d5 51 01  mov   $0151+x,a
109c: eb 22     mov   y,$22
109e: f5 70 01  mov   a,$0170+x
10a1: d6 2f 00  mov   $002f+y,a
10a4: f5 71 01  mov   a,$0171+x
10a7: d6 30 00  mov   $0030+y,a
10aa: f8 22     mov   x,$22
10ac: 5f bd 0c  jmp   $0cbd

10af: d5 05 02  mov   $0205+x,a
10b2: e8 00     mov   a,#$00
10b4: d4 3f     mov   $3f+x,a
10b6: 5f bd 0c  jmp   $0cbd

10b9: d4 3f     mov   $3f+x,a
10bb: 2d        push  a
10bc: 3f 85 0e  call  $0e85
10bf: d4 40     mov   $40+x,a
10c1: 80        setc
10c2: b5 05 02  sbc   a,$0205+x
10c5: ce        pop   x
10c6: 3f 4c 13  call  $134c
10c9: d5 00 01  mov   $0100+x,a
10cc: dd        mov   a,y
10cd: d5 01 01  mov   $0101+x,a
10d0: f5 05 02  mov   a,$0205+x
10d3: d5 11 01  mov   $0111+x,a
10d6: e8 00     mov   a,#$00
10d8: d5 10 01  mov   $0110+x,a
10db: 5f bd 0c  jmp   $0cbd

10de: d5 56 02  mov   $0256+x,a
10e1: 5f bd 0c  jmp   $0cbd

10e4: 3f 87 0e  call  $0e87
10e7: 3f 87 0e  call  $0e87
10ea: 5f bd 0c  jmp   $0cbd

10ed: d5 e6 02  mov   $02e6+x,a
10f0: e8 00     mov   a,#$00
10f2: d5 e5 02  mov   $02e5+x,a
10f5: d5 70 00  mov   $0070+x,a
10f8: 5f bd 0c  jmp   $0cbd

10fb: d4 70     mov   $70+x,a
10fd: 2d        push  a
10fe: 3f 85 0e  call  $0e85
1101: d5 15 02  mov   $0215+x,a
1104: 80        setc
1105: b5 e6 02  sbc   a,$02e6+x
1108: ce        pop   x
1109: 3f 4c 13  call  $134c
110c: d5 f5 02  mov   $02f5+x,a
110f: dd        mov   a,y
1110: d5 f6 02  mov   $02f6+x,a
1113: 5f bd 0c  jmp   $0cbd

1116: d4 8f     mov   $8f+x,a
1118: 82 21     set4  $21
111a: e8 00     mov   a,#$00
111c: d4 90     mov   $90+x,a
111e: 5f bd 0c  jmp   $0cbd

1121: d4 90     mov   $90+x,a
1123: 3f 85 0e  call  $0e85
1126: d4 8f     mov   $8f+x,a
1128: 3f 85 0e  call  $0e85
112b: d5 06 02  mov   $0206+x,a
112e: 92 21     clr4  $21
1130: 5f bd 0c  jmp   $0cbd

1133: 8f 00 04  mov   $04,#$00
1136: 08 00     or    a,#$00
1138: 30 05     bmi   $113f
113a: 8d 04     mov   y,#$04
113c: cf        mul   ya
113d: 2f 0c     bra   $114b
113f: 48 ff     eor   a,#$ff
1141: bc        inc   a
1142: 8d 04     mov   y,#$04
1144: cf        mul   ya
1145: da 04     movw  $04,ya
1147: ba 0e     movw  ya,$0e
1149: 9a 04     subw  ya,$04
114b: d5 95 03  mov   $0395+x,a
114e: dd        mov   a,y
114f: d5 96 03  mov   $0396+x,a
1152: 5f bd 0c  jmp   $0cbd

1155: 3f 87 0e  call  $0e87
1158: 3f 87 0e  call  $0e87
115b: 5f bd 0c  jmp   $0cbd

115e: c4 18     mov   $18,a
1160: f0 0f     beq   $1171
1162: 3f 85 0e  call  $0e85
1165: c4 10     mov   $10,a
1167: 3f 85 0e  call  $0e85
116a: c4 11     mov   $11,a
116c: b2 14     clr5  $14
116e: 5f bd 0c  jmp   $0cbd

1171: c4 10     mov   $10,a
1173: c4 11     mov   $11,a
1175: a2 14     set5  $14
1177: 3f 87 0e  call  $0e87
117a: 3f 87 0e  call  $0e87
117d: 5f bd 0c  jmp   $0cbd

1180: 3f a6 11  call  $11a6
1183: 3f 85 0e  call  $0e85
1186: c4 15     mov   $15,a
1188: 3f 85 0e  call  $0e85
118b: bc        inc   a
118c: 8d 08     mov   y,#$08
118e: cf        mul   ya
118f: 5d        mov   x,a
1190: 8d 08     mov   y,#$08
1192: f6 a0 16  mov   a,$16a0+y
1195: c5 f2 00  mov   $00f2,a
1198: f5 80 16  mov   a,$1680+x
119b: c5 f3 00  mov   $00f3,a
119e: 1d        dec   x
119f: fe f1     dbnz  y,$1192
11a1: f8 22     mov   x,$22
11a3: 5f bd 0c  jmp   $0cbd

11a6: c4 1a     mov   $1a,a
11a8: 8d 7d     mov   y,#$7d
11aa: cc f2 00  mov   $00f2,y
11ad: e5 f3 00  mov   a,$00f3
11b0: 64 1a     cmp   a,$1a
11b2: f0 31     beq   $11e5
11b4: 28 0f     and   a,#$0f
11b6: 48 ff     eor   a,#$ff
11b8: f3 19 03  bbc7  $19,$11be
11bb: 60        clrc
11bc: 84 19     adc   a,$19
11be: c4 19     mov   $19,a
11c0: 8d 04     mov   y,#$04
11c2: f6 6c 15  mov   a,$156c+y
11c5: c5 f2 00  mov   $00f2,a
11c8: e8 00     mov   a,#$00
11ca: c5 f3 00  mov   $00f3,a
11cd: fe f3     dbnz  y,$11c2
11cf: e4 14     mov   a,$14
11d1: 08 20     or    a,#$20
11d3: 8d 6c     mov   y,#$6c
11d5: cc f2 00  mov   $00f2,y
11d8: c5 f3 00  mov   $00f3,a
11db: e4 1a     mov   a,$1a
11dd: 8d 7d     mov   y,#$7d
11df: cc f2 00  mov   $00f2,y
11e2: c5 f3 00  mov   $00f3,a
11e5: 68 00     cmp   a,#$00
11e7: f0 0b     beq   $11f4
11e9: 1c        asl   a
11ea: c4 1a     mov   $1a,a
11ec: 1c        asl   a
11ed: 1c        asl   a
11ee: 48 ff     eor   a,#$ff
11f0: bc        inc   a
11f1: 60        clrc
11f2: 88 00     adc   a,#$00
11f4: 8d 6d     mov   y,#$6d
11f6: cc f2 00  mov   $00f2,y
11f9: c5 f3 00  mov   $00f3,a
11fc: 6f        ret

11fd: f4 2f     mov   a,$2f+x
11ff: d5 c5 02  mov   $02c5+x,a
1202: f4 30     mov   a,$30+x
1204: d5 c6 02  mov   $02c6+x,a
1207: e8 c0     mov   a,#$c0
1209: 4e 21 00  tclr1 $0021
120c: 5f bd 0c  jmp   $0cbd

120f: c3 21 08  bbs6  $21,$121a
1212: e3 21 20  bbs7  $21,$1235
1215: c2 21     set6  $21
1217: 5f bd 0c  jmp   $0cbd

121a: d2 21     clr6  $21
121c: e2 21     set7  $21
121e: f4 2f     mov   a,$2f+x
1220: d5 d5 02  mov   $02d5+x,a
1223: f4 30     mov   a,$30+x
1225: d5 d6 02  mov   $02d6+x,a
1228: f5 c5 02  mov   a,$02c5+x
122b: d4 2f     mov   $2f+x,a
122d: f5 c6 02  mov   a,$02c6+x
1230: d4 30     mov   $30+x,a
1232: 5f bd 0c  jmp   $0cbd

1235: c2 21     set6  $21
1237: f2 21     clr7  $21
1239: f5 d5 02  mov   a,$02d5+x
123c: d4 2f     mov   $2f+x,a
123e: f5 d6 02  mov   a,$02d6+x
1241: d4 30     mov   $30+x,a
1243: 5f bd 0c  jmp   $0cbd

1246: d4 6f     mov   $6f+x,a
1248: 2d        push  a
1249: 3f 85 0e  call  $0e85
124c: d5 26 02  mov   $0226+x,a
124f: 80        setc
1250: b5 06 03  sbc   a,$0306+x
1253: ce        pop   x
1254: 3f 4c 13  call  $134c
1257: d5 15 03  mov   $0315+x,a
125a: dd        mov   a,y
125b: d5 16 03  mov   $0316+x,a
125e: 5f bd 0c  jmp   $0cbd

1261: d5 46 02  mov   $0246+x,a
1264: 2d        push  a
1265: 8d 00     mov   y,#$00
1267: f5 55 02  mov   a,$0255+x
126a: ce        pop   x
126b: 9e        div   ya,x
126c: 2d        push  a
126d: e8 00     mov   a,#$00
126f: 9e        div   ya,x
1270: f8 22     mov   x,$22
1272: d5 65 03  mov   $0365+x,a
1275: ae        pop   a
1276: d5 66 03  mov   $0366+x,a
1279: 5f bd 0c  jmp   $0cbd

127c: 8d 00     mov   y,#$00
127e: 68 a0     cmp   a,#$a0
1280: b0 34     bcs   $12b6
1282: cd 0a     mov   x,#$0a
1284: 9e        div   ya,x
1285: c4 04     mov   $04,a
1287: dd        mov   a,y
1288: 28 07     and   a,#$07
128a: 9f        xcn   a
128b: 04 04     or    a,$04
128d: 08 80     or    a,#$80
128f: fd        mov   y,a
1290: e8 05     mov   a,#$05
1292: 04 24     or    a,$24
1294: 5d        mov   x,a
1295: c9 f2 00  mov   $00f2,x
1298: cc f3 00  mov   $00f3,y
129b: 3d        inc   x
129c: c9 f2 00  mov   $00f2,x
129f: f8 22     mov   x,$22
12a1: 3f 85 0e  call  $0e85
12a4: 8d 00     mov   y,#$00
12a6: cd 1e     mov   x,#$1e
12a8: 9e        div   ya,x
12a9: fc        inc   y
12aa: fc        inc   y
12ab: cb 04     mov   $04,y
12ad: 9f        xcn   a
12ae: 1c        asl   a
12af: 04 04     or    a,$04
12b1: c5 f3 00  mov   $00f3,a
12b4: 2f 18     bra   $12ce
12b6: e8 05     mov   a,#$05
12b8: 04 24     or    a,$24
12ba: 5d        mov   x,a
12bb: c9 f2 00  mov   $00f2,x
12be: cc f3 00  mov   $00f3,y
12c1: 3d        inc   x
12c2: 3d        inc   x
12c3: c9 f2 00  mov   $00f2,x
12c6: f8 22     mov   x,$22
12c8: 3f 85 0e  call  $0e85
12cb: c5 f3 00  mov   $00f3,a
12ce: f8 22     mov   x,$22
12d0: 3f 85 0e  call  $0e85
12d3: 8d 10     mov   y,#$10
12d5: cf        mul   ya
12d6: d5 a5 03  mov   $03a5+x,a
12d9: dd        mov   a,y
12da: d5 a6 03  mov   $03a6+x,a
12dd: 5f bd 0c  jmp   $0cbd

12e0: c4 16     mov   $16,a
12e2: 5f bd 0c  jmp   $0cbd

12e5: d5 35 03  mov   $0335+x,a
12e8: 3f 85 0e  call  $0e85
12eb: d5 36 03  mov   $0336+x,a
12ee: 5f bd 0c  jmp   $0cbd

12f1: c4 04     mov   $04,a
12f3: 3f 85 0e  call  $0e85
12f6: d4 30     mov   $30+x,a
12f8: e4 04     mov   a,$04
12fa: d4 2f     mov   $2f+x,a
12fc: 5f bd 0c  jmp   $0cbd

12ff: c4 04     mov   $04,a
1301: 3f 85 0e  call  $0e85
1304: c4 05     mov   $05,a
1306: f4 2f     mov   a,$2f+x
1308: d5 20 01  mov   $0120+x,a
130b: f4 30     mov   a,$30+x
130d: d5 21 01  mov   $0121+x,a
1310: ba 04     movw  ya,$04
1312: d4 2f     mov   $2f+x,a
1314: db 30     mov   $30+x,y
1316: 42 21     set2  $21
1318: 5f bd 0c  jmp   $0cbd

131b: 53 21 0f  bbc2  $21,$132d
131e: 52 21     clr2  $21
1320: f5 20 01  mov   a,$0120+x
1323: d4 2f     mov   $2f+x,a
1325: f5 21 01  mov   a,$0121+x
1328: d4 30     mov   $30+x,a
132a: 5f bd 0c  jmp   $0cbd

132d: c8 00     cmp   x,#$00
132f: d0 07     bne   $1338
1331: 78 90 d0  cmp   $d0,#$90
1334: d0 02     bne   $1338
1336: 02 28     set0  $28
1338: e8 00     mov   a,#$00
133a: d4 bf     mov   $bf+x,a
133c: d4 d0     mov   $d0+x,a
133e: d5 25 02  mov   $0225+x,a
1341: e4 1c     mov   a,$1c
1343: 0e 13 00  tset1 $0013
1346: 4e 1b 00  tclr1 $001b
1349: a2 21     set5  $21
134b: 6f        ret

134c: ed        notc
134d: 6b 04     ror   $04
134f: 10 03     bpl   $1354
1351: 48 ff     eor   a,#$ff
1353: bc        inc   a
1354: 8d 00     mov   y,#$00
1356: 9e        div   ya,x
1357: 2d        push  a
1358: e8 00     mov   a,#$00
135a: 9e        div   ya,x
135b: ee        pop   y
135c: f8 22     mov   x,$22
135e: f3 04 07  bbc7  $04,$1368
1361: da 04     movw  $04,ya
1363: e8 00     mov   a,#$00
1365: fd        mov   y,a
1366: 9a 04     subw  ya,$04
1368: 6f        ret

1369: f4 7f     mov   a,$7f+x
136b: f0 58     beq   $13c5
136d: f5 26 03  mov   a,$0326+x
1370: c4 07     mov   $07,a
1372: f5 25 03  mov   a,$0325+x
1375: c4 06     mov   $06,a
1377: ba 04     movw  ya,$04
1379: ad 60     cmp   y,#$60
137b: 90 0c     bcc   $1389
137d: ad 80     cmp   y,#$80
137f: 90 04     bcc   $1385
1381: ba 0e     movw  ya,$0e
1383: 2f 04     bra   $1389
1385: 8d 5f     mov   y,#$5f
1387: e8 ff     mov   a,#$ff
1389: d5 45 03  mov   $0345+x,a
138c: 2d        push  a
138d: dd        mov   a,y
138e: d5 46 03  mov   $0346+x,a
1391: f4 7f     mov   a,$7f+x
1393: 5d        mov   x,a
1394: ae        pop   a
1395: 9a 06     subw  ya,$06
1397: da 04     movw  $04,ya
1399: ed        notc
139a: 6b 06     ror   $06
139c: 10 0a     bpl   $13a8
139e: 58 ff 04  eor   $04,#$ff
13a1: 58 ff 05  eor   $05,#$ff
13a4: 3a 04     incw  $04
13a6: ba 04     movw  ya,$04
13a8: 8d 00     mov   y,#$00
13aa: e4 05     mov   a,$05
13ac: 9e        div   ya,x
13ad: 2d        push  a
13ae: e4 04     mov   a,$04
13b0: 9e        div   ya,x
13b1: ee        pop   y
13b2: f3 06 07  bbc7  $06,$13bc
13b5: da 04     movw  $04,ya
13b7: e8 00     mov   a,#$00
13b9: fd        mov   y,a
13ba: 9a 04     subw  ya,$04
13bc: f8 22     mov   x,$22
13be: d5 35 03  mov   $0335+x,a
13c1: dd        mov   a,y
13c2: d5 36 03  mov   $0336+x,a
13c5: 6f        ret

13c6: c4 04     mov   $04,a
13c8: 5c        lsr   a
13c9: 28 3f     and   a,#$3f
13cb: fd        mov   y,a
13cc: f6 a9 16  mov   a,$16a9+y
13cf: fd        mov   y,a
13d0: f5 56 03  mov   a,$0356+x
13d3: cf        mul   ya
13d4: da 08     movw  $08,ya
13d6: f5 55 02  mov   a,$0255+x
13d9: 30 08     bmi   $13e3
13db: 4b 09     lsr   $09
13dd: 6b 08     ror   $08
13df: 4b 09     lsr   $09
13e1: 6b 08     ror   $08
13e3: f3 04 08  bbc7  $04,$13ee
13e6: 58 ff 08  eor   $08,#$ff
13e9: 58 ff 09  eor   $09,#$ff
13ec: 3a 08     incw  $08
13ee: 6f        ret

13ef: e8 00     mov   a,#$00
13f1: c4 05     mov   $05,a
13f3: c4 04     mov   $04,a
13f5: f5 76 02  mov   a,$0276+x
13f8: f0 3f     beq   $1439
13fa: 60        clrc
13fb: 94 60     adc   a,$60+x
13fd: d4 60     mov   $60+x,a
13ff: 90 38     bcc   $1439
1401: f5 76 03  mov   a,$0376+x
1404: 5c        lsr   a
1405: f5 75 03  mov   a,$0375+x
1408: 5d        mov   x,a
1409: 90 09     bcc   $1414
140b: f5 6a 18  mov   a,$186a+x
140e: fd        mov   y,a
140f: f5 69 18  mov   a,$1869+x
1412: 2f 07     bra   $141b
1414: f5 6a 17  mov   a,$176a+x
1417: fd        mov   y,a
1418: f5 69 17  mov   a,$1769+x
141b: f8 22     mov   x,$22
141d: 35 85 03  and   a,$0385+x
1420: c4 04     mov   $04,a
1422: dd        mov   a,y
1423: 35 86 03  and   a,$0386+x
1426: c4 05     mov   $05,a
1428: f5 75 03  mov   a,$0375+x
142b: 60        clrc
142c: 88 01     adc   a,#$01
142e: d5 75 03  mov   $0375+x,a
1431: f5 76 03  mov   a,$0376+x
1434: 88 00     adc   a,#$00
1436: d5 76 03  mov   $0376+x,a
1439: 6f        ret

143a: 2d        push  a
143b: 6d        push  y
143c: f5 85 02  mov   a,$0285+x
143f: c4 05     mov   $05,a
1441: f5 86 02  mov   a,$0286+x
1444: c4 04     mov   $04,a
1446: 30 06     bmi   $144e
1448: ee        pop   y
1449: ae        pop   a
144a: 7a 04     addw  ya,$04
144c: 2f 0c     bra   $145a
144e: ae        pop   a
144f: 60        clrc
1450: 84 05     adc   a,$05
1452: fd        mov   y,a
1453: ae        pop   a
1454: 60        clrc
1455: 84 04     adc   a,$04
1457: b0 01     bcs   $145a
1459: dc        dec   y
145a: da 04     movw  $04,ya
145c: dd        mov   a,y
145d: 28 7f     and   a,#$7f
145f: 1c        asl   a
1460: fd        mov   y,a
1461: f6 82 15  mov   a,$1582+y
1464: c4 07     mov   $07,a
1466: f6 81 15  mov   a,$1581+y
1469: c4 06     mov   $06,a
146b: f6 84 15  mov   a,$1584+y
146e: 2d        push  a
146f: f6 83 15  mov   a,$1583+y
1472: ee        pop   y
1473: 9a 06     subw  ya,$06
1475: b0 04     bcs   $147b
1477: 8d 00     mov   y,#$00
1479: e8 c9     mov   a,#$c9
147b: 6d        push  y
147c: eb 04     mov   y,$04
147e: cf        mul   ya
147f: 8f 00 09  mov   $09,#$00
1482: cb 08     mov   $08,y
1484: ae        pop   a
1485: eb 04     mov   y,$04
1487: cf        mul   ya
1488: 7a 08     addw  ya,$08
148a: 7a 06     addw  ya,$06
148c: 6d        push  y
148d: 2d        push  a
148e: e8 02     mov   a,#$02
1490: 04 24     or    a,$24
1492: fd        mov   y,a
1493: ae        pop   a
1494: cc f2 00  mov   $00f2,y
1497: c5 f3 00  mov   $00f3,a
149a: fc        inc   y
149b: ae        pop   a
149c: 28 3f     and   a,#$3f
149e: cc f2 00  mov   $00f2,y
14a1: c5 f3 00  mov   $00f3,a
14a4: 6f        ret

14a5: 8f 00 07  mov   $07,#$00
14a8: f5 b6 03  mov   a,$03b6+x
14ab: c4 06     mov   $06,a
14ad: f5 30 01  mov   a,$0130+x
14b0: c4 08     mov   $08,a
14b2: f5 31 01  mov   a,$0131+x
14b5: c4 09     mov   $09,a
14b7: f5 40 01  mov   a,$0140+x
14ba: c4 0a     mov   $0a,a
14bc: f5 41 01  mov   a,$0141+x
14bf: c4 0b     mov   $0b,a
14c1: f5 16 02  mov   a,$0216+x
14c4: 8d 00     mov   y,#$00
14c6: 9a 06     subw  ya,$06
14c8: 7a 08     addw  ya,$08
14ca: 7a 0a     addw  ya,$0a
14cc: ad 80     cmp   y,#$80
14ce: b0 24     bcs   $14f4
14d0: ad 00     cmp   y,#$00
14d2: d0 04     bne   $14d8
14d4: 08 00     or    a,#$00
14d6: 10 02     bpl   $14da
14d8: e8 7f     mov   a,#$7f
14da: fd        mov   y,a
14db: f5 e6 02  mov   a,$02e6+x
14de: cf        mul   ya
14df: dd        mov   a,y
14e0: 78 00 1e  cmp   $1e,#$00
14e3: f0 07     beq   $14ec
14e5: 03 21 04  bbs0  $21,$14ec
14e8: eb 1e     mov   y,$1e
14ea: cf        mul   ya
14eb: dd        mov   a,y
14ec: 80        setc
14ed: b5 95 02  sbc   a,$0295+x
14f0: 90 02     bcc   $14f4
14f2: b0 02     bcs   $14f6
14f4: e8 00     mov   a,#$00
14f6: fd        mov   y,a
14f7: f6 e9 16  mov   a,$16e9+y
14fa: c4 05     mov   $05,a
14fc: e8 0a     mov   a,#$0a
14fe: 03 20 09  bbs0  $20,$150a
1501: f5 06 03  mov   a,$0306+x
1504: 68 15     cmp   a,#$15
1506: 90 02     bcc   $150a
1508: e8 14     mov   a,#$14
150a: c4 04     mov   $04,a
150c: e4 24     mov   a,$24
150e: 08 00     or    a,#$00
1510: c5 f2 00  mov   $00f2,a
1513: eb 04     mov   y,$04
1515: f6 43 15  mov   a,$1543+y
1518: eb 05     mov   y,$05
151a: cf        mul   ya
151b: dd        mov   a,y
151c: c5 f3 00  mov   $00f3,a
151f: e4 24     mov   a,$24
1521: 08 01     or    a,#$01
1523: c5 f2 00  mov   $00f2,a
1526: eb 04     mov   y,$04
1528: f6 58 15  mov   a,$1558+y
152b: eb 05     mov   y,$05
152d: cf        mul   ya
152e: dd        mov   a,y
152f: c5 f3 00  mov   $00f3,a
1532: 6f        ret



1533: 00        nop
1534: 00        nop
1535: 10 00     bpl   $1537
1537: 20        clrp
1538: 00        nop
1539: 30 00     bmi   $153b
153b: 40        setp
153c: 00        nop
153d: 50 00     bvc   $153f
153f: 60        clrc
1540: 00        nop
1541: 70 00     bvs   $1543

1543: 00        nop
1544: 0a 18 28  or1   c,$0503,0
1547: 3c        rol   a
1548: 50 64     bvc   $15ae
154a: 78 8c a0  cmp   $a0,#$8c
154d: b2 c4     clr5  $c4
154f: d2 de     clr6  $de
1551: e8 f0     mov   a,#$f0
1553: f6 fa fc  mov   a,$fcfa+y
1556: fc        inc   y
1557: fe

1558: fe
1559: fc        inc   y
155a: fc        inc   y
155b: fa f6 f0  mov   ($f0),($f6)
155e: e8 de     mov   a,#$de
1560: d2 c4     clr6  $c4
1562: b2 a0     clr5  $a0
1564: 8c 78 64  dec   $6478
1567: 50 3c     bvc   $15a5
1569: 28 18     and   a,#$18
156b: 0a 00 2c  or1   c,$0580,0
156e: 3c        rol   a
156f: 0d        push  psw
1570: 4d        push  x
1571: 6c 4c 5c  ror   $5c4c
1574: 3d        inc   x
1575: 2d        push  a
1576: 5c        lsr   a
1577: 10 11     bpl   $158a
1579: 15 18 14  or    a,$1418+x
157c: 12 0e     clr0  $0e
157e: 17 16     or    a,($16)+y
1580: 13 42 00  bbc0  $42,$1583
1583: 46        eor   a,(x)
1584: 00        nop
1585: 4b 00     lsr   $00
1587: 4f 00     pcall $00
1589: 54 00     eor   a,$00+x
158b: 59        eor   (x),(y)
158c: 00        nop
158d: 5e 00 64  cmp   y,$6400
1590: 00        nop
1591: 6a 00 70  and1  c,!($0e00,0)
1594: 00        nop
1595: 77 00     cmp   a,($00)+y
1597: 7e 00     cmp   y,$00
1599: 85 00 8d  adc   a,$8d00
159c: 00        nop
159d: 96 00 9f  adc   a,$9f00+y
15a0: 00        nop
15a1: a8 00     sbc   a,#$00
15a3: b2 00     clr5  $00
15a5: bd        mov   sp,x
15a6: 00        nop
15a7: c8 00     cmp   x,#$00
15a9: d4 00     mov   $00+x,a
15ab: e1        tcall 14
15ac: 00        nop
15ad: ee        pop   y
15ae: 00        nop
15af: fc        inc   y
15b0: 00        nop
15b1: 0b 01     asl   $01
15b3: 1b 01     asl   $01+x
15b5: 2c 01 3e  rol   $3e01
15b8: 01        tcall 0
15b9: 51        tcall 5
15ba: 01        tcall 0
15bb: 65 01 7a  cmp   a,$7a01
15be: 01        tcall 0
15bf: 91        tcall 9
15c0: 01        tcall 0
15c1: a9 01 c2  sbc   ($c2),($01)
15c4: 01        tcall 0
15c5: dd        mov   a,y
15c6: 01        tcall 0
15c7: f9 01     mov   x,$01+y
15c9: 17 02     or    a,($02)+y
15cb: 37 02     and   a,($02)+y
15cd: 59        eor   (x),(y)
15ce: 02 7d     set0  $7d
15d0: 02 a3     set0  $a3
15d2: 02 cb     set0  $cb
15d4: 02 f5     set0  $f5
15d6: 02 22     set0  $22
15d8: 03 52 03  bbs0  $52,$15de
15db: 85 03 ba  adc   a,$ba03
15de: 03 f3 03  bbs0  $f3,$15e4
15e1: 2f 04     bra   $15e7
15e3: 6f        ret

15e4: 04 b2     or    a,$b2
15e6: 04 fa     or    a,$fa
15e8: 04 46     or    a,$46
15ea: 05 96 05  or    a,$0596
15ed: eb 05     mov   y,$05
15ef: 45 06 a5  eor   a,$a506
15f2: 06        or    a,(x)
15f3: 0a 07 75  or1   c,$0ea0,7
15f6: 07 e6     or    a,($e6+x)
15f8: 07 5f     or    a,($5f+x)
15fa: 08 de     or    a,#$de
15fc: 08 65     or    a,#$65
15fe: 09 f4 09  or    ($09),($f4)
1601: 8c 0a 2c  dec   $2c0a
1604: 0b d6     asl   $d6
1606: 0b 8b     asl   $8b
1608: 0c 4a 0d  asl   $0d4a
160b: 14 0e     or    a,$0e+x
160d: ea 0e cd  not1  $19a1,6
1610: 0f        brk
1611: be        das   a
1612: 10 bd     bpl   $15d1
1614: 11        tcall 1
1615: cb 12     mov   $12,y
1617: e9 13 18  mov   x,$1813
161a: 15 59 16  or    a,$1659+x
161d: ad 17     cmp   y,#$17
161f: 16 19 94  or    a,$9419+y
1622: 1a 28     decw  $28
1624: 1c        asl   a
1625: d5 1d 9b  mov   $9b1d+x,a
1628: 1f 7c 21  jmp   ($217c+x)
162b: 7a 23     addw  ya,$23
162d: 96 25 d2  adc   a,$d225+y
1630: 27 30     and   a,($30+x)
1632: 2a b2 2c  or1   c,!($0596,2)
1635: 5a 2f     cmpw  ya,$2f
1637: 2c 32 28  rol   $2832
163a: 35 50 38  and   a,$3850+x
163d: ac 3b 36  inc   $363b
1640: 3f 0b 00  call  $000b
1643: 0c 00 0d  asl   $0d00
1646: 00        nop
1647: 0e 00 0e  tset1 $0e00
164a: 00        nop
164b: 0f        brk
164c: 00        nop
164d: 10 00     bpl   $164f
164f: 10 00     bpl   $1651
1651: 12 00     clr0  $00
1653: 13 00 14  bbc0  $00,$166a
1656: 00        nop
1657: 15 00 16  or    a,$1600+x
165a: 00        nop
165b: 18 00 19  or    $19,#$00
165e: 00        nop
165f: 1b 00     asl   $00+x
1661: 1c        asl   a
1662: 00        nop
1663: 1e 00 20  cmp   x,$2000
1666: 00        nop
1667: 21        tcall 2
1668: 00        nop
1669: 23 00 26  bbs1  $00,$1692
166c: 00        nop
166d: 28 00     and   a,#$00
166f: 2a 00 2d  or1   c,!($05a0,0)
1672: 00        nop
1673: 2f 00     bra   $1675
1675: 32 00     clr1  $00
1677: 35 00 38  and   a,$3800+x
167a: 00        nop
167b: 3c        rol   a
167c: 00        nop
167d: 3f 00 42  call  $4200
1680: 00        nop

; echo FIR preset
1681: db $7f,$00,$00,$00,$00,$00,$00,$00 ; 00
1689: db $58,$bf,$db,$f0,$fe,$07,$0c,$0c ; 01
1691: db $0c,$21,$2b,$2b,$13,$fe,$f3,$f9 ; 02
1699: db $34,$33,$00,$d9,$e5,$01,$fc,$eb ; 03

16a1: db $0f,$1f,$2f,$3f,$4f,$5f,$6f,$7f

16a9: 00        nop
16aa: 01        tcall 0
16ab: 02 03     set0  $03
16ad: 04 05     or    a,$05
16af: 06        or    a,(x)
16b0: 07 08     or    a,($08+x)
16b2: 09 0a 0b  or    ($0b),($0a)
16b5: 0c 0d 0e  asl   $0e0d
16b8: 0f        brk
16b9: 10 11     bpl   $16cc
16bb: 12 13     clr0  $13
16bd: 14 15     or    a,$15+x
16bf: 16 17 18  or    a,$1817+y
16c2: 19        or    (x),(y)
16c3: 1a 1b     decw  $1b
16c5: 1c        asl   a
16c6: 1d        dec   x
16c7: 1e 1f 20  cmp   x,$201f
16ca: 1f 1e 1d  jmp   ($1d1e+x)
16cd: 1c        asl   a
16ce: 1b 1a     asl   $1a+x
16d0: 19        or    (x),(y)
16d1: 18 17 16  or    $16,#$17
16d4: 15 14 13  or    a,$1314+x
16d7: 12 11     clr0  $11
16d9: 10 0f     bpl   $16ea
16db: 0e 0d 0c  tset1 $0c0d
16de: 0b 0a     asl   $0a
16e0: 09 08 07  or    ($07),($08)
16e3: 06        or    a,(x)
16e4: 05 04 03  or    a,$0304
16e7: 02 01     set0  $01
16e9: 00        nop
16ea: 01        tcall 0
16eb: 01        tcall 0
16ec: 01        tcall 0
16ed: 01        tcall 0
16ee: 01        tcall 0
16ef: 01        tcall 0
16f0: 01        tcall 0
16f1: 01        tcall 0
16f2: 01        tcall 0
16f3: 01        tcall 0
16f4: 01        tcall 0
16f5: 01        tcall 0
16f6: 01        tcall 0
16f7: 01        tcall 0
16f8: 01        tcall 0
16f9: 01        tcall 0
16fa: 01        tcall 0
16fb: 01        tcall 0
16fc: 01        tcall 0
16fd: 01        tcall 0
16fe: 01        tcall 0
16ff: 01        tcall 0
1700: 01        tcall 0
1701: 01        tcall 0
1702: 01        tcall 0
1703: 01        tcall 0
1704: 01        tcall 0
1705: 01        tcall 0
1706: 01        tcall 0
1707: 02 02     set0  $02
1709: 02 02     set0  $02
170b: 02 02     set0  $02
170d: 02 02     set0  $02
170f: 02 02     set0  $02
1711: 03 03 03  bbs0  $03,$1717
1714: 03 03 03  bbs0  $03,$171a
1717: 04 04     or    a,$04
1719: 04 04     or    a,$04
171b: 04 04     or    a,$04
171d: 05 05 05  or    a,$0505
1720: 05 06 06  or    a,$0606
1723: 07 07     or    a,($07+x)
1725: 07 07     or    a,($07+x)
1727: 08 08     or    a,#$08
1729: 09 09 0a  or    ($0a),($09)
172c: 0a 0a 0a  or1   c,$0141,2
172f: 0b 0b     asl   $0b
1731: 0c 0c 0d  asl   $0d0c
1734: 0d        push  psw
1735: 0e 0f 10  tset1 $100f
1738: 10 11     bpl   $174b
173a: 12 13     clr0  $13
173c: 14 15     or    a,$15+x
173e: 15 16 17  or    a,$1716+x
1741: 18 19 1b  or    $1b,#$19
1744: 1c        asl   a
1745: 1d        dec   x
1746: 1e 20 22  cmp   x,$2220
1749: 23 24 26  bbs1  $24,$1772
174c: 28 2a     and   a,#$2a
174e: 2c 2d 2f  rol   $2f2d
1751: 31        tcall 3
1752: 33 35 38  bbc1  $35,$178d
1755: 3a 3d     incw  $3d
1757: 40        setp
1758: 43 46 49  bbs2  $46,$17a4
175b: 4c 4f 52  lsr   $524f
175e: 56 5a 5e  eor   a,$5e5a+y
1761: 62 66     set3  $66
1763: 6b 6f     ror   $6f
1765: 73 77 7b  bbc3  $77,$17e3
1768: 7f        reti
1769: 9f        xcn   a
176a: 3c        rol   a
176b: b2 52     clr5  $52
176d: af        mov   (x)+,a
176e: 45 c7 89  eor   a,$89c7
1771: 10 7f     bpl   $17f2
1773: e0        clrv
1774: 9d        mov   x,sp
1775: dc        dec   y
1776: 1f 61 16  jmp   ($1661+x)
1779: 39        and   (x),(y)
177a: c9 9c eb  mov   $eb9c,x
177d: 57 08     eor   a,($08)+y
177f: 66        cmp   a,(x)
1780: f8 5a     mov   x,$5a
1782: 24 bf     and   a,$bf
1784: 0e 3e 15  tset1 $153e
1787: 4b db     lsr   $db
1789: ab f5     inc   $f5
178b: 31        tcall 3
178c: 0c 43 02  asl   $0243
178f: 55 de 41  eor   a,$41de+x
1792: da bd     movw  $bd,ya
1794: ae        pop   a
1795: 19        or    (x),(y)
1796: b0 48     bcs   $17e0
1798: 57 ba     eor   a,($ba)+y
179a: a3 36 0b  bbs5  $36,$17a8
179d: f9 df     mov   x,$df+y
179f: 17 a8     or    a,($a8)+y
17a1: 04 0c     or    a,$0c
17a3: e0        clrv
17a4: 91        tcall 9
17a5: 18 5d dd  or    $dd,#$5d
17a8: d3 28 8a  bbc6  $28,$1735
17ab: f2 11     clr7  $11
17ad: 59        eor   (x),(y)
17ae: 6f        ret

17af: 06        or    a,(x)
17b0: 0a 34 2a  or1   c,$0546,4
17b3: 79        cmp   (x),(y)
17b4: ac 5e a7  inc   $a75e
17b7: 83 c6 39  bbs4  $c6,$17f3
17ba: c1        tcall 12
17bb: b4 3a     sbc   a,$3a+x
17bd: 3f fe 4f  call  $4ffe
17c0: ef        sleep
17c1: 1f 00 30  jmp   ($3000+x)
17c4: 99        adc   (x),(y)
17c5: 4c 28 83  lsr   $8328
17c8: ed        notc
17c9: 8a 2f 2c  eor1  c,$0585,7
17cc: 66        cmp   a,(x)
17cd: 3f d6 6c  call  $6cd6
17d0: b7 49     sbc   a,($49)+y
17d2: 22 bc     set1  $bc
17d4: 65 fa cf  cmp   a,$cffa
17d7: 02 b1     set0  $b1
17d9: 46        eor   a,(x)
17da: f0 9a     beq   $1776
17dc: d7 e2     mov   ($e2)+y,a
17de: 0f        brk
17df: 11        tcall 1
17e0: c5 74 f6  mov   $f674,a
17e3: 7a 2c     addw  ya,$2c
17e5: 8f fb 19  mov   $19,#$fb
17e8: 6a e5 75  and1  c,!($0ebc,5)
17eb: 51        tcall 5
17ec: 4a ff b8  and1  c,$171f,7
17ef: 7f        reti
17f0: 62 db     set3  $db
17f2: 4f 14     pcall $14
17f4: e8 c2     mov   a,#$c2
17f6: d6 62 b9  mov   $b962+y,a
17f9: c2 ad     set6  $ad
17fb: a6        sbc   a,(x)
17fc: d5 29 f7  mov   $f729+x,a
17ff: 14 2e     or    a,$2e+x
1801: 6c 18 8e  ror   $8e18
1804: a4 c1     sbc   a,$c1
1806: a0        ei
1807: 7e 32     cmp   y,$32
1809: a9 be e6  sbc   ($e6),($be)
180c: 15 b6 7d  or    a,$7db6+x
180f: f8 a0     mov   x,$a0
1811: e6        mov   a,(x)
1812: 12 7a     clr0  $7a
1814: 67 47     cmp   a,($47+x)
1816: d2 03     clr6  $03
1818: 58 fd d1  eor   $d1,#$fd
181b: 9e        div   ya,x
181c: 31        tcall 3
181d: 9b 50     dec   $50+x
181f: 54 a5     eor   a,$a5+x
1821: ec 72 70  mov   y,$7072
1824: d5 b6 9b  mov   $9bb6+x,a
1827: 82 7c     set4  $7c
1829: a1        tcall 10
182a: c5 72 b9  mov   $b972,a
182d: a8 33     sbc   a,#$33
182f: 8b eb     dec   $eb
1831: 59        eor   (x),(y)
1832: 9e        div   ya,x
1833: 29 87 8e  and   ($8e),($87)
1836: 36 ad 86  and   a,$86ad+y
1839: ea e4 d4  not1  $1a9c,4
183c: 37 92     and   a,($92)+y
183e: ae        pop   a
183f: 45 f0 6e  eor   a,$6ef0
1842: 24 d0     and   a,$d0
1844: f3 cb 95  bbc7  $cb,$17dc
1847: 71        tcall 7
1848: e8 c6     mov   a,#$c6
184a: 38 ed 71  and   $71,#$ed
184d: 44 22     eor   a,$22
184f: 2b 60     rol   $60
1851: c0        di
1852: 26        and   a,(x)
1853: f9 53     mov   x,$53+y
1855: 5a c4     cmpw  ya,$c4
1857: 2d        push  a
1858: 74 c8     cmp   a,$c8+x
185a: 50 8d     bvc   $17e9
185c: f7 ac     mov   a,($ac)+y
185e: 01        tcall 0
185f: 79        cmp   (x),(y)
1860: bd        mov   sp,x
1861: 52 10     clr2  $10
1863: 5e 67 b0  cmp   y,$b067
1866: 01        tcall 0
1867: a6        sbc   a,(x)
1868: ff        stop
1869: 5f ce 54  jmp   $54ce

186c: 64 5c     cmp   a,$5c
186e: e7 ba     mov   a,($ba+x)
1870: 32 96     clr1  $96
1872: fc        inc   y
1873: 00        nop
1874: e9 cc 77  mov   x,$77cc
1877: dc        dec   y
1878: 60        clrc
1879: b5 92 4d  sbc   a,$4d92+x
187c: 84 1c     adc   a,$1c
187e: 68 ee     cmp   a,#$ee
1880: 4e af ab  tclr1 $abaf
1883: 07 87     or    a,($87+x)
1885: 9f        xcn   a
1886: 93 bf 88  bbc4  $bf,$1811
1889: f1        tcall 15
188a: cd 80     mov   x,#$80
188c: d7 4a     mov   ($4a)+y,a
188e: e5 20 4e  mov   a,$4e20
1891: 63 44 da  bbs3  $44,$186e
1894: 78 8f 07  cmp   $07,#$8f
1897: f3 40 98  bbc7  $40,$1832
189a: 6b 37     ror   $37
189c: a2 f4     set5  $f4
189e: 21        tcall 2
189f: 1e 96 13  cmp   x,$1396
18a2: 7e 09     cmp   y,$09
18a4: ea 3d 27  not1  $04e7,5
18a7: 86        adc   a,(x)
18a8: 4d        push  x
18a9: be        das   a
18aa: 8b cd     dec   $cd
18ac: d0 fb     bne   $18a9
18ae: a3 1b 97  bbs5  $1b,$1848
18b1: 2a bc 1a  or1   c,!($0357,4)
18b4: 8c 73 4c  dec   $4c73
18b7: 46        eor   a,(x)
18b8: 6f        ret

18b9: de 26 8c  cbne  $26+x,$1848
18bc: b8 33 aa  sbc   $aa,#$33
18bf: 0e bb 21  tset1 $21bb
18c2: d8 89     mov   $89,x
18c4: 3b 97     rol   $97+x
18c6: 1d        dec   x
18c7: bb 23     inc   $23+x
18c9: 48 90     eor   a,#$90
18cb: 69 fa c9  cmp   ($c9),($fa)
18ce: 76 ce e7  cmp   a,$e7ce+y
18d1: 1b 7b     asl   $7b+x
18d3: 1a 2b     decw  $2b
18d5: 78 c0 03  cmp   $03,#$c0
18d8: 08 6b     or    a,#$6b
18da: cf        mul   ya
18db: cc b3 6d  mov   $6db3,y
18de: a9 b2 3a  sbc   ($3a),($b2)
18e1: 16 a1 56  or    a,$56a1+y
18e4: 95 0b 17  adc   a,$170b+x
18e7: f1        tcall 15
18e8: 12 99     clr0  $99
18ea: 13 d8 81  bbc0  $d8,$186e
18ed: 56 38 f5  eor   a,$f538+y
18f0: 3e d1     cmp   x,$d1
18f2: b1        tcall 11
18f3: e3 e1 b3  bbs7  $e1,$18a9
18f6: 2e 84 ef  cbne  $84,$18e8
18f9: 5b 04     lsr   $04+x
18fb: 64 5d     cmp   a,$5d
18fd: b5 0a 5b  sbc   a,$5b0a+x
1900: 1d        dec   x
1901: 94 06     adc   a,$06+x
1903: d9 e4     mov   $e4+y,x
1905: 0d        push  psw
1906: fd        mov   y,a
1907: 6d        push  y
1908: a2 4b     set5  $4b
190a: f6 65 a5  mov   a,$a565+y
190d: cb aa     mov   $aa,y
190f: 5f e3 70  jmp   $70e3

1912: 1e f2 93  cmp   x,$93f2
1915: 82 63     set4  $63
1917: 27 ee     and   a,($ee+x)
1919: e2 1c     set7  $1c
191b: 98 23 ca  adc   $ca,#$23
191e: 0d        push  psw
191f: ec d9 05  mov   y,$05d9
1922: f4 3b     mov   a,$3b+x
1924: 53 9d 73  bbc2  $9d,$199a
1927: 3c        rol   a
1928: c3 d4 fe  bbs6  $d4,$1929
192b: e1        tcall 14
192c: dd        mov   a,y
192d: 61        tcall 6
192e: 69 90 2d  cmp   ($2d),($90)
1931: 2f 8d     bra   $18c0
1933: 51        tcall 5
1934: c4 a4     mov   $a4,a
1936: 7c        ror   a
1937: 43 88 34  bbs2  $88,$196e
193a: 5c        lsr   a
193b: 85 b4 7d  adc   a,$7db4
193e: 25 30 58  and   a,$5830
1941: 85 68 35  adc   a,$3568
1944: c7 47     mov   ($47+x),a
1946: 20        clrp
1947: 05 35 09  or    a,$0935
194a: 25 80 55  and   a,$5580
194d: ca 6a 91  mov1  $122d,2,c
1950: fc        inc   y
1951: c8 d3     cmp   x,#$d3
1953: 0f        brk
1954: a7 c3     sbc   a,($c3+x)
1956: 9a 6e     subw  ya,$6e
1958: 94 9c     adc   a,$9c+x
195a: 42 df     set2  $df
195c: 77 42     cmp   a,($42)+y
195e: 40        setp
195f: d2 76     clr6  $76
1961: 3d        inc   x
1962: 41        tcall 4
1963: 81        tcall 8
1964: 75 b7 49  cmp   a,$49b7+x
1967: e9 7b e5  mov   x,$e57b
196a: f4 00     mov   a,$00+x
196c: 65 f4 00  cmp   a,$00f4
196f: d0 f8     bne   $1969
1971: 68 00     cmp   a,#$00
1973: d0 01     bne   $1976
1975: 6f        ret

1976: c5 f4 00  mov   $00f4,a
1979: 8d 13     mov   y,#$13
197b: cc f1 00  mov   $00f1,y
197e: c4 0c     mov   $0c,a
1980: 68 ff     cmp   a,#$ff
1982: d0 06     bne   $198a
1984: 3f 57 1b  call  $1b57
1987: e8 00     mov   a,#$00
1989: 6f        ret

198a: 68 99     cmp   a,#$99
198c: b0 f9     bcs   $1987
198e: 68 91     cmp   a,#$91
1990: b0 17     bcs   $19a9
1992: 68 17     cmp   a,#$17
1994: f0 f1     beq   $1987
1996: 68 4b     cmp   a,#$4b
1998: f0 ed     beq   $1987
199a: 68 8c     cmp   a,#$8c
199c: f0 e9     beq   $1987
199e: 68 8d     cmp   a,#$8d
19a0: f0 e5     beq   $1987
19a2: 68 8f     cmp   a,#$8f
19a4: f0 e1     beq   $1987
19a6: 5f 05 1a  jmp   $1a05

19a9: 80        setc
19aa: a8 92     sbc   a,#$92
19ac: 1c        asl   a
19ad: 5d        mov   x,a
19ae: e8 00     mov   a,#$00
19b0: 1f b3 19  jmp   ($19b3+x)
19b3: c1        tcall 12
19b4: 19        or    (x),(y)
19b5: c4 19     mov   $19,a
19b7: c7 19     mov   ($19+x),a
19b9: c7 19     mov   ($19+x),a
19bb: c7 19     mov   ($19+x),a
19bd: db 19     mov   $19+x,y
19bf: e3 19 02  bbs7  $19,$19c4
19c2: 20        clrp
19c3: 6f        ret

19c4: 12 20     clr0  $20
19c6: 6f        ret

19c7: e4 0c     mov   a,$0c
19c9: 80        setc
19ca: a8 94     sbc   a,#$94
19cc: 5d        mov   x,a
19cd: f5 95 06  mov   a,$0695+x
19d0: c5 04 02  mov   $0204,a
19d3: 8f ff 1e  mov   $1e,#$ff
19d6: e8 00     mov   a,#$00
19d8: c4 1f     mov   $1f,a
19da: 6f        ret

19db: e8 00     mov   a,#$00
19dd: 8d 0c     mov   y,#$0c
19df: cd 3f     mov   x,#$3f
19e1: 2f 06     bra   $19e9
19e3: e8 00     mov   a,#$00
19e5: 8d 10     mov   y,#$10
19e7: cd ff     mov   x,#$ff
19e9: 32 20     clr1  $20
19eb: d6 ce 00  mov   $00ce+y,a
19ee: d6 23 02  mov   $0223+y,a
19f1: dc        dec   y
19f2: fe f7     dbnz  y,$19eb
19f4: c4 1e     mov   $1e,a
19f6: c4 27     mov   $27,a
19f8: c4 15     mov   $15,a
19fa: c4 18     mov   $18,a
19fc: c4 10     mov   $10,a
19fe: c4 11     mov   $11,a
1a00: 3f a6 11  call  $11a6
1a03: 7d        mov   a,x
1a04: 6f        ret

1a05: e4 0c     mov   a,$0c
1a07: 8f c5 04  mov   $04,#$c5
1a0a: 8f 03 05  mov   $05,#$03
1a0d: 9c        dec   a
1a0e: 8d 05     mov   y,#$05
1a10: cf        mul   ya
1a11: 7a 04     addw  ya,$04
1a13: da 04     movw  $04,ya
1a15: 8d 00     mov   y,#$00
1a17: cd 00     mov   x,#$00
1a19: f7 04     mov   a,($04)+y
1a1b: c4 1c     mov   $1c,a
1a1d: fc        inc   y
1a1e: f7 04     mov   a,($04)+y
1a20: c4 06     mov   $06,a
1a22: e4 0c     mov   a,$0c
1a24: 68 4b     cmp   a,#$4b
1a26: b0 2c     bcs   $1a54
1a28: cd 0c     mov   x,#$0c
1a2a: 68 17     cmp   a,#$17
1a2c: b0 1c     bcs   $1a4a
1a2e: 74 d2     cmp   a,$d2+x
1a30: d0 06     bne   $1a38
1a32: 3d        inc   x
1a33: 3d        inc   x
1a34: 0b 1c     asl   $1c
1a36: 2f 1c     bra   $1a54
1a38: 74 d0     cmp   a,$d0+x
1a3a: f0 18     beq   $1a54
1a3c: f5 25 02  mov   a,$0225+x
1a3f: 75 27 02  cmp   a,$0227+x
1a42: f0 06     beq   $1a4a
1a44: 90 04     bcc   $1a4a
1a46: 3d        inc   x
1a47: 3d        inc   x
1a48: 0b 1c     asl   $1c
1a4a: e4 06     mov   a,$06
1a4c: 75 25 02  cmp   a,$0225+x
1a4f: b0 03     bcs   $1a54
1a51: e8 00     mov   a,#$00
1a53: 6f        ret

1a54: fc        inc   y
1a55: f7 04     mov   a,($04)+y
1a57: 28 0f     and   a,#$0f
1a59: c4 07     mov   $07,a
1a5b: f7 04     mov   a,($04)+y
1a5d: 9f        xcn   a
1a5e: 28 0f     and   a,#$0f
1a60: f0 04     beq   $1a66
1a62: 9c        dec   a
1a63: c5 26 00  mov   $0026,a
1a66: fc        inc   y
1a67: f7 04     mov   a,($04)+y
1a69: c4 0a     mov   $0a,a
1a6b: fc        inc   y
1a6c: f7 04     mov   a,($04)+y
1a6e: c4 0b     mov   $0b,a
1a70: 8f 00 16  mov   $16,#$00
1a73: 78 4b 0c  cmp   $0c,#$4b
1a76: 90 12     bcc   $1a8a
1a78: 8f 00 1e  mov   $1e,#$00
1a7b: e8 00     mov   a,#$00
1a7d: c4 15     mov   $15,a
1a7f: c4 18     mov   $18,a
1a81: c4 10     mov   $10,a
1a83: c4 11     mov   $11,a
1a85: a2 14     set5  $14
1a87: 3f a6 11  call  $11a6
1a8a: 8d 00     mov   y,#$00
1a8c: f7 0a     mov   a,($0a)+y
1a8e: d4 2f     mov   $2f+x,a
1a90: d5 70 01  mov   $0170+x,a
1a93: d5 80 01  mov   $0180+x,a
1a96: d5 c5 02  mov   $02c5+x,a
1a99: fc        inc   y
1a9a: f7 0a     mov   a,($0a)+y
1a9c: d4 30     mov   $30+x,a
1a9e: d5 71 01  mov   $0171+x,a
1aa1: d5 81 01  mov   $0181+x,a
1aa4: d5 c6 02  mov   $02c6+x,a
1aa7: e8 ff     mov   a,#$ff
1aa9: d5 05 02  mov   $0205+x,a
1aac: e8 7f     mov   a,#$7f
1aae: d5 e5 02  mov   $02e5+x,a
1ab1: d5 16 02  mov   $0216+x,a
1ab4: e8 0a     mov   a,#$0a
1ab6: d5 06 03  mov   $0306+x,a
1ab9: d5 26 02  mov   $0226+x,a
1abc: e8 00     mov   a,#$00
1abe: d5 e6 02  mov   $02e6+x,a
1ac1: d4 3f     mov   $3f+x,a
1ac3: d5 05 03  mov   $0305+x,a
1ac6: d5 a5 02  mov   $02a5+x,a
1ac9: d4 4f     mov   $4f+x,a
1acb: d4 5f     mov   $5f+x,a
1acd: d5 30 01  mov   $0130+x,a
1ad0: d5 31 01  mov   $0131+x,a
1ad3: d5 40 01  mov   $0140+x,a
1ad6: d5 41 01  mov   $0141+x,a
1ad9: d5 50 01  mov   $0150+x,a
1adc: d5 51 01  mov   $0151+x,a
1adf: d5 60 01  mov   $0160+x,a
1ae2: d5 61 01  mov   $0161+x,a
1ae5: d5 95 03  mov   $0395+x,a
1ae8: d5 96 03  mov   $0396+x,a
1aeb: d5 56 02  mov   $0256+x,a
1aee: d4 8f     mov   $8f+x,a
1af0: d4 7f     mov   $7f+x,a
1af2: d5 55 03  mov   $0355+x,a
1af5: d5 56 03  mov   $0356+x,a
1af8: d5 55 02  mov   $0255+x,a
1afb: d5 76 02  mov   $0276+x,a
1afe: d4 70     mov   $70+x,a
1b00: d4 6f     mov   $6f+x,a
1b02: d5 a5 03  mov   $03a5+x,a
1b05: d5 a6 03  mov   $03a6+x,a
1b08: d5 66 02  mov   $0266+x,a
1b0b: d5 75 02  mov   $0275+x,a
1b0e: d5 50 00  mov   $0050+x,a
1b11: d5 65 02  mov   $0265+x,a
1b14: d5 b6 02  mov   $02b6+x,a
1b17: d5 a6 02  mov   $02a6+x,a
1b1a: d5 95 02  mov   $0295+x,a
1b1d: bc        inc   a
1b1e: d5 b5 02  mov   $02b5+x,a
1b21: d4 bf     mov   $bf+x,a
1b23: e4 0c     mov   a,$0c
1b25: d4 d0     mov   $d0+x,a
1b27: 68 4b     cmp   a,#$4b
1b29: 90 0c     bcc   $1b37
1b2b: 68 8f     cmp   a,#$8f
1b2d: b0 08     bcs   $1b37
1b2f: 68 8c     cmp   a,#$8c
1b31: 90 0b     bcc   $1b3e
1b33: 68 8d     cmp   a,#$8d
1b35: b0 07     bcs   $1b3e
1b37: e8 01     mov   a,#$01
1b39: d5 a5 02  mov   $02a5+x,a
1b3c: 2f 08     bra   $1b46
1b3e: 78 90 0c  cmp   $0c,#$90
1b41: f0 03     beq   $1b46
1b43: fa 0c 27  mov   ($27),($0c)
1b46: e4 06     mov   a,$06
1b48: d5 25 02  mov   $0225+x,a
1b4b: 6e 07 03  dbnz  $07,$1b51
1b4e: e4 1c     mov   a,$1c
1b50: 6f        ret

1b51: 3d        inc   x
1b52: 3d        inc   x
1b53: fc        inc   y
1b54: 5f 8c 1a  jmp   $1a8c

1b57: cd cc     mov   x,#$cc
1b59: 1e f4 00  cmp   x,$00f4
1b5c: d0 f9     bne   $1b57
1b5e: 2f 20     bra   $1b80
1b60: ec f4 00  mov   y,$00f4
1b63: d0 fb     bne   $1b60
1b65: 5e f4 00  cmp   y,$00f4
1b68: d0 0f     bne   $1b79
1b6a: e5 f5 00  mov   a,$00f5
1b6d: cc f4 00  mov   $00f4,y
1b70: d7 04     mov   ($04)+y,a
1b72: fc        inc   y
1b73: d0 f0     bne   $1b65
1b75: ab 05     inc   $05
1b77: 2f ec     bra   $1b65
1b79: 10 ea     bpl   $1b65
1b7b: 5e f4 00  cmp   y,$00f4
1b7e: 10 e5     bpl   $1b65
1b80: e5 f6 00  mov   a,$00f6
1b83: ec f7 00  mov   y,$00f7
1b86: da 04     movw  $04,ya
1b88: e5 f4 00  mov   a,$00f4
1b8b: ec f5 00  mov   y,$00f5
1b8e: c5 f4 00  mov   $00f4,a
1b91: dd        mov   a,y
1b92: 5d        mov   x,a
1b93: d0 cb     bne   $1b60
1b95: e8 33     mov   a,#$33
1b97: c5 f1 00  mov   $00f1,a
1b9a: 6f        ret

1b9b: 9d        mov   x,sp
1b9c: 1b ea     asl   $ea+x
1b9e: fa ee c8  mov   ($c8),($ee)
1ba1: e2 02     set7  $02
1ba3: ec 00 f2  mov   y,$f200
1ba6: 0a fa 95  or1   c,$12bf,2
1ba9: e8 00     mov   a,#$00
1bab: f1        tcall 15
1bac: 00        nop
1bad: 04 02     or    a,$02
1baf: fc        inc   y
1bb0: 80        setc
1bb1: 00        nop
1bb2: 45 1e 4a  eor   a,$4a1e
1bb5: 7f        reti
1bb6: c5 51 50  mov   $5051,a
1bb9: ff        stop
1bba: bc        inc   a
1bbb: 1b ea     asl   $ea+x
1bbd: 78 ee ca  cmp   $ca,#$ee
1bc0: e2 02     set7  $02
1bc2: ec 05 fa  mov   y,$fa05
1bc5: 96 da 00  adc   a,$00da+y
1bc8: 4f 05     pcall $05
1bca: 7f        reti
1bcb: 7f        reti
1bcc: e6        mov   a,(x)
1bcd: 61        tcall 6
1bce: 4f 04     pcall $04
1bd0: 7f        reti
1bd1: 55 cf c6  eor   a,$c6cf+x
1bd4: cf        mul   ya
1bd5: b7 e7     sbc   a,($e7)+y
1bd7: 02 fb     set0  $fb
1bd9: 00        nop
1bda: ff        stop
1bdb: dd        mov   a,y
1bdc: 1b e2     asl   $e2+x
1bde: 19        or    (x),(y)
1bdf: ea 64 ee  not1  $1dcc,4
1be2: f0 fa     beq   $1bde
1be4: 96 dc 00  adc   a,$00dc+y
1be7: 2b 32     rol   $32
1be9: 7d        mov   a,x
1bea: 7f        reti
1beb: ff        stop
1bec: e2 11     set7  $11
1bee: ea 78 ee  not1  $1dcf,0
1bf1: fe e4     dbnz  y,$1bd7
1bf3: 00        nop
1bf4: 82 78     set4  $78
1bf6: e6        mov   a,(x)
1bf7: 61        tcall 6
1bf8: 18 05 7d  or    $7d,#$05
1bfb: 7f        reti
1bfc: e0        clrv
1bfd: 02 e7     set0  $e7
1bff: 1e 00 14  cmp   x,$1400
1c02: ff        stop
1c03: 05 1c e2  or    a,$e21c
1c06: 1b ea     asl   $ea+x
1c08: 64 ee     cmp   a,$ee
1c0a: ec 23 1e  mov   y,$1e23
1c0d: 7d        mov   a,x
1c0e: 7f        reti
1c0f: 23 0a bc  bbs1  $0a,$1bce
1c12: ff        stop
1c13: 15 1c e2  or    a,$e21c+x
1c16: 1b ea     asl   $ea+x
1c18: 64 ee     cmp   a,$ee
1c1a: f4 fa     mov   a,$fa+x
1c1c: 95 e6 00  adc   a,$00e6+x
1c1f: e4 05     mov   a,$05
1c21: a0        ei
1c22: 14 23     or    a,$23+x
1c24: 11        tcall 1
1c25: 7d        mov   a,x
1c26: 7f        reti
1c27: f3 00 11  bbc7  $00,$1c3b
1c2a: 20        clrp
1c2b: fc        inc   y
1c2c: d3 ff 23  bbc6  $ff,$1c52
1c2f: 12 c2     clr0  $c2
1c31: ff        stop
1c32: 34 1c     and   a,$1c+x
1c34: e2 1a     set7  $1a
1c36: ea 64 ee  not1  $1dcc,4
1c39: fe fa     dbnz  y,$1c35
1c3b: 96 e1 00  adc   a,$00e1+y
1c3e: 22 24     set1  $24
1c40: 7d        mov   a,x
1c41: 7f        reti
1c42: 22 14     set1  $14
1c44: b5 ff e2  sbc   a,$e2ff+x
1c47: 11        tcall 1
1c48: ea 78 ee  not1  $1dcf,0
1c4b: fe e4     dbnz  y,$1c31
1c4d: 00        nop
1c4e: 82 78     set4  $78
1c50: e6        mov   a,(x)
1c51: 61        tcall 6
1c52: 18 05 7d  or    $7d,#$05
1c55: 7f        reti
1c56: e0        clrv
1c57: 02 e7     set0  $e7
1c59: 1e 00 14  cmp   x,$1400
1c5c: ff        stop
1c5d: 5f 1c e2  jmp   $e21c

1c60: 1a ea     decw  $ea
1c62: 64 ee     cmp   a,$ee
1c64: fe fa     dbnz  y,$1c60
1c66: 96 e1 00  adc   a,$00e1+y
1c69: f2 a6     clr7  $a6
1c6b: 22 09     set1  $09
1c6d: 7d        mov   a,x
1c6e: 7f        reti
1c6f: f3 00 07  bbc7  $00,$1c79
1c72: 25 fc 6d  and   a,$6dfc
1c75: 00        nop
1c76: f3 00 05  bbc7  $00,$1c7e
1c79: 1d        dec   x
1c7a: fc        inc   y
1c7b: 67 fe     cmp   a,($fe+x)
1c7d: 22 0c     set1  $0c
1c7f: d5 f3 00  mov   $00f3+x,a
1c82: 07 25     or    a,($25+x)
1c84: fc        inc   y
1c85: 6d        push  y
1c86: 00        nop
1c87: f3 00 05  bbc7  $00,$1c8f
1c8a: 1d        dec   x
1c8b: fc        inc   y
1c8c: 67 fe     cmp   a,($fe+x)
1c8e: ff        stop
1c8f: e2 11     set7  $11
1c91: ea 78 ee  not1  $1dcf,0
1c94: fe e4     dbnz  y,$1c7a
1c96: 00        nop
1c97: 82 78     set4  $78
1c99: e6        mov   a,(x)
1c9a: 61        tcall 6
1c9b: 18 05 7d  or    $7d,#$05
1c9e: 7f        reti
1c9f: e0        clrv
1ca0: 02 e7     set0  $e7
1ca2: 1e 00 14  cmp   x,$1400
1ca5: ff        stop
1ca6: a8 1c     sbc   a,#$1c
1ca8: e2 1c     set7  $1c
1caa: ea 64 ee  not1  $1dcc,4
1cad: fe 26     dbnz  y,$1cd5
1caf: 1b 7d     asl   $7d+x
1cb1: 7f        reti
1cb2: fa 82 e1  mov   ($e1),($82)
1cb5: 00        nop
1cb6: 26        and   a,(x)
1cb7: 1e d5 ff  cmp   x,$ffd5
1cba: e2 11     set7  $11
1cbc: ea 78 ee  not1  $1dcf,0
1cbf: fe e4     dbnz  y,$1ca5
1cc1: 00        nop
1cc2: 82 78     set4  $78
1cc4: e6        mov   a,(x)
1cc5: 61        tcall 6
1cc6: 18 05 7d  or    $7d,#$05
1cc9: 7f        reti
1cca: e0        clrv
1ccb: 02 e7     set0  $e7
1ccd: 1e 00 14  cmp   x,$1400
1cd0: ff        stop
1cd1: d3 1c e2  bbc6  $1c,$1cb6
1cd4: 1c        asl   a
1cd5: ea 64 ee  not1  $1dcc,4
1cd8: fe fa     dbnz  y,$1cd4
1cda: 96 d8 00  adc   a,$00d8+y
1cdd: f2 c9     clr7  $c9
1cdf: 26        and   a,(x)
1ce0: 08 7d     or    a,#$7d
1ce2: 7f        reti
1ce3: f3 00 06  bbc7  $00,$1cec
1ce6: 29 fc 80  and   ($80),($fc)
1ce9: 00        nop
1cea: f3 00 04  bbc7  $00,$1cf1
1ced: 26        and   a,(x)
1cee: fc        inc   y
1cef: 40        setp
1cf0: ff        stop
1cf1: 26        and   a,(x)
1cf2: 14 e4     or    a,$e4+x
1cf4: f3 00 0e  bbc7  $00,$1d05
1cf7: 26        and   a,(x)
1cf8: fc        inc   y
1cf9: 00        nop
1cfa: 00        nop
1cfb: 26        and   a,(x)
1cfc: 0f        brk
1cfd: b9        sbc   (x),(y)
1cfe: f3 00 0e  bbc7  $00,$1d0f
1d01: 26        and   a,(x)
1d02: fc        inc   y
1d03: 00        nop
1d04: 00        nop
1d05: ff        stop
1d06: e2 11     set7  $11
1d08: ea 78 ee  not1  $1dcf,0
1d0b: fe e4     dbnz  y,$1cf1
1d0d: 00        nop
1d0e: 82 78     set4  $78
1d10: e6        mov   a,(x)
1d11: 61        tcall 6
1d12: 18 05 7d  or    $7d,#$05
1d15: 7f        reti
1d16: e0        clrv
1d17: 02 e7     set0  $e7
1d19: 1e 00 14  cmp   x,$1400
1d1c: ff        stop
1d1d: 1f 1d e2  jmp   ($e21d+x)
1d20: 1b ea     asl   $ea+x
1d22: 64 ee     cmp   a,$ee
1d24: fe fa     dbnz  y,$1d20
1d26: 8c df 00  dec   $00df
1d29: 23 06 7f  bbs1  $06,$1dab
1d2c: 7f        reti
1d2d: 21        tcall 2
1d2e: 14 7d     or    a,$7d+x
1d30: 7f        reti
1d31: 23 06 7f  bbs1  $06,$1db3
1d34: 3c        rol   a
1d35: 21        tcall 2
1d36: 0e 7d 3c  tset1 $3c7d
1d39: ff        stop
1d3a: e2 11     set7  $11
1d3c: ea 78 ee  not1  $1dcf,0
1d3f: fe e4     dbnz  y,$1d25
1d41: 00        nop
1d42: 82 78     set4  $78
1d44: e6        mov   a,(x)
1d45: 61        tcall 6
1d46: 18 05 7d  or    $7d,#$05
1d49: 7f        reti
1d4a: e0        clrv
1d4b: 02 e7     set0  $e7
1d4d: 1e 00 14  cmp   x,$1400
1d50: ff        stop
1d51: 53 1d e2  bbc2  $1d,$1d36
1d54: 1b ea     asl   $ea+x
1d56: 64 ee     cmp   a,$ee
1d58: fe f2     dbnz  y,$1d4c
1d5a: e7 fa     mov   a,($fa+x)
1d5c: 96 e1 00  adc   a,$00e1+y
1d5f: 21        tcall 2
1d60: 04 7f     or    a,$7f
1d62: 7f        reti
1d63: 21        tcall 2
1d64: 05 7d 7f  or    a,$7f7d
1d67: 23 04 7f  bbs1  $04,$1de9
1d6a: 50 21     bvc   $1d8d
1d6c: 06        or    a,(x)
1d6d: 7d        mov   a,x
1d6e: 50 ff     bvc   $1d6f
1d70: e2 11     set7  $11
1d72: ea 78 ee  not1  $1dcf,0
1d75: fe e4     dbnz  y,$1d5b
1d77: 00        nop
1d78: 82 78     set4  $78
1d7a: e6        mov   a,(x)
1d7b: 61        tcall 6
1d7c: 18 05 7d  or    $7d,#$05
1d7f: 7f        reti
1d80: e0        clrv
1d81: 02 e7     set0  $e7
1d83: 1e 00 14  cmp   x,$1400
1d86: ff        stop
1d87: 89 1d e2  adc   ($e2),($1d)
1d8a: 1b ea     asl   $ea+x
1d8c: 64 ee     cmp   a,$ee
1d8e: fe 1c     dbnz  y,$1dac
1d90: 1c        asl   a
1d91: 7d        mov   a,x
1d92: 7f        reti
1d93: f3 00 1c  bbc7  $00,$1db2
1d96: 20        clrp
1d97: fc        inc   y
1d98: 24 00     and   a,$00
1d9a: 1c        asl   a
1d9b: 14 bf     or    a,$bf+x
1d9d: f3 00 14  bbc7  $00,$1db4
1da0: 20        clrp
1da1: fc        inc   y
1da2: 33 00 ff  bbc1  $00,$1da4
1da5: e2 11     set7  $11
1da7: ea 78 ee  not1  $1dcf,0
1daa: fe e4     dbnz  y,$1d90
1dac: 00        nop
1dad: 82 78     set4  $78
1daf: e6        mov   a,(x)
1db0: 61        tcall 6
1db1: 18 05 7d  or    $7d,#$05
1db4: 7f        reti
1db5: e0        clrv
1db6: 02 e7     set0  $e7
1db8: 1e 00 14  cmp   x,$1400
1dbb: ff        stop
1dbc: be        das   a
1dbd: 1d        dec   x
1dbe: e2 1b     set7  $1b
1dc0: ea 64 ee  not1  $1dcc,4
1dc3: fe f2     dbnz  y,$1db7
1dc5: 1f fa 78  jmp   ($78fa+x)
1dc8: e7 00     mov   a,($00+x)
1dca: 1e 0a 7d  cmp   x,$7d0a
1dcd: 7f        reti
1dce: 1c        asl   a
1dcf: 0d        push  psw
1dd0: d0 ff     bne   $1dd1
1dd2: e2 11     set7  $11
1dd4: ea 78 ee  not1  $1dcf,0
1dd7: fe e4     dbnz  y,$1dbd
1dd9: 00        nop
1dda: 82 78     set4  $78
1ddc: e6        mov   a,(x)
1ddd: 61        tcall 6
1dde: 18 05 7d  or    $7d,#$05
1de1: 7f        reti
1de2: e0        clrv
1de3: 02 e7     set0  $e7
1de5: 1e 00 14  cmp   x,$1400
1de8: ff        stop
1de9: eb 1d     mov   y,$1d
1deb: e2 1c     set7  $1c
1ded: ea 64 ee  not1  $1dcc,4
1df0: fe e4     dbnz  y,$1dd6
1df2: 0a 5a 1e  or1   c,$03cb,2
1df5: 20        clrp
1df6: 07 7d     or    a,($7d+x)
1df8: 7f        reti
1df9: 20        clrp
1dfa: 1e ff f3  cmp   x,$f3ff
1dfd: 00        nop
1dfe: 23 1a fc  bbs1  $1a,$1dfd
1e01: d5 ff 21  mov   $21ff+x,a
1e04: 19        or    (x),(y)
1e05: c2 f3     set6  $f3
1e07: 00        nop
1e08: 23 1a fc  bbs1  $1a,$1e07
1e0b: cd ff     mov   x,#$ff
1e0d: ff        stop
1e0e: e2 11     set7  $11
1e10: ea 78 ee  not1  $1dcf,0
1e13: fe e4     dbnz  y,$1df9
1e15: 00        nop
1e16: 82 78     set4  $78
1e18: e6        mov   a,(x)
1e19: 61        tcall 6
1e1a: 18 05 7d  or    $7d,#$05
1e1d: 7f        reti
1e1e: e0        clrv
1e1f: 02 e7     set0  $e7
1e21: 1e 00 14  cmp   x,$1400
1e24: ff        stop
1e25: 27 1e     and   a,($1e+x)
1e27: e2 1c     set7  $1c
1e29: ea 64 ee  not1  $1dcc,4
1e2c: fe f2     dbnz  y,$1e20
1e2e: aa 22 09  mov1  c,$0124,2
1e31: 7d        mov   a,x
1e32: 7f        reti
1e33: a2 d5     set5  $d5
1e35: 22 0d     set1  $0d
1e37: bc        inc   a
1e38: ff        stop
1e39: e2 11     set7  $11
1e3b: ea 78 ee  not1  $1dcf,0
1e3e: fe e4     dbnz  y,$1e24
1e40: 00        nop
1e41: 82 78     set4  $78
1e43: e6        mov   a,(x)
1e44: 61        tcall 6
1e45: 18 05 7d  or    $7d,#$05
1e48: 7f        reti
1e49: e0        clrv
1e4a: 02 e7     set0  $e7
1e4c: 1e 00 14  cmp   x,$1400
1e4f: ff        stop
1e50: 52 1e     clr2  $1e
1e52: e2 1c     set7  $1c
1e54: ea 64 ee  not1  $1dcc,4
1e57: fe 23     dbnz  y,$1e7c
1e59: 1e 7d 7f  cmp   x,$7f7d
1e5c: a3 c6 ff  bbs5  $c6,$1e5e
1e5f: e2 11     set7  $11
1e61: ea 78 ee  not1  $1dcf,0
1e64: fe e4     dbnz  y,$1e4a
1e66: 00        nop
1e67: 82 78     set4  $78
1e69: e6        mov   a,(x)
1e6a: 61        tcall 6
1e6b: 18 05 7d  or    $7d,#$05
1e6e: 7f        reti
1e6f: e0        clrv
1e70: 02 e7     set0  $e7
1e72: 1e 00 14  cmp   x,$1400
1e75: ff        stop
1e76: 78 1e e2  cmp   $e2,#$1e
1e79: 1c        asl   a
1e7a: ea 64 ee  not1  $1dcc,4
1e7d: fe f2     dbnz  y,$1e71
1e7f: aa f1 00  mov1  c,$001e,1
1e82: 08 02     or    a,#$02
1e84: fc        inc   y
1e85: 40        setp
1e86: 00        nop
1e87: 23 0a 7d  bbs1  $0a,$1f07
1e8a: 7f        reti
1e8b: f2 a4     clr7  $a4
1e8d: 23 0c ce  bbs1  $0c,$1e5e
1e90: 23 08 bc  bbs1  $08,$1e4f
1e93: ff        stop
1e94: e2 11     set7  $11
1e96: ea 78 ee  not1  $1dcf,0
1e99: fe e4     dbnz  y,$1e7f
1e9b: 00        nop
1e9c: 82 78     set4  $78
1e9e: e6        mov   a,(x)
1e9f: 61        tcall 6
1ea0: 18 05 7d  or    $7d,#$05
1ea3: 7f        reti
1ea4: e0        clrv
1ea5: 02 e7     set0  $e7
1ea7: 1e 00 14  cmp   x,$1400
1eaa: ff        stop
1eab: ad 1e     cmp   y,#$1e
1ead: ea 32 ee  not1  $1dc6,2
1eb0: fe e2     dbnz  y,$1e94
1eb2: 1b 1f     asl   $1f+x
1eb4: 0f        brk
1eb5: 7d        mov   a,x
1eb6: 7f        reti
1eb7: f3 00 14  bbc7  $00,$1ece
1eba: 2d        push  a
1ebb: fc        inc   y
1ebc: b3 00 1f  bbc5  $00,$1ede
1ebf: 0a c1 f3  or1   c,$1e78,1
1ec2: 00        nop
1ec3: 14 2d     or    a,$2d+x
1ec5: fc        inc   y
1ec6: b3 00 ff  bbc5  $00,$1ec8
1ec9: cb 1e     mov   $1e,y
1ecb: ea 64 ee  not1  $1dcc,4
1ece: fe e2     dbnz  y,$1eb2
1ed0: 1b 23     asl   $23+x
1ed2: 0a 7d 7f  or1   c,$0fef,5
1ed5: f3 00 09  bbc7  $00,$1ee1
1ed8: 21        tcall 2
1ed9: fc        inc   y
1eda: c8 ff     cmp   x,#$ff
1edc: 23 09 ca  bbs1  $09,$1ea9
1edf: f3 00 09  bbc7  $00,$1eeb
1ee2: 21        tcall 2
1ee3: fc        inc   y
1ee4: c8 ff     cmp   x,#$ff
1ee6: a3 b2 f3  bbs5  $b2,$1edc
1ee9: 00        nop
1eea: 09 21 fc  or    ($fc),($21)
1eed: c8 ff     cmp   x,#$ff
1eef: ff        stop
1ef0: f2 1e     clr7  $1e
1ef2: e2 1c     set7  $1c
1ef4: ea 64 ee  not1  $1dcc,4
1ef7: fe e4     dbnz  y,$1edd
1ef9: 00        nop
1efa: 14 19     or    a,$19+x
1efc: 1e 21 7d  cmp   x,$7d21
1eff: 7f        reti
1f00: f3 00 25  bbc7  $00,$1f28
1f03: 17 fc     or    a,($fc)+y
1f05: d0 ff     bne   $1f06
1f07: 1e 19 c6  cmp   x,$c619
1f0a: f3 00 25  bbc7  $00,$1f32
1f0d: 17 fc     or    a,($fc)+y
1f0f: d0 ff     bne   $1f10
1f11: ff        stop
1f12: e2 11     set7  $11
1f14: ea 78 ee  not1  $1dcf,0
1f17: fe e4     dbnz  y,$1efd
1f19: 00        nop
1f1a: 82 78     set4  $78
1f1c: e6        mov   a,(x)
1f1d: 61        tcall 6
1f1e: 18 05 7d  or    $7d,#$05
1f21: 7f        reti
1f22: e0        clrv
1f23: 02 e7     set0  $e7
1f25: 1e 00 14  cmp   x,$1400
1f28: ff        stop
1f29: 2b 1f     rol   $1f
1f2b: e2 1c     set7  $1c
1f2d: ea 64 ee  not1  $1dcc,4
1f30: fe e4     dbnz  y,$1f16
1f32: 00        nop
1f33: 14 23     or    a,$23+x
1f35: 1d        dec   x
1f36: 0e 7d 7f  tset1 $7f7d
1f39: f3 00 04  bbc7  $00,$1f40
1f3c: 18 fc c0  or    $c0,#$fc
1f3f: fe f3     dbnz  y,$1f34
1f41: 00        nop
1f42: 08 1f     or    a,#$1f
1f44: fc        inc   y
1f45: e0        clrv
1f46: 00        nop
1f47: 1d        dec   x
1f48: 0d        push  psw
1f49: c6        mov   (x),a
1f4a: f3 00 04  bbc7  $00,$1f51
1f4d: 18 fc c0  or    $c0,#$fc
1f50: fe f3     dbnz  y,$1f45
1f52: 00        nop
1f53: 08 1f     or    a,#$1f
1f55: fc        inc   y
1f56: e0        clrv
1f57: 00        nop
1f58: ff        stop
1f59: e2 11     set7  $11
1f5b: ea 78 ee  not1  $1dcf,0
1f5e: fe e4     dbnz  y,$1f44
1f60: 00        nop
1f61: 82 78     set4  $78
1f63: e6        mov   a,(x)
1f64: 61        tcall 6
1f65: 18 05 7d  or    $7d,#$05
1f68: 7f        reti
1f69: e0        clrv
1f6a: 02 e7     set0  $e7
1f6c: 1e 00 14  cmp   x,$1400
1f6f: ff        stop
1f70: 72 1f     clr3  $1f
1f72: ea 96 e2  not1  $1c52,6
1f75: 1c        asl   a
1f76: ee        pop   y
1f77: fe ec     dbnz  y,$1f65
1f79: f2 48     clr7  $48
1f7b: 07 65     or    a,($65+x)
1f7d: 67 f3     cmp   a,($f3+x)
1f7f: 00        nop
1f80: 06        or    a,(x)
1f81: 0c fc 00  asl   $00fc
1f84: f6 3e 0c  mov   a,$0c3e+y
1f87: ff        stop
1f88: f3 00 0b  bbc7  $00,$1f96
1f8b: 1d        dec   x
1f8c: fc        inc   y
1f8d: 00        nop
1f8e: fd        mov   y,a
1f8f: be        das   a
1f90: e8 f3     mov   a,#$f3
1f92: 00        nop
1f93: 0b 1d     asl   $1d
1f95: fc        inc   y
1f96: 00        nop
1f97: fd        mov   y,a
1f98: be        das   a
1f99: d0 f3     bne   $1f8e
1f9b: 00        nop
1f9c: 0c 1d fc  asl   $fc1d
1f9f: 40        setp
1fa0: fd        mov   y,a
1fa1: ff        stop
1fa2: a6        sbc   a,(x)
1fa3: 1f b7 1f  jmp   ($1fb7+x)
1fa6: ea e6 ee  not1  $1ddc,6
1fa9: fc        inc   y
1faa: e2 07     set7  $07
1fac: e3 0c e6  bbs7  $0c,$1f95
1faf: fe ce     dbnz  y,$1f7f
1fb1: 1f e7 05  jmp   ($05e7+x)
1fb4: ec 00 ff  mov   y,$ff00
1fb7: ea e6 ee  not1  $1ddc,6
1fba: f0 e2     beq   $1f9e
1fbc: 07 e3     or    a,($e3+x)
1fbe: 06        or    a,(x)
1fbf: f2 05     clr7  $05
1fc1: 3e 0e     cmp   x,$0e
1fc3: 7f        reti
1fc4: 7b e6     ror   $e6+x
1fc6: fe ce     dbnz  y,$1f96
1fc8: 1f e7 05  jmp   ($05e7+x)
1fcb: ec 00 ff  mov   y,$ff00
1fce: 61        tcall 6
1fcf: 3e 0a     cmp   x,$0a
1fd1: 7f        reti
1fd2: 7b b9     ror   $b9+x
1fd4: ff        stop
1fd5: c2 fb     set6  $fb
1fd7: c5 ff ff  mov   $ffff,a
1fda: de 1f f9  cbne  $1f+x,$1fd6
1fdd: 1f e2 0d  jmp   ($0de2+x)
1fe0: ea 8c ee  not1  $1dd1,4
1fe3: b6 ec 07  sbc   a,$07ec+y
1fe6: e4 00     mov   a,$00
1fe8: 46        eor   a,(x)
1fe9: 00        nop
1fea: fe 10     dbnz  y,$1ffc
1fec: 20        clrp
1fed: f1        tcall 15
1fee: 00        nop
1fef: 05 fe fc  or    a,$fcfe
1ff2: 9a ff     subw  ya,$ff
1ff4: b7 d6     sbc   a,($d6)+y
1ff6: b7 b5     sbc   a,($b5)+y
1ff8: ff        stop
1ff9: e2 0e     set7  $0e
1ffb: ea 8c ee  not1  $1dd1,4
1ffe: d8 ec     mov   $ec,x
2000: 0c fe 10  asl   $10fe
2003: 20        clrp
2004: f1        tcall 15
2005: 00        nop
2006: 05 02 fc  or    a,$fc02
2009: 66        cmp   a,(x)
200a: 00        nop
200b: b7 d6     sbc   a,($d6)+y
200d: b7 b5     sbc   a,($b5)+y
200f: ff        stop
2010: fa 82 de  mov   ($de),($82)
2013: 00        nop
2014: f1        tcall 15
2015: 00        nop
2016: 04 05     or    a,$05
2018: fc        inc   y
2019: 40        setp
201a: 01        tcall 0
201b: 37 06     and   a,($06)+y
201d: 7f        reti
201e: 78 f1 00  cmp   $00,#$f1
2021: 04 05     or    a,$05
2023: fc        inc   y
2024: 40        setp
2025: 01        tcall 0
2026: b7 f8     sbc   a,($f8)+y
2028: f1        tcall 15
2029: 00        nop
202a: 05 07 fc  or    a,$fc07
202d: 66        cmp   a,(x)
202e: 01        tcall 0
202f: 37 14     and   a,($14)+y
2031: f8 ff     mov   x,$ff
2033: 37 20     and   a,($20)+y
2035: 43 20 e2  bbs2  $20,$201a
2038: 10 ea     bpl   $2024
203a: 64 ee     cmp   a,$ee
203c: fe e3     dbnz  y,$2021
203e: 0c fe 53  asl   $53fe
2041: 20        clrp
2042: ff        stop
2043: ec ff e2  mov   y,$e2ff
2046: 10 ea     bpl   $2032
2048: 64 ee     cmp   a,$ee
204a: fe e3     dbnz  y,$202f
204c: 08 e0     or    a,#$e0
204e: 01        tcall 0
204f: fe 53     dbnz  y,$20a4
2051: 20        clrp
2052: ff        stop
2053: 15 0a 7d  or    a,$7d0a+x
2056: 78 23 28  cmp   $28,#$23
2059: ff        stop
205a: f3 00 09  bbc7  $00,$2066
205d: 1f fc 8f  jmp   ($8ffc+x)
2060: ff        stop
2061: 21        tcall 2
2062: 0f        brk
2063: bc        inc   a
2064: f3 00 09  bbc7  $00,$2070
2067: 1d        dec   x
2068: fc        inc   y
2069: 8f ff ff  mov   $ff,#$ff
206c: 70 20     bvs   $208e
206e: 7b 20     ror   $20+x
2070: e2 10     set7  $10
2072: ea 64 ee  not1  $1dcc,4
2075: fe 27     dbnz  y,$209e
2077: 32 7d     clr1  $7d
2079: 7f        reti
207a: ff        stop
207b: e2 01     set7  $01
207d: ea ff ee  not1  $1ddf,7
2080: 84 fa     adc   a,$fa
2082: 78 ed 00  cmp   $00,#$ed
2085: ec 00 e8  mov   y,$e800
2088: f6 e6 f2  mov   a,$f2e6+y
208b: 00        nop
208c: 61        tcall 6
208d: 56 01 7f  eor   a,$7f01+y
2090: 7f        reti
2091: f2 0a     clr7  $0a
2093: d6 ff e7  mov   $e7ff+y,a
2096: 04 00     or    a,$00
2098: 00        nop
2099: f7 e0     mov   a,($e0)+y
209b: 01        tcall 0
209c: ee        pop   y
209d: 64 f7     cmp   a,$f7
209f: e0        clrv
20a0: 01        tcall 0
20a1: ee        pop   y
20a2: 50 f7     bvc   $209b
20a4: e9 02 ec  mov   x,$ec02
20a7: fc        inc   y
20a8: ff        stop
20a9: ad 20     cmp   y,#$20
20ab: c2 20     set6  $20
20ad: ea aa ee  not1  $1dd5,2
20b0: fa e2 0b  mov   ($0b),($e2)
20b3: fa 82 dd  mov   ($dd),($82)
20b6: 00        nop
20b7: e3 0a e6  bbs7  $0a,$20a0
20ba: fe e0     dbnz  y,$209c
20bc: 20        clrp
20bd: e7 04     mov   a,($04+x)
20bf: ec 00 ff  mov   y,$ff00
20c2: ea aa ee  not1  $1dd5,2
20c5: e2 e2     set7  $e2
20c7: 0b fa     asl   $fa
20c9: 82 dd     set4  $dd
20cb: 00        nop
20cc: e3 11 f8  bbs7  $11,$20c7
20cf: fa 04 f2  mov   ($f2),($04)
20d2: 0a ec 11  or1   c,$023d,4
20d5: e0        clrv
20d6: 04 e6     or    a,$e6
20d8: fe e0     dbnz  y,$20ba
20da: 20        clrp
20db: e7 04     mov   a,($04+x)
20dd: ec 00 ff  mov   y,$ff00
20e0: 61        tcall 6
20e1: 30 06     bmi   $20e9
20e3: 7f        reti
20e4: 78 30 04  cmp   $04,#$30
20e7: f4 32     mov   a,$32+x
20e9: 06        or    a,(x)
20ea: f8 32     mov   x,$32
20ec: 04 f4     or    a,$f4
20ee: 34 06     and   a,$06+x
20f0: f8 34     mov   x,$34
20f2: 04 f4     or    a,$f4
20f4: 37 06     and   a,($06)+y
20f6: f8 37     mov   x,$37
20f8: 04 f4     or    a,$f4
20fa: 3c        rol   a
20fb: 06        or    a,(x)
20fc: f8 3c     mov   x,$3c
20fe: 04 f4     or    a,$f4
2100: ff        stop
2101: 05 21 43  or    a,$4321
2104: 21        tcall 2
2105: ea a0 ee  not1  $1dd4,0
2108: fa e2 0f  mov   ($0f),($e2)
210b: e3 09 e4  bbs7  $09,$20f2
210e: 00        nop
210f: 5a 3c     cmpw  ya,$3c
2111: fa 78 e2  mov   ($e2),($78)
2114: 00        nop
2115: e0        clrv
2116: 0f        brk
2117: ef        sleep
2118: 30 8c     bmi   $20a6
211a: 2b 30     rol   $30
211c: 7d        mov   a,x
211d: 7f        reti
211e: f3 03 05  bbc7  $03,$2126
2121: 42 fc     set2  $fc
2123: 99        adc   (x),(y)
2124: 04 f3     or    a,$f3
2126: 00        nop
2127: 2b 20     rol   $20
2129: fc        inc   y
212a: 36 ff ee  and   a,$eeff+y
212d: c6        mov   (x),a
212e: 44 19     eor   a,$19
2130: c4 f3     mov   $f3,a
2132: 00        nop
2133: 19        or    (x),(y)
2134: 1f fc 86  jmp   ($86fc+x)
2137: fe c3     dbnz  y,$20fc
2139: 7f        reti
213a: 38 f3 00  and   $00,#$f3
213d: 19        or    (x),(y)
213e: 1e fc 86  cmp   x,$86fc
2141: fe ff     dbnz  y,$2142
2143: ea a0 ee  not1  $1dd4,0
2146: f0 e2     beq   $212a
2148: 13 e3 09  bbc0  $e3,$2154
214b: e4 00     mov   a,$00
214d: 5a 7f     cmpw  ya,$7f
214f: fa 73 e4  mov   ($e4),($73)
2152: 00        nop
2153: e0        clrv
2154: 0f        brk
2155: 33 30 7d  bbc1  $30,$21d5
2158: 75 f3 00  cmp   a,$00f3+x
215b: 30 24     bmi   $2181
215d: fc        inc   y
215e: b0 ff     bcs   $215f
2160: 33 19 cf  bbc1  $19,$2132
2163: f3 00 19  bbc7  $00,$217f
2166: 24 fc     and   a,$fc
2168: 67 ff     cmp   a,($ff+x)
216a: b3 7f 3d  bbc5  $7f,$21aa
216d: f3 00 19  bbc7  $00,$2189
2170: 24 fc     and   a,$fc
2172: 67 ff     cmp   a,($ff+x)
2174: ff        stop
2175: 79        cmp   (x),(y)
2176: 21        tcall 2
2177: 85 21 ea  adc   a,$ea21
217a: a0        ei
217b: ee        pop   y
217c: ca e2 0b  mov1  $017c,2,c
217f: e3 07 fe  bbs7  $07,$2180
2182: 91        tcall 9
2183: 21        tcall 2
2184: ff        stop
2185: ea a0 ee  not1  $1dd4,0
2188: ee        pop   y
2189: e2 06     set7  $06
218b: e3 0a fe  bbs7  $0a,$218c
218e: 91        tcall 9
218f: 21        tcall 2
2190: ff        stop
2191: 2a 06 7d  or1   c,!($0fa0,6)
2194: 7f        reti
2195: f3 00 0a  bbc7  $00,$21a2
2198: 05 fc 4d  or    a,$4dfc
219b: fc        inc   y
219c: 2a 08 65  or1   c,!($0ca1,0)
219f: 5a f3     cmpw  ya,$f3
21a1: 00        nop
21a2: 0e 03 fc  tset1 $fc03
21a5: 37 fd     and   a,($fd)+y
21a7: 2a 1a 7d  or1   c,!($0fa3,2)
21aa: 7f        reti
21ab: f3 00 14  bbc7  $00,$21c2
21ae: 05 fc 27  or    a,$27fc
21b1: fe 2b     dbnz  y,$21de
21b3: 10 ee     bpl   $21a3
21b5: f3 00 0e  bbc7  $00,$21c6
21b8: 03 fc 25  bbs0  $fc,$21e0
21bb: fd        mov   y,a
21bc: 2c 0e db  rol   $db0e
21bf: f3 00 0e  bbc7  $00,$21d0
21c2: 03 fc 13  bbs0  $fc,$21d8
21c5: fd        mov   y,a
21c6: ad d0     cmp   y,#$d0
21c8: f3 00 0e  bbc7  $00,$21d9
21cb: 03 fc 00  bbs0  $fc,$21ce
21ce: fd        mov   y,a
21cf: ff        stop
21d0: d4 21     mov   $21+x,a
21d2: 01        tcall 0
21d3: 22 ea     set1  $ea
21d5: ff        stop
21d6: ee        pop   y
21d7: d8 e2     mov   $e2,x
21d9: 0e e3 08  tset1 $08e3
21dc: e6        mov   a,(x)
21dd: 61        tcall 6
21de: 3c        rol   a
21df: 01        tcall 0
21e0: 7f        reti
21e1: 78 c8 f8  cmp   $f8,#$c8
21e4: e7 19     mov   a,($19+x)
21e6: 00        nop
21e7: f3 e6 61  bbc7  $e6,$224b
21ea: 3c        rol   a
21eb: 01        tcall 0
21ec: 7f        reti
21ed: 5a c8     cmpw  ya,$c8
21ef: da e7     movw  $e7,ya
21f1: 19        or    (x),(y)
21f2: 00        nop
21f3: f3 e6 61  bbc7  $e6,$2257
21f6: 3c        rol   a
21f7: 01        tcall 0
21f8: 7f        reti
21f9: 41        tcall 4
21fa: c8 c1     cmp   x,#$c1
21fc: e7 19     mov   a,($19+x)
21fe: 00        nop
21ff: f3 ff ea  bbc7  $ff,$21ec
2202: ff        stop
2203: ee        pop   y
2204: ce        pop   x
2205: e2 0e     set7  $0e
2207: f2 08     clr7  $08
2209: ec 0c e3  mov   y,$e30c
220c: 0c e6 61  asl   $61e6
220f: 3c        rol   a
2210: 01        tcall 0
2211: 7f        reti
2212: 78 c8 f8  cmp   $f8,#$c8
2215: e7 19     mov   a,($19+x)
2217: 00        nop
2218: ec e6 61  mov   y,$61e6
221b: 3c        rol   a
221c: 01        tcall 0
221d: 7f        reti
221e: 5a c8     cmpw  ya,$c8
2220: da e7     movw  $e7,ya
2222: 19        or    (x),(y)
2223: 00        nop
2224: ec e6 61  mov   y,$61e6
2227: 3c        rol   a
2228: 01        tcall 0
2229: 7f        reti
222a: 41        tcall 4
222b: c8 c1     cmp   x,#$c1
222d: e7 19     mov   a,($19+x)
222f: 00        nop
2230: ec ff e2  mov   y,$e2ff
2233: 11        tcall 1
2234: ea 78 ee  not1  $1dcf,0
2237: fe e4     dbnz  y,$221d
2239: 00        nop
223a: 82 78     set4  $78
223c: e6        mov   a,(x)
223d: 61        tcall 6
223e: 18 05 7d  or    $7d,#$05
2241: 7f        reti
2242: e0        clrv
2243: 02 e7     set0  $e7
2245: 1e 00 14  cmp   x,$1400
2248: ff        stop
2249: 4d        push  x
224a: 22 67     set1  $67
224c: 22 ea     set1  $ea
224e: a0        ei
224f: ee        pop   y
2250: c6        mov   (x),a
2251: e2 03     set7  $03
2253: f2 05     clr7  $05
2255: e3 0c e0  bbs7  $0c,$2238
2258: 01        tcall 0
2259: fe 7d     dbnz  y,$22d8
225b: 22 3d     set1  $3d
225d: 0a bb f3  or1   c,$1e77,3
2260: 00        nop
2261: 0a 17 fc  or1   c,$1f82,7
2264: 34 fc     and   a,$fc+x
2266: ff        stop
2267: ea a0 ee  not1  $1dd4,0
226a: ea e2 03  not1  $007c,2
226d: e3 09 fe  bbs7  $09,$226e
2270: 7d        mov   a,x
2271: 22 3d     set1  $3d
2273: 0a bd f3  or1   c,$1e77,5
2276: 00        nop
2277: 0a 17 fc  or1   c,$1f82,7
227a: 34 fc     and   a,$fc+x
227c: ff        stop
227d: e4 00     mov   a,$00
227f: 5a 78     cmpw  ya,$78
2281: 34 0b     and   a,$0b+x
2283: 7f        reti
2284: 7f        reti
2285: f3 00 04  bbc7  $00,$228c
2288: 11        tcall 1
2289: fc        inc   y
228a: 40        setp
228b: f7 41     mov   a,($41)+y
228d: 0c fd f3  asl   $f3fd
2290: 00        nop
2291: 0c 18 fc  asl   $fc18
2294: 96 fc bf  adc   a,$bffc+y
2297: da f3     movw  $f3,ya
2299: 00        nop
229a: 0c 17 fc  asl   $fc17
229d: ab fc     inc   $fc
229f: be        das   a
22a0: cf        mul   ya
22a1: f3 00 0c  bbc7  $00,$22b0
22a4: 17 fc     or    a,($fc)+y
22a6: c0        di
22a7: fc        inc   y
22a8: ff        stop
22a9: ad 22     cmp   y,#$22
22ab: 09 23 ea  or    ($ea),($23)
22ae: a0        ei
22af: ee        pop   y
22b0: ce        pop   x
22b1: e2 03     set7  $03
22b3: e3 0c f8  bbs7  $0c,$22ae
22b6: 7a 08     addw  ya,$08
22b8: e4 00     mov   a,$00
22ba: 5a 46     cmpw  ya,$46
22bc: 34 0b     and   a,$0b+x
22be: 7f        reti
22bf: 7f        reti
22c0: f3 00 06  bbc7  $00,$22c9
22c3: 11        tcall 1
22c4: fc        inc   y
22c5: 2b fa     rol   $fa
22c7: b4 ff     sbc   a,$ff+x
22c9: f3 00 06  bbc7  $00,$22d2
22cc: 11        tcall 1
22cd: fc        inc   y
22ce: 2b fa     rol   $fa
22d0: 41        tcall 4
22d1: 16 f8 f3  or    a,$f3f8+y
22d4: 00        nop
22d5: 16 18 fc  or    a,$fc18+y
22d8: 23 fe c1  bbs1  $fe,$229c
22db: c6        mov   (x),a
22dc: f3 00 16  bbc7  $00,$22f5
22df: 18 fc 23  or    $23,#$fc
22e2: fe 41     dbnz  y,$2325
22e4: 0d        push  psw
22e5: fa f3 00  mov   ($00),($f3)
22e8: 0c 18 fc  asl   $fc18
22eb: 96 fc bf  adc   a,$bffc+y
22ee: d0 f3     bne   $22e3
22f0: 00        nop
22f1: 0c 17 fc  asl   $fc17
22f4: ab fc     inc   $fc
22f6: be        das   a
22f7: c1        tcall 12
22f8: f3 00 0c  bbc7  $00,$2307
22fb: 17 fc     or    a,($fc)+y
22fd: c0        di
22fe: fc        inc   y
22ff: be        das   a
2300: b7 f3     sbc   a,($f3)+y
2302: 00        nop
2303: 0c 17 fc  asl   $fc17
2306: c0        di
2307: fc        inc   y
2308: ff        stop
2309: ea a0 ee  not1  $1dd4,0
230c: f8 e2     mov   x,$e2
230e: 0f        brk
230f: e4 00     mov   a,$00
2311: 2d        push  a
2312: 32 1f     clr1  $1f
2314: 0b 7f     asl   $7f
2316: 6e f3 00  dbnz  $f3,$2319
2319: 14 45     or    a,$45+x
231b: fc        inc   y
231c: e6        mov   a,(x)
231d: 01        tcall 0
231e: 9f        xcn   a
231f: f8 f3     mov   x,$f3
2321: 00        nop
2322: 14 45     or    a,$45+x
2324: fc        inc   y
2325: e6        mov   a,(x)
2326: 01        tcall 0
2327: 1f 16 ff  jmp   ($ff16+x)
232a: f3 02 16  bbc7  $02,$2343
232d: 46        eor   a,(x)
232e: fc        inc   y
232f: c5 01 9f  mov   $9f01,a
2332: c6        mov   (x),a
2333: f3 02 16  bbc7  $02,$234c
2336: 45 fc ba  eor   a,$bafc
2339: 01        tcall 0
233a: 41        tcall 4
233b: 0d        push  psw
233c: fa f3 00  mov   ($00),($f3)
233f: 0b 25     asl   $25
2341: fc        inc   y
2342: 75 fd bf  cmp   a,$bffd+x
2345: d0 f3     bne   $233a
2347: 00        nop
2348: 0b 24     asl   $24
234a: fc        inc   y
234b: 8c fd be  dec   $befd
234e: c1        tcall 12
234f: f3 00 0b  bbc7  $00,$235d
2352: 24 fc     and   a,$fc
2354: a3 fd 3e  bbs5  $fd,$2395
2357: 0c b7 f3  asl   $f3b7
235a: 00        nop
235b: 0b 24     asl   $24
235d: fc        inc   y
235e: a3 fd ff  bbs5  $fd,$2360
2361: 65 23 9b  cmp   a,$9b23
2364: 23 ea 80  bbs1  $ea,$22e7
2367: ee        pop   y
2368: c0        di
2369: e2 02     set7  $02
236b: ec 0c e3  mov   y,$e30c
236e: 09 e0 02  or    ($02),($e0)
2371: fa 64 e9  mov   ($e9),($64)
2374: 00        nop
2375: ec f8 fe  mov   y,$fef8
2378: cd 23     mov   x,#$23
237a: 98 f0 f3  adc   $f3,#$f0
237d: 00        nop
237e: 08 3c     or    a,#$3c
2380: fc        inc   y
2381: 80        setc
2382: 04 f3     or    a,$f3
2384: 00        nop
2385: 0c 1c fc  asl   $fc1c
2388: 56 fd 97  eor   a,$97fd+y
238b: c4 f3     mov   $f3,a
238d: 00        nop
238e: 08 3b     or    a,#$3b
2390: fc        inc   y
2391: 80        setc
2392: 04 f3     or    a,$f3
2394: 00        nop
2395: 0c 1b fc  asl   $fc1b
2398: 56 fd ff  eor   a,$fffd+y
239b: ea 80 ee  not1  $1dd0,0
239e: f6 e2 03  mov   a,$03e2+y
23a1: e3 0b ec  bbs7  $0b,$2390
23a4: f5 fa 64  mov   a,$64fa+x
23a7: e9 00 fe  mov   x,$fe00
23aa: cd 23     mov   x,#$23
23ac: 98 f4 f3  adc   $f3,#$f4
23af: 00        nop
23b0: 08 3c     or    a,#$3c
23b2: fc        inc   y
23b3: 80        setc
23b4: 04 f3     or    a,$f3
23b6: 00        nop
23b7: 0c 1c fc  asl   $fc1c
23ba: 56 fd 97  eor   a,$97fd+y
23bd: c6        mov   (x),a
23be: f3 00 08  bbc7  $00,$23c9
23c1: 3b fc     rol   $fc+x
23c3: 80        setc
23c4: 04 f3     or    a,$f3
23c6: 00        nop
23c7: 0c 1b fc  asl   $fc1b
23ca: 56 fd ff  eor   a,$fffd+y
23cd: 18 14 7d  or    $7d,#$14
23d0: 7f        reti
23d1: f3 00 08  bbc7  $00,$23dc
23d4: 3c        rol   a
23d5: fc        inc   y
23d6: 80        setc
23d7: 04 f3     or    a,$f3
23d9: 00        nop
23da: 0c 1c fc  asl   $fc1c
23dd: 56 fd ff  eor   a,$fffd+y
23e0: e4 23     mov   a,$23
23e2: ff        stop
23e3: 23 ea fa  bbs1  $ea,$23e0
23e6: ee        pop   y
23e7: bc        inc   a
23e8: e2 00     set7  $00
23ea: e4 00     mov   a,$00
23ec: 32 7f     clr1  $7f
23ee: e3 0d f8  bbs7  $0d,$23e9
23f1: 8c 08 ec  dec   $ec08
23f4: f9 3e     mov   x,$3e+y
23f6: 3c        rol   a
23f7: 7f        reti
23f8: 78 3e 28  cmp   $28,#$3e
23fb: da be     movw  $be,ya
23fd: c3 ff ea  bbs6  $ff,$23ea
2400: fa ee dc  mov   ($dc),($ee)
2403: e2 00     set7  $00
2405: e4 00     mov   a,$00
2407: 32 7f     clr1  $7f
2409: e3 09 f8  bbs7  $09,$2404
240c: 8c 0c 3e  dec   $3e0c
240f: 3c        rol   a
2410: 7f        reti
2411: 78 3e 28  cmp   $28,#$3e
2414: da be     movw  $be,ya
2416: c3 ff 1c  bbs6  $ff,$2435
2419: 24 4e     and   a,$4e
241b: 24 ea     and   a,$ea
241d: dc        dec   y
241e: ee        pop   y
241f: fe e2     dbnz  y,$2403
2421: 0e e3 0d  tset1 $0de3
2424: f8 50     mov   x,$50
2426: 07 35     or    a,($35+x)
2428: 14 7d     or    a,$7d+x
242a: 7d        mov   a,x
242b: f3 00 10  bbc7  $00,$243e
242e: 17 fc     or    a,($fc)+y
2430: 20        clrp
2431: fe b9     dbnz  y,$23ec
2433: e9 f3 00  mov   x,$00f3
2436: 12 17     clr0  $17
2438: fc        inc   y
2439: 1d        dec   x
243a: fe b9     dbnz  y,$23f5
243c: e1        tcall 14
243d: f3 00 12  bbc7  $00,$2452
2440: 17 fc     or    a,($fc)+y
2442: 1d        dec   x
2443: fe b9     dbnz  y,$23fe
2445: cc f3 00  mov   $00f3,y
2448: 12 17     clr0  $17
244a: fc        inc   y
244b: 1d        dec   x
244c: fe ff     dbnz  y,$244d
244e: ea a0 ee  not1  $1dd4,0
2451: fe e2     dbnz  y,$2435
2453: 0f        brk
2454: e4 00     mov   a,$00
2456: 32 78     clr1  $78
2458: 1c        asl   a
2459: 1e 7d 78  cmp   x,$787d
245c: f3 00 1e  bbc7  $00,$247d
245f: 50 fc     bvc   $245d
2461: bb 01     inc   $01+x
2463: 9c        dec   a
2464: d3 f3 00  bbc6  $f3,$2467
2467: 1e 50 fc  cmp   x,$fc50
246a: bb 01     inc   $01+x
246c: ef        sleep
246d: 1e 64 9c  cmp   x,$9c64
2470: c9 f3 00  mov   $00f3,x
2473: 1e 4e fc  cmp   x,$fc4e
2476: aa 01 ff  mov1  c,$1fe0,1
2479: 7d        mov   a,x
247a: 24 89     and   a,$89
247c: 24 ec     and   a,$ec
247e: 0c ea b4  asl   $b4ea
2481: ee        pop   y
2482: e2 e2     set7  $e2
2484: 07 fe     or    a,($fe+x)
2486: 93 24 ff  bbc4  $24,$2488
2489: ea b4 ee  not1  $1dd6,4
248c: ec e2 0f  mov   y,$0fe2
248f: fe 93     dbnz  y,$2424
2491: 24 ff     and   a,$ff
2493: e4 00     mov   a,$00
2495: 28 87     and   a,#$87
2497: f9 14     mov   x,$14+y
2499: 48 41     eor   a,#$41
249b: 7d        mov   a,x
249c: 7b f3     ror   $f3+x
249e: 00        nop
249f: 2e 36 fc  cbne  $36,$249e
24a2: 9c        dec   a
24a3: ff        stop
24a4: 3c        rol   a
24a5: 1e d0 f3  cmp   x,$f3d0
24a8: 00        nop
24a9: 28 31     and   a,#$31
24ab: fc        inc   y
24ac: ba ff     movw  ya,$ff
24ae: bc        inc   a
24af: b2 f3     clr5  $f3
24b1: 00        nop
24b2: 28 31     and   a,#$31
24b4: fc        inc   y
24b5: ba ff     movw  ya,$ff
24b7: ff        stop
24b8: bc        inc   a
24b9: 24 ed     and   a,$ed
24bb: 24 ea     and   a,$ea
24bd: a0        ei
24be: ee        pop   y
24bf: dc        dec   y
24c0: e2 13     set7  $13
24c2: e3 09 e4  bbs7  $09,$24a9
24c5: 00        nop
24c6: 5a 7f     cmpw  ya,$7f
24c8: ec fd fa  mov   y,$fafd
24cb: 6e de 00  dbnz  $de,$24ce
24ce: 37 26     and   a,($26)+y
24d0: 7f        reti
24d1: 73 f3 00  bbc3  $f3,$24d4
24d4: 26        and   a,(x)
24d5: 24 fc     and   a,$fc
24d7: 80        setc
24d8: ff        stop
24d9: 37 1e     and   a,($1e)+y
24db: da f3     movw  $f3,ya
24dd: 00        nop
24de: 1e 24 fc  cmp   x,$fc24
24e1: 5e ff b7  cmp   y,$b7ff
24e4: c4 f3     mov   $f3,a
24e6: 00        nop
24e7: 1e 24 fc  cmp   x,$fc24
24ea: 5e ff ff  cmp   y,$ffff
24ed: ea a0 ee  not1  $1dd4,0
24f0: dc        dec   y
24f1: e2 0f     set7  $0f
24f3: e3 09 e4  bbs7  $09,$24da
24f6: 00        nop
24f7: 00        nop
24f8: 32 fa     clr1  $fa
24fa: 6e de 00  dbnz  $de,$24fd
24fd: ef        sleep
24fe: 2e ce 2b  cbne  $ce,$252c
2501: 26        and   a,(x)
2502: 7f        reti
2503: 7f        reti
2504: f3 03 05  bbc7  $03,$250c
2507: 42 fc     set2  $fc
2509: 99        adc   (x),(y)
250a: 04 f3     or    a,$f3
250c: 00        nop
250d: 1e 20 fc  cmp   x,$fc20
2510: de fe ee  cbne  $fe+x,$2501
2513: f0 44     beq   $2559
2515: 1e de f3  cmp   x,$f3de
2518: 00        nop
2519: 1e 1f fc  cmp   x,$fc1f
251c: c5 fe c4  mov   $c4fe,a
251f: c4 f3     mov   $f3,a
2521: 00        nop
2522: 1e 1f fc  cmp   x,$fc1f
2525: c5 fe ff  mov   $fffe,a
2528: 2c 25 3f  rol   $3f25
252b: 25 ea f0  and   a,$f0ea
252e: ee        pop   y
252f: ca e2 09  mov1  $013c,2,c
2532: f2 05     clr7  $05
2534: e3 07 e6  bbs7  $07,$251d
2537: fe 50     dbnz  y,$2589
2539: 25 e7 0e  and   a,$0ee7
253c: f9 00     mov   x,$00+y
253e: ff        stop
253f: ea f0 ee  not1  $1dde,0
2542: f0 e2     beq   $2526
2544: 02 e3     set0  $e3
2546: 09 e6 fe  or    ($fe),($e6)
2549: 50 25     bvc   $2570
254b: e7 0e     mov   a,($0e+x)
254d: f9 00     mov   x,$00+y
254f: ff        stop
2550: 61        tcall 6
2551: 54 0c     eor   a,$0c+x
2553: 7d        mov   a,x
2554: 7f        reti
2555: f3 00 0a  bbc7  $00,$2562
2558: 20        clrp
2559: fc        inc   y
255a: cd fa     mov   x,#$fa
255c: ff        stop
255d: 61        tcall 6
255e: 25 7b 25  and   a,$257b
2561: ea c8 ea  not1  $1d59,0
2564: c8 ee     cmp   x,#$ee
2566: fe e2     dbnz  y,$254a
2568: 1d        dec   x
2569: e3 07 ef  bbs7  $07,$255b
256c: 23 d2 3c  bbs1  $d2,$25ab
256f: 28 7f     and   a,#$7f
2571: 7f        reti
2572: ee        pop   y
2573: ee        pop   y
2574: ef        sleep
2575: 50 48     bvc   $25bf
2577: 3c        rol   a
2578: 55 ff ff  eor   a,$ffff+x
257b: ea c8 ee  not1  $1dd9,0
257e: fa e2 1d  mov   ($1d),($e2)
2581: e3 0d e0  bbs7  $0d,$2564
2584: 08 ef     or    a,#$ef
2586: 23 be 40  bbs1  $be,$25c9
2589: 28 7f     and   a,#$7f
258b: 7f        reti
258c: ee        pop   y
258d: c8 ef     cmp   x,#$ef
258f: 55 64 40  eor   a,$4064+x
2592: 55 ff ff  eor   a,$ffff+x
2595: 99        adc   (x),(y)
2596: 25 d1 25  and   a,$25d1
2599: ea a0 ee  not1  $1dd4,0
259c: fe e2     dbnz  y,$2580
259e: 0e e3 0d  tset1 $0de3
25a1: e6        mov   a,(x)
25a2: 61        tcall 6
25a3: 35 0d 72  and   a,$720d+x
25a6: 78 f3 00  cmp   $00,#$f3
25a9: 0b 17     asl   $17
25ab: fc        inc   y
25ac: 46        eor   a,(x)
25ad: fd        mov   y,a
25ae: e7 03     mov   a,($03+x)
25b0: fd        mov   y,a
25b1: 00        nop
25b2: ea dc 35  not1  $06bb,4
25b5: 14 7d     or    a,$7d+x
25b7: 7f        reti
25b8: f3 00 10  bbc7  $00,$25cb
25bb: 17 fc     or    a,($fc)+y
25bd: 20        clrp
25be: fe e6     dbnz  y,$25a6
25c0: 61        tcall 6
25c1: 39        and   (x),(y)
25c2: 14 7d     or    a,$7d+x
25c4: 6e f3 00  dbnz  $f3,$25c7
25c7: 12 17     clr0  $17
25c9: fc        inc   y
25ca: 1d        dec   x
25cb: fe e7     dbnz  y,$25b4
25cd: 0a f8 00  or1   c,$001f,0
25d0: ff        stop
25d1: ea a0 ee  not1  $1dd4,0
25d4: f6 e2 0f  mov   a,$0fe2+y
25d7: e4 00     mov   a,$00
25d9: 32 78     clr1  $78
25db: e6        mov   a,(x)
25dc: 61        tcall 6
25dd: 1c        asl   a
25de: 0d        push  psw
25df: 72 78     clr3  $78
25e1: f3 00 1e  bbc7  $00,$2602
25e4: 50 fc     bvc   $25e2
25e6: bb 01     inc   $01+x
25e8: e7 03     mov   a,($03+x)
25ea: fc        inc   y
25eb: 00        nop
25ec: 1c        asl   a
25ed: 21        tcall 2
25ee: 7d        mov   a,x
25ef: 7f        reti
25f0: f3 00 21  bbc7  $00,$2614
25f3: 50 fc     bvc   $25f1
25f5: 93 01 9c  bbc4  $01,$2594
25f8: d9 f3     mov   $f3+y,x
25fa: 00        nop
25fb: 21        tcall 2
25fc: 50 fc     bvc   $25fa
25fe: 93 01 9c  bbc4  $01,$259d
2601: cc f3 00  mov   $00f3,y
2604: 21        tcall 2
2605: 50 fc     bvc   $2603
2607: 93 01 1c  bbc4  $01,$2626
260a: 23 65 35  bbs1  $65,$2642
260d: f3 00 23  bbc7  $00,$2633
2610: 4e fc 6d  tclr1 $6dfc
2613: 01        tcall 0
2614: ff        stop
2615: 19        or    (x),(y)
2616: 26        and   a,(x)
2617: 2c 26 ea  rol   $ea26
261a: fa ee ea  mov   ($ea),($ee)
261d: e2 0f     set7  $0f
261f: e4 00     mov   a,$00
2621: 32 8c     clr1  $8c
2623: 46        eor   a,(x)
2624: 3c        rol   a
2625: 7f        reti
2626: 78 c6 da  cmp   $da,#$c6
2629: c6        mov   (x),a
262a: c3 ff ea  bbs6  $ff,$2617
262d: fa ee e0  mov   ($e0),($ee)
2630: e2 0e     set7  $0e
2632: e4 00     mov   a,$00
2634: 32 80     clr1  $80
2636: 39        and   (x),(y)
2637: 3c        rol   a
2638: 7f        reti
2639: 7f        reti
263a: f3 00 3c  bbc7  $00,$2679
263d: 46        eor   a,(x)
263e: fc        inc   y
263f: 37 00     and   a,($00)+y
2641: b9        sbc   (x),(y)
2642: da f3     movw  $f3,ya
2644: 00        nop
2645: 3c        rol   a
2646: 46        eor   a,(x)
2647: fc        inc   y
2648: 37 00     and   a,($00)+y
264a: b9        sbc   (x),(y)
264b: c3 f3 00  bbs6  $f3,$264e
264e: 3c        rol   a
264f: 46        eor   a,(x)
2650: fc        inc   y
2651: 37 00     and   a,($00)+y
2653: ff        stop
2654: 58 26 7e  eor   $7e,#$26
2657: 26        and   a,(x)
2658: ea 82 ee  not1  $1dd0,2
265b: e6        mov   a,(x)
265c: e2 13     set7  $13
265e: e3 0c e4  bbs7  $0c,$2645
2661: 00        nop
2662: 3c        rol   a
2663: 80        setc
2664: 43 fa 7f  bbs2  $fa,$26e6
2667: 5f f3 00  jmp   $00f3

266a: fa 13 fc  mov   ($fc),($13)
266d: cf        mul   ya
266e: ff        stop
266f: ef        sleep
2670: a0        ei
2671: 00        nop
2672: 13 b4 7d  bbc0  $b4,$26f2
2675: 5f f3 00  jmp   $00f3

2678: b4 13     sbc   a,$13+x
267a: fc        inc   y
267b: 00        nop
267c: 00        nop
267d: ff        stop
267e: ea 82 ee  not1  $1dd0,2
2681: e0        clrv
2682: e2 0f     set7  $0f
2684: e3 08 e4  bbs7  $08,$266b
2687: 00        nop
2688: 3c        rol   a
2689: 7f        reti
268a: 48 c8     eor   a,#$c8
268c: 7f        reti
268d: 7f        reti
268e: f3 00 c8  bbc7  $00,$2659
2691: 18 fc c3  or    $c3,#$fc
2694: ff        stop
2695: ef        sleep
2696: a0        ei
2697: 00        nop
2698: 18 b4 7d  or    $7d,#$b4
269b: 7f        reti
269c: f3 00 b4  bbc7  $00,$2653
269f: 10 fc     bpl   $269d
26a1: f5 ff ff  mov   a,$ffff+x
26a4: a8 26     sbc   a,#$26
26a6: d1        tcall 13
26a7: 26        and   a,(x)
26a8: ea c8 eb  not1  $1d79,0
26ab: 64 fa     cmp   a,$fa
26ad: ee        pop   y
26ae: ec e2 04  mov   y,$04e2
26b1: e3 07 e4  bbs7  $07,$2698
26b4: 00        nop
26b5: 8c 28 e6  dec   $e628
26b8: fe f7     dbnz  y,$26b1
26ba: 26        and   a,(x)
26bb: e7 19     mov   a,($19+x)
26bd: 00        nop
26be: 20        clrp
26bf: e6        mov   a,(x)
26c0: f1        tcall 15
26c1: 00        nop
26c2: 19        or    (x),(y)
26c3: 0a fc 66  or1   c,$0cdf,4
26c6: 00        nop
26c7: 61        tcall 6
26c8: 3b 19     rol   $19+x
26ca: 7d        mov   a,x
26cb: 52 e7     clr2  $e7
26cd: 0f        brk
26ce: fd        mov   y,a
26cf: 23 ff f2  bbs1  $ff,$26c4
26d2: 07 ea     or    a,($ea+x)
26d4: c8 eb     cmp   x,#$eb
26d6: 64 fa     cmp   a,$fa
26d8: ee        pop   y
26d9: b4 e2     sbc   a,$e2+x
26db: 00        nop
26dc: e3 0d e4  bbs7  $0d,$26c3
26df: 00        nop
26e0: 8c 28 e0  dec   $e028
26e3: 07 e6     or    a,($e6+x)
26e5: fe f7     dbnz  y,$26de
26e7: 26        and   a,(x)
26e8: e7 19     mov   a,($19+x)
26ea: 00        nop
26eb: 23 e6 61  bbs1  $e6,$274f
26ee: 3b 19     rol   $19+x
26f0: 7d        mov   a,x
26f1: 53 e7 0f  bbc2  $e7,$2703
26f4: fd        mov   y,a
26f5: 23 ff f1  bbs1  $ff,$26e9
26f8: 00        nop
26f9: 19        or    (x),(y)
26fa: 0a fc 66  or1   c,$0cdf,4
26fd: 00        nop
26fe: 61        tcall 6
26ff: 30 19     bmi   $271a
2701: 7d        mov   a,x
2702: 7f        reti
2703: ff        stop
2704: 08 27     or    a,#$27
2706: 2e 27 ea  cbne  $27,$26f3
2709: b4 ee     sbc   a,$ee+x
270b: fe e2     dbnz  y,$26ef
270d: 13 e3 09  bbc0  $e3,$2719
2710: e4 00     mov   a,$00
2712: 5a 7f     cmpw  ya,$7f
2714: fa 96 e3  mov   ($e3),($96)
2717: 00        nop
2718: 33 50 7d  bbc1  $50,$2798
271b: 7f        reti
271c: f3 00 50  bbc7  $00,$276f
271f: 1f fc c0  jmp   ($c0fc+x)
2722: ff        stop
2723: 33 46 cb  bbc1  $46,$26f1
2726: f3 00 50  bbc7  $00,$2779
2729: 1f fc c0  jmp   ($c0fc+x)
272c: ff        stop
272d: ff        stop
272e: ea b4 ee  not1  $1dd6,4
2731: fe e4     dbnz  y,$2717
2733: 00        nop
2734: 37 5a     and   a,($5a)+y
2736: e2 0f     set7  $0f
2738: e3 0c fa  bbs7  $0c,$2735
273b: 96 e3 00  adc   a,$00e3+y
273e: 46        eor   a,(x)
273f: 50 7d     bvc   $27be
2741: 7f        reti
2742: f3 00 50  bbc7  $00,$2795
2745: 1a fc     decw  $fc
2747: 74 ff     cmp   a,$ff+x
2749: 46        eor   a,(x)
274a: 46        eor   a,(x)
274b: cb f3     mov   $f3,y
274d: 00        nop
274e: 50 1a     bvc   $276a
2750: fc        inc   y
2751: 74 ff     cmp   a,$ff+x
2753: ff        stop
2754: 58 27 7d  eor   $7d,#$27
2757: 27 e2     and   a,($e2+x)
2759: 0e e3 0c  tset1 $0ce3
275c: ee        pop   y
275d: fe ea     dbnz  y,$2749
275f: dc        dec   y
2760: 35 14 7d  and   a,$7d14+x
2763: 7f        reti
2764: f3 00 10  bbc7  $00,$2777
2767: 17 fc     or    a,($fc)+y
2769: 20        clrp
276a: fe e6     dbnz  y,$2752
276c: 61        tcall 6
276d: 39        and   (x),(y)
276e: 14 7d     or    a,$7d+x
2770: 6e f3 00  dbnz  $f3,$2773
2773: 12 17     clr0  $17
2775: fc        inc   y
2776: 1d        dec   x
2777: fe e7     dbnz  y,$2760
2779: 0a f8 00  or1   c,$001f,0
277c: ff        stop
277d: ea a0 ee  not1  $1dd4,0
2780: fe e2     dbnz  y,$2764
2782: 0f        brk
2783: e4 00     mov   a,$00
2785: 32 78     clr1  $78
2787: 1c        asl   a
2788: 21        tcall 2
2789: 7d        mov   a,x
278a: 7f        reti
278b: f3 00 21  bbc7  $00,$27af
278e: 50 fc     bvc   $278c
2790: 93 01 9c  bbc4  $01,$272f
2793: d9 f3     mov   $f3+y,x
2795: 00        nop
2796: 21        tcall 2
2797: 50 fc     bvc   $2795
2799: 93 01 9c  bbc4  $01,$2738
279c: cc f3 00  mov   $00f3,y
279f: 21        tcall 2
27a0: 50 fc     bvc   $279e
27a2: 93 01 1c  bbc4  $01,$27c1
27a5: 23 65 35  bbs1  $65,$27dd
27a8: f3 00 23  bbc7  $00,$27ce
27ab: 4e fc 6d  tclr1 $6dfc
27ae: 01        tcall 0
27af: ff        stop
27b0: b4 27     sbc   a,$27+x
27b2: f1        tcall 15
27b3: 27 f2     and   a,($f2+x)
27b5: 08 ea     or    a,#$ea
27b7: a0        ei
27b8: e2 02     set7  $02
27ba: e3 0c e4  bbs7  $0c,$27a1
27bd: 00        nop
27be: 3c        rol   a
27bf: 5a fa     cmpw  ya,$fa
27c1: 46        eor   a,(x)
27c2: d2 00     clr6  $00
27c4: e6        mov   a,(x)
27c5: ee        pop   y
27c6: f0 ef     beq   $27b7
27c8: a0        ei
27c9: be        das   a
27ca: 61        tcall 6
27cb: 3e b4     cmp   x,$b4
27cd: 7d        mov   a,x
27ce: 71        tcall 7
27cf: f3 46 6e  bbc7  $46,$2840
27d2: 33 fc e7  bbc1  $fc,$27bc
27d5: ff        stop
27d6: ee        pop   y
27d7: f0 ef     beq   $27c8
27d9: 96 b8 33  adc   a,$33b8+y
27dc: a0        ei
27dd: f1        tcall 15
27de: f3 0a 46  bbc7  $0a,$2827
27e1: 37 fc     and   a,($fc)+y
27e3: 0e 00 f3  tset1 $f300
27e6: 00        nop
27e7: 5a 41     cmpw  ya,$41
27e9: fc        inc   y
27ea: 1c        asl   a
27eb: 00        nop
27ec: e7 02     mov   a,($02+x)
27ee: 00        nop
27ef: 00        nop
27f0: ff        stop
27f1: ea a0 e2  not1  $1c54,0
27f4: 02 e3     set0  $e3
27f6: 09 e4 00  or    ($00),($e4)
27f9: 3c        rol   a
27fa: 5a fa     cmpw  ya,$fa
27fc: 46        eor   a,(x)
27fd: d2 00     clr6  $00
27ff: e6        mov   a,(x)
2800: ee        pop   y
2801: f0 ef     beq   $27f2
2803: a0        ei
2804: be        das   a
2805: 61        tcall 6
2806: 3e b4     cmp   x,$b4
2808: 7d        mov   a,x
2809: 7b f3     ror   $f3+x
280b: 46        eor   a,(x)
280c: 6e 33 fc  dbnz  $33,$280b
280f: e7 ff     mov   a,($ff+x)
2811: ee        pop   y
2812: f0 ef     beq   $2803
2814: 96 b8 33  adc   a,$33b8+y
2817: a0        ei
2818: f9 f3     mov   x,$f3+y
281a: 0a 46 37  or1   c,$06e8,6
281d: fc        inc   y
281e: 0e 00 f3  tset1 $f300
2821: 00        nop
2822: 5a 41     cmpw  ya,$41
2824: fc        inc   y
2825: 1c        asl   a
2826: 00        nop
2827: e7 02     mov   a,($02+x)
2829: 00        nop
282a: 00        nop
282b: ff        stop
282c: 30 28     bmi   $2856
282e: 53 28 ea  bbc2  $28,$281b
2831: fa ee e6  mov   ($e6),($ee)
2834: e2 0f     set7  $0f
2836: e3 08 e4  bbs7  $08,$281d
2839: 00        nop
283a: 34 4e     and   a,$4e+x
283c: e6        mov   a,(x)
283d: 61        tcall 6
283e: 40        setp
283f: 12 7d     clr0  $7d
2841: 7d        mov   a,x
2842: 40        setp
2843: 02 bc     set0  $bc
2845: e7 0e     mov   a,($0e+x)
2847: 00        nop
2848: 00        nop
2849: 41        tcall 4
284a: 3c        rol   a
284b: 7f        reti
284c: 7d        mov   a,x
284d: 3f 28 da  call  $da28
2850: bd        mov   sp,x
2851: c3 ff ea  bbs6  $ff,$283e
2854: fa ee ea  mov   ($ea),($ee)
2857: e2 05     set7  $05
2859: e3 0b e4  bbs7  $0b,$2840
285c: 00        nop
285d: 32 80     clr1  $80
285f: ec 00 fa  mov   y,$fa00
2862: 96 d2 00  adc   a,$00d2+y
2865: e6        mov   a,(x)
2866: 61        tcall 6
2867: 41        tcall 4
2868: 14 69     or    a,$69+x
286a: 7f        reti
286b: f3 00 12  bbc7  $00,$2880
286e: 46        eor   a,(x)
286f: fc        inc   y
2870: 47 00     eor   a,($00+x)
2872: e7 0e     mov   a,($0e+x)
2874: 00        nop
2875: 00        nop
2876: 39        and   (x),(y)
2877: 3c        rol   a
2878: 7f        reti
2879: 7f        reti
287a: f3 00 3c  bbc7  $00,$28b9
287d: 46        eor   a,(x)
287e: fc        inc   y
287f: 37 00     and   a,($00)+y
2881: b9        sbc   (x),(y)
2882: da f3     movw  $f3,ya
2884: 00        nop
2885: 3c        rol   a
2886: 46        eor   a,(x)
2887: fc        inc   y
2888: 37 00     and   a,($00)+y
288a: b9        sbc   (x),(y)
288b: c3 f3 00  bbs6  $f3,$288e
288e: 3c        rol   a
288f: 46        eor   a,(x)
2890: fc        inc   y
2891: 37 00     and   a,($00)+y
2893: ff        stop
2894: 98 28 d5  adc   $d5,#$28
2897: 28 f2     and   a,#$f2
2899: ec ea 32  mov   y,$32ea
289c: ee        pop   y
289d: d6 e2 0a  mov   $0ae2+y,a
28a0: e3 09 f8  bbs7  $09,$289b
28a3: 46        eor   a,(x)
28a4: 01        tcall 0
28a5: 31        tcall 3
28a6: 05 7f 6e  or    a,$6e7f
28a9: 35 03 ee  and   a,$ee03+x
28ac: fa 82 d2  mov   ($d2),($82)
28af: 00        nop
28b0: f6 fe 09  mov   a,$09fe+y
28b3: 29 f7 61  and   ($61),($f7)
28b6: 3b 02     rol   $02+x
28b8: 7f        reti
28b9: 7f        reti
28ba: bf        mov   a,(x)+
28bb: ff        stop
28bc: ec 0c f7  mov   y,$f70c
28bf: 61        tcall 6
28c0: 3f 02 7f  call  $7f02
28c3: 6e c1 ee  dbnz  $c1,$28b4
28c6: ec 06 fa  mov   y,$fa06
28c9: 96 e0 00  adc   a,$00e0+y
28cc: e6        mov   a,(x)
28cd: fe 21     dbnz  y,$28f0
28cf: 29 e7 04  and   ($04),($e7)
28d2: fa 00 ff  mov   ($ff),($00)
28d5: ea 32 ee  not1  $1dc6,2
28d8: ec e2 0a  mov   y,$0ae2
28db: e3 0b f8  bbs7  $0b,$28d6
28de: 46        eor   a,(x)
28df: 13 fa 82  bbc0  $fa,$2864
28e2: d2 00     clr6  $00
28e4: f6 fe 09  mov   a,$09fe+y
28e7: 29 f7 61  and   ($61),($f7)
28ea: 3b 02     rol   $02+x
28ec: 7f        reti
28ed: 7f        reti
28ee: bf        mov   a,(x)+
28ef: ff        stop
28f0: ec 0c f7  mov   y,$f70c
28f3: 61        tcall 6
28f4: 3f 02 7f  call  $7f02
28f7: 7f        reti
28f8: c1        tcall 12
28f9: ff        stop
28fa: ec 06 fa  mov   y,$fa06
28fd: 96 e0 00  adc   a,$00e0+y
2900: e6        mov   a,(x)
2901: fe 21     dbnz  y,$2924
2903: 29 e7 04  and   ($04),($e7)
2906: fa 00 ff  mov   ($ff),($00)
2909: 61        tcall 6
290a: 31        tcall 3
290b: 02 7f     set0  $7f
290d: 7f        reti
290e: b5 ff b3  sbc   a,$b3ff+x
2911: ff        stop
2912: b7 ff     sbc   a,($ff)+y
2914: b5 ff b9  sbc   a,$b9ff+x
2917: ff        stop
2918: b7 ff     sbc   a,($ff)+y
291a: bb ff     inc   $ff+x
291c: b9        sbc   (x),(y)
291d: ff        stop
291e: bd        mov   sp,x
291f: ff        stop
2920: ff        stop
2921: 61        tcall 6
2922: 3b 02     rol   $02+x
2924: 7f        reti
2925: 6e bf ee  dbnz  $bf,$2916
2928: bd        mov   sp,x
2929: ee        pop   y
292a: c1        tcall 12
292b: ee        pop   y
292c: bf        mov   a,(x)+
292d: ee        pop   y
292e: c3 ee c5  bbs6  $ee,$28f6
2931: ee        pop   y
2932: c7 ee     mov   ($ee+x),a
2934: ff        stop
2935: 39        and   (x),(y)
2936: 29 70 29  and   ($29),($70)
2939: f2 0a     clr7  $0a
293b: ea 32 ee  not1  $1dc6,2
293e: d6 e2 0a  mov   $0ae2+y,a
2941: e3 02 f8  bbs7  $02,$293c
2944: 37 09     and   a,($09)+y
2946: 46        eor   a,(x)
2947: 05 7f 7f  or    a,$7f7f
294a: 42 03     set2  $03
294c: ff        stop
294d: f6 fe 9e  mov   a,$9efe+y
2950: 29 f7 61  and   ($61),($f7)
2953: 3c        rol   a
2954: 02 7f     set0  $7f
2956: 7f        reti
2957: b8 ff ec  sbc   $ec,#$ff
295a: f4 f7     mov   a,$f7+x
295c: 61        tcall 6
295d: 3c        rol   a
295e: 02 7f     set0  $7f
2960: 6e b8 ee  dbnz  $b8,$2951
2963: fa 96 df  mov   ($df),($96)
2966: 00        nop
2967: e6        mov   a,(x)
2968: fe b6     dbnz  y,$2920
296a: 29 e7 04  and   ($04),($e7)
296d: fa 00 ff  mov   ($ff),($00)
2970: ea 32 ee  not1  $1dc6,2
2973: ec e2 0a  mov   y,$0ae2
2976: e3 13 f8  bbs7  $13,$2971
2979: 37 0b     and   a,($0b)+y
297b: f6 fe 9e  mov   a,$9efe+y
297e: 29 f7 61  and   ($61),($f7)
2981: 3c        rol   a
2982: 02 7f     set0  $7f
2984: 7f        reti
2985: b8 ff ec  sbc   $ec,#$ff
2988: f4 f7     mov   a,$f7+x
298a: 61        tcall 6
298b: 3c        rol   a
298c: 02 7f     set0  $7f
298e: 7f        reti
298f: b8 ff fa  sbc   $fa,#$ff
2992: 96 df 00  adc   a,$00df+y
2995: e6        mov   a,(x)
2996: fe b6     dbnz  y,$294e
2998: 29 e7 04  and   ($04),($e7)
299b: fa 00 ff  mov   ($ff),($00)
299e: 61        tcall 6
299f: 46        eor   a,(x)
29a0: 02 7f     set0  $7f
29a2: 7f        reti
29a3: c2 ff     set6  $ff
29a5: c4 ff     mov   $ff,a
29a7: c0        di
29a8: ff        stop
29a9: c2 ff     set6  $ff
29ab: be        das   a
29ac: ff        stop
29ad: c0        di
29ae: ff        stop
29af: bc        inc   a
29b0: ff        stop
29b1: be        das   a
29b2: ff        stop
29b3: ba ff     movw  ya,$ff
29b5: ff        stop
29b6: 61        tcall 6
29b7: 42 02     set2  $02
29b9: 7f        reti
29ba: 6e be ee  dbnz  $be,$29ab
29bd: c0        di
29be: ee        pop   y
29bf: bc        inc   a
29c0: ee        pop   y
29c1: be        das   a
29c2: ee        pop   y
29c3: ba ee     movw  ya,$ee
29c5: bc        inc   a
29c6: ee        pop   y
29c7: b8 ee ff  sbc   $ff,#$ee
29ca: ce        pop   x
29cb: 29 fd 29  and   ($29),($fd)
29ce: f2 08     clr7  $08
29d0: ea 96 ee  not1  $1dd2,6
29d3: da e2     movw  $e2,ya
29d5: 0e e3 0c  tset1 $0ce3
29d8: e4 00     mov   a,$00
29da: 1e 5a f1  cmp   x,$f15a
29dd: 00        nop
29de: 12 f6     clr0  $f6
29e0: fc        inc   y
29e1: 72 ff     clr3  $ff
29e3: 2d        push  a
29e4: 12 7f     clr0  $7f
29e6: 64 e6     cmp   a,$e6
29e8: fe 1f     dbnz  y,$2a09
29ea: 2a e7 0f  or1   c,!($01fc,7)
29ed: fe 1e     dbnz  y,$2a0d
29ef: e8 e6     mov   a,#$e6
29f1: fe 2c     dbnz  y,$2a1f
29f3: 2a e7 06  or1   c,!($00dc,7)
29f6: ff        stop
29f7: 14 e9     or    a,$e9+x
29f9: 04 f4     or    a,$f4
29fb: 28 ff     and   a,#$ff
29fd: ea 96 ee  not1  $1dd2,6
2a00: f4 e2     mov   a,$e2+x
2a02: 0e e3 08  tset1 $08e3
2a05: e4 00     mov   a,$00
2a07: 1e 5a e6  cmp   x,$e65a
2a0a: fe 1f     dbnz  y,$2a2b
2a0c: 2a e7 0f  or1   c,!($01fc,7)
2a0f: fe 1e     dbnz  y,$2a2f
2a11: e8 e6     mov   a,#$e6
2a13: fe 2c     dbnz  y,$2a41
2a15: 2a e7 06  or1   c,!($00dc,7)
2a18: ff        stop
2a19: 14 e9     or    a,$e9+x
2a1b: 04 f4     or    a,$f4
2a1d: 28 ff     and   a,#$ff
2a1f: f1        tcall 15
2a20: 00        nop
2a21: 0b f6     asl   $f6
2a23: fc        inc   y
2a24: 18 ff 61  or    $61,#$ff
2a27: 2d        push  a
2a28: 0b 7f     asl   $7f
2a2a: 78 ff f1  cmp   $f1,#$ff
2a2d: 00        nop
2a2e: 0a f6 fc  or1   c,$1f9e,6
2a31: 00        nop
2a32: ff        stop
2a33: 61        tcall 6
2a34: 37 0a     and   a,($0a)+y
2a36: 7f        reti
2a37: 55 ff 3d  eor   a,$3dff+x
2a3a: 2a 58 2a  or1   c,!($054b,0)
2a3d: ea 19 eb  not1  $1d63,1
2a40: ff        stop
2a41: 32 ee     clr1  $ee
2a43: d4 e2     mov   $e2+x,a
2a45: 0e e3 09  tset1 $09e3
2a48: f8 82     mov   x,$82
2a4a: 02 fa     set0  $fa
2a4c: 64 da     cmp   a,$da
2a4e: 00        nop
2a4f: e6        mov   a,(x)
2a50: fe 7e     dbnz  y,$2ad0
2a52: 2a e7 09  or1   c,!($013c,7)
2a55: ff        stop
2a56: 28 ff     and   a,#$ff
2a58: f2 0a     clr7  $0a
2a5a: ea 19 eb  not1  $1d63,1
2a5d: ff        stop
2a5e: 32 ee     clr1  $ee
2a60: dc        dec   y
2a61: e2 0a     set7  $0a
2a63: e3 0b f8  bbs7  $0b,$2a5e
2a66: 82 12     set4  $12
2a68: fa 64 da  mov   ($da),($64)
2a6b: 00        nop
2a6c: 3c        rol   a
2a6d: 02 7f     set0  $7f
2a6f: 7f        reti
2a70: 38 01 ff  and   $ff,#$01
2a73: be        das   a
2a74: ff        stop
2a75: e6        mov   a,(x)
2a76: fe 7e     dbnz  y,$2af6
2a78: 2a e7 09  or1   c,!($013c,7)
2a7b: ff        stop
2a7c: 28 ff     and   a,#$ff
2a7e: 61        tcall 6
2a7f: 3c        rol   a
2a80: 01        tcall 0
2a81: 7f        reti
2a82: 7f        reti
2a83: b8 ff be  sbc   $be,#$ff
2a86: ff        stop
2a87: ba ff     movw  ya,$ff
2a89: c0        di
2a8a: ff        stop
2a8b: bc        inc   a
2a8c: ff        stop
2a8d: c2 ff     set6  $ff
2a8f: be        das   a
2a90: ff        stop
2a91: c4 ff     mov   $ff,a
2a93: c0        di
2a94: ff        stop
2a95: c6        mov   (x),a
2a96: ff        stop
2a97: c2 ff     set6  $ff
2a99: c8 ff     cmp   x,#$ff
2a9b: c4 ff     mov   $ff,a
2a9d: ff        stop
2a9e: a2 2a     set5  $2a
2aa0: b0 2a     bcs   $2acc
2aa2: ea 64 e2  not1  $1c4c,4
2aa5: 0f        brk
2aa6: ee        pop   y
2aa7: e2 f2     set7  $f2
2aa9: 00        nop
2aaa: e3 0d fe  bbs7  $0d,$2aab
2aad: bc        inc   a
2aae: 2a ff ea  or1   c,!($1d5f,7)
2ab1: 64 e2     cmp   a,$e2
2ab3: 03 ee fe  bbs0  $ee,$2ab4
2ab6: e3 07 fe  bbs7  $07,$2ab7
2ab9: bc        inc   a
2aba: 2a ff e4  or1   c,!($1c9f,7)
2abd: 00        nop
2abe: 32 50     clr1  $50
2ac0: e5 fa 11  mov   a,$11fa
2ac3: 00        nop
2ac4: fa 3c e0  mov   ($e0),($3c)
2ac7: 00        nop
2ac8: 16 91 7d  or    a,$7d91+y
2acb: 7f        reti
2acc: ff        stop
2acd: d1        tcall 13
2ace: 2a ea 2a  or1   c,!($055d,2)
2ad1: ea b4 ee  not1  $1dd6,4
2ad4: e8 e2     mov   a,#$e2
2ad6: 0f        brk
2ad7: e3 0b fe  bbs7  $0b,$2ad8
2ada: 04 2b     or    a,$2b
2adc: e6        mov   a,(x)
2add: fe 23     dbnz  y,$2b02
2adf: 2b e7     rol   $e7
2ae1: 06        or    a,(x)
2ae2: 00        nop
2ae3: e2 c8     set7  $c8
2ae5: ff        stop
2ae6: 43 07 ff  bbs2  $07,$2ae8
2ae9: ff        stop
2aea: ea b4 ee  not1  $1dd6,4
2aed: c6        mov   (x),a
2aee: ec f4 e2  mov   y,$e2f4
2af1: 0f        brk
2af2: e3 06 fe  bbs7  $06,$2af3
2af5: 04 2b     or    a,$2b
2af7: 48 0f     eor   a,#$0f
2af9: 7f        reti
2afa: 7f        reti
2afb: e6        mov   a,(x)
2afc: fe 23     dbnz  y,$2b21
2afe: 2b e7     rol   $e7
2b00: 06        or    a,(x)
2b01: 00        nop
2b02: e2 ff     set7  $ff
2b04: e4 00     mov   a,$00
2b06: 28 7f     and   a,#$7f
2b08: e5 41 07  mov   a,$0741
2b0b: 00        nop
2b0c: fa 82 d2  mov   ($d2),($82)
2b0f: 00        nop
2b10: 44 17     eor   a,$17
2b12: 65 7f f3  cmp   a,$f37f
2b15: 00        nop
2b16: 0d        push  psw
2b17: 1c        asl   a
2b18: fc        inc   y
2b19: ed        notc
2b1a: fc        inc   y
2b1b: fa 96 d2  mov   ($d2),($96)
2b1e: 00        nop
2b1f: ef        sleep
2b20: ff        stop
2b21: 28 ff     and   a,#$ff
2b23: 61        tcall 6
2b24: 48 08     eor   a,#$08
2b26: 7f        reti
2b27: 7f        reti
2b28: 43 0a ff  bbs2  $0a,$2b2a
2b2b: 40        setp
2b2c: 0c ff 43  asl   $43ff
2b2f: 08 ff     or    a,#$ff
2b31: ff        stop
2b32: 36 2b 45  and   a,$452b+y
2b35: 2b ea     rol   $ea
2b37: 96 ee fe  adc   a,$feee+y
2b3a: e2 0f     set7  $0f
2b3c: e3 03 f8  bbs7  $03,$2b37
2b3f: b4 0d     sbc   a,$0d+x
2b41: fe 58     dbnz  y,$2b9b
2b43: 2b ff     rol   $ff
2b45: ea 96 ee  not1  $1dd2,6
2b48: de ec 06  cbne  $ec+x,$2b51
2b4b: e0        clrv
2b4c: 1e e2 0f  cmp   x,$0fe2
2b4f: e3 03 f8  bbs7  $03,$2b4a
2b52: e6        mov   a,(x)
2b53: 0d        push  psw
2b54: fe 58     dbnz  y,$2bae
2b56: 2b ff     rol   $ff
2b58: e4 00     mov   a,$00
2b5a: 1e 1e fa  cmp   x,$fa1e
2b5d: 35 d2 00  and   a,$00d2+x
2b60: 18 17 7f  or    $7f,#$17
2b63: 7f        reti
2b64: f3 0a 3c  bbc7  $0a,$2ba3
2b67: 43 fc b7  bbs2  $fc,$2b21
2b6a: 00        nop
2b6b: 1a 70     decw  $70
2b6d: ff        stop
2b6e: f3 0a 68  bbc7  $0a,$2bd9
2b71: 43 fc 64  bbs2  $fc,$2bd8
2b74: 00        nop
2b75: f8 78     mov   x,$78
2b77: 12 ef     clr0  $ef
2b79: 78 14 43  cmp   $43,#$14
2b7c: 69 7d 7f  cmp   ($7f),($7d)
2b7f: f3 00 55  bbc7  $00,$2bd7
2b82: 18 fc 7f  or    $7f,#$fc
2b85: ff        stop
2b86: ff        stop
2b87: 8b 2b     dec   $2b
2b89: af        mov   (x)+,a
2b8a: 2b ea     rol   $ea
2b8c: b4 ee     sbc   a,$ee+x
2b8e: ec e4 00  mov   y,$00e4
2b91: 5a 7f     cmpw  ya,$7f
2b93: e5 5a 0f  mov   a,$0f5a
2b96: 00        nop
2b97: ec fb e2  mov   y,$e2fb
2b9a: 15 3c 11  or    a,$113c+x
2b9d: 7d        mov   a,x
2b9e: 7f        reti
2b9f: e2 16     set7  $16
2ba1: 2b 37     rol   $37
2ba3: ff        stop
2ba4: e2 15     set7  $15
2ba6: 3c        rol   a
2ba7: 11        tcall 1
2ba8: ff        stop
2ba9: e2 16     set7  $16
2bab: 26        and   a,(x)
2bac: be        das   a
2bad: ff        stop
2bae: ff        stop
2baf: ea b4 ee  not1  $1dd6,4
2bb2: f6 ec 05  mov   a,$05ec+y
2bb5: e4 00     mov   a,$00
2bb7: 5a b4     cmpw  ya,$b4
2bb9: e2 02     set7  $02
2bbb: 30 11     bmi   $2bce
2bbd: 7d        mov   a,x
2bbe: 7f        reti
2bbf: f3 00 11  bbc7  $00,$2bd3
2bc2: 0c fc e2  asl   $e2fc
2bc5: fd        mov   y,a
2bc6: 24 37     and   a,$37
2bc8: ff        stop
2bc9: f3 00 37  bbc7  $00,$2c03
2bcc: 0c fc 91  asl   $91fc
2bcf: ff        stop
2bd0: 30 11     bmi   $2be3
2bd2: ff        stop
2bd3: f3 00 11  bbc7  $00,$2be7
2bd6: 0c fc e2  asl   $e2fc
2bd9: fd        mov   y,a
2bda: fa 78 d2  mov   ($d2),($78)
2bdd: 0a 24 be  or1   c,$17c4,4
2be0: 4c 7f f3  lsr   $f37f
2be3: 00        nop
2be4: be        das   a
2be5: 0c fc e0  asl   $e0fc
2be8: ff        stop
2be9: ff        stop
2bea: ee        pop   y
2beb: 2b 2e     rol   $2e
2bed: 2c ea b4  rol   $b4ea
2bf0: ee        pop   y
2bf1: fe e2     dbnz  y,$2bd5
2bf3: 0e e4 00  tset1 $00e4
2bf6: 5a 80     cmpw  ya,$80
2bf8: e0        clrv
2bf9: 46        eor   a,(x)
2bfa: e6        mov   a,(x)
2bfb: f1        tcall 15
2bfc: 00        nop
2bfd: 11        tcall 1
2bfe: dc        dec   y
2bff: fc        inc   y
2c00: e2 fd     set7  $fd
2c02: 61        tcall 6
2c03: 0c 11 7d  asl   $7d11
2c06: 7f        reti
2c07: f1        tcall 15
2c08: 00        nop
2c09: 37 ea     and   a,($ea)+y
2c0b: fc        inc   y
2c0c: 9a ff     subw  ya,$ff
2c0e: 0c 37 ff  asl   $ff37
2c11: e7 06     mov   a,($06+x)
2c13: 00        nop
2c14: 00        nop
2c15: f1        tcall 15
2c16: 00        nop
2c17: 11        tcall 1
2c18: dc        dec   y
2c19: fc        inc   y
2c1a: e2 fd     set7  $fd
2c1c: 0c 11 ff  asl   $ff11
2c1f: fa 78 d2  mov   ($d2),($78)
2c22: 0a f1 00  or1   c,$001e,1
2c25: be        das   a
2c26: e9 fc e2  mov   x,$e2fc
2c29: ff        stop
2c2a: 0c be ff  asl   $ffbe
2c2d: ff        stop
2c2e: ea b4 ee  not1  $1dd6,4
2c31: fe e4     dbnz  y,$2c17
2c33: 00        nop
2c34: 28 3c     and   a,#$3c
2c36: fa 96 d2  mov   ($d2),($96)
2c39: 1e e0 46  cmp   x,$46e0
2c3c: e6        mov   a,(x)
2c3d: e2 1d     set7  $1d
2c3f: 61        tcall 6
2c40: 19        or    (x),(y)
2c41: 48 72     eor   a,#$72
2c43: 7f        reti
2c44: e7 06     mov   a,($06+x)
2c46: 00        nop
2c47: 12 16     clr0  $16
2c49: be        das   a
2c4a: 7d        mov   a,x
2c4b: 7f        reti
2c4c: f3 3c 82  bbc7  $3c,$2bd1
2c4f: 04 fc     or    a,$fc
2c51: dd        mov   a,y
2c52: ff        stop
2c53: ff        stop
2c54: 58 2c 75  eor   $75,#$2c
2c57: 2c ea 48  rol   $48ea
2c5a: eb ff     mov   y,$ff
2c5c: 64 ee     cmp   a,$ee
2c5e: e4 e2     mov   a,$e2
2c60: 0a e3 0f  or1   c,$01fc,3
2c63: f8 c8     mov   x,$c8
2c65: 07 fa     or    a,($fa+x)
2c67: 82 da     set4  $da
2c69: 00        nop
2c6a: ec 0d e6  mov   y,$e60d
2c6d: fe 9d     dbnz  y,$2c0c
2c6f: 2c e7 19  rol   $19e7
2c72: 00        nop
2c73: ec ff ea  mov   y,$eaff
2c76: 48 eb     eor   a,#$eb
2c78: ff        stop
2c79: 64 ee     cmp   a,$ee
2c7b: e0        clrv
2c7c: e2 0a     set7  $0a
2c7e: e3 07 f8  bbs7  $07,$2c79
2c81: c8 0f     cmp   x,#$0f
2c83: f2 0a     clr7  $0a
2c85: ec 0d fa  mov   y,$fa0d
2c88: 78 da 00  cmp   $00,#$da
2c8b: 46        eor   a,(x)
2c8c: 04 7f     or    a,$7f
2c8e: 7f        reti
2c8f: 46        eor   a,(x)
2c90: 02 f8     set0  $f8
2c92: c2 ff     set6  $ff
2c94: e6        mov   a,(x)
2c95: fe 9d     dbnz  y,$2c34
2c97: 2c e7 19  rol   $19e7
2c9a: 00        nop
2c9b: ec ff 61  mov   y,$61ff
2c9e: 46        eor   a,(x)
2c9f: 03 7f 7f  bbs0  $7f,$2d21
2ca2: 46        eor   a,(x)
2ca3: 02 f8     set0  $f8
2ca5: 42 03     set2  $03
2ca7: ff        stop
2ca8: 42 02     set2  $02
2caa: f8 3f     mov   x,$3f
2cac: 03 ff 3f  bbs0  $ff,$2cee
2caf: 02 f8     set0  $f8
2cb1: 3c        rol   a
2cb2: 03 ff 3c  bbs0  $ff,$2cf1
2cb5: 02 f8     set0  $f8
2cb7: ff        stop
2cb8: bc        inc   a
2cb9: 2c d2 2c  rol   $2cd2
2cbc: ea ff ee  not1  $1ddf,7
2cbf: dc        dec   y
2cc0: e2 09     set7  $09
2cc2: e3 07 fe  bbs7  $07,$2cc3
2cc5: ee        pop   y
2cc6: 2c 38 78  rol   $7838
2cc9: c6        mov   (x),a
2cca: f3 00 8c  bbc7  $00,$2c59
2ccd: 19        or    (x),(y)
2cce: fc        inc   y
2ccf: c8 ff     cmp   x,#$ff
2cd1: ff        stop
2cd2: ea ff ee  not1  $1ddf,7
2cd5: be        das   a
2cd6: e0        clrv
2cd7: 04 f2     or    a,$f2
2cd9: 04 e2     or    a,$e2
2cdb: 09 ec fb  or    ($fb),($ec)
2cde: e3 0d fe  bbs7  $0d,$2cdf
2ce1: ee        pop   y
2ce2: 2c 38 78  rol   $7838
2ce5: c6        mov   (x),a
2ce6: f3 00 96  bbc7  $00,$2c7f
2ce9: 19        or    (x),(y)
2cea: fc        inc   y
2ceb: cc ff ff  mov   $ffff,y
2cee: e5 64 00  mov   a,$0064
2cf1: ff        stop
2cf2: e4 00     mov   a,$00
2cf4: 32 7f     clr1  $7f
2cf6: 3f aa 7f  call  $7faa
2cf9: 7f        reti
2cfa: f3 00 aa  bbc7  $00,$2ca7
2cfd: 1e fc cf  cmp   x,$cffc
2d00: ff        stop
2d01: ff        stop
2d02: 06        or    a,(x)
2d03: 2d        push  a
2d04: 36 2d e2  and   a,$e22d+y
2d07: 0e e3 0a  tset1 $0ae3
2d0a: ea ff ee  not1  $1ddf,7
2d0d: ae        pop   a
2d0e: f2 0a     clr7  $0a
2d10: fa 82 da  mov   ($da),($82)
2d13: 00        nop
2d14: 30 1e     bmi   $2d34
2d16: 7f        reti
2d17: 64 f3     cmp   a,$f3
2d19: 00        nop
2d1a: 0d        push  psw
2d1b: 42 fc     set2  $fc
2d1d: 62 01     set3  $01
2d1f: fe 64     dbnz  y,$2d85
2d21: 2d        push  a
2d22: b0 7d     bcs   $2da1
2d24: 4b f3     lsr   $f3
2d26: 00        nop
2d27: 14 42     or    a,$42+x
2d29: fc        inc   y
2d2a: e6        mov   a,(x)
2d2b: 00        nop
2d2c: b0 b2     bcs   $2ce0
2d2e: f3 00 14  bbc7  $00,$2d45
2d31: 42 fc     set2  $fc
2d33: e6        mov   a,(x)
2d34: 00        nop
2d35: ff        stop
2d36: e2 0e     set7  $0e
2d38: e3 0a ea  bbs7  $0a,$2d25
2d3b: ff        stop
2d3c: ee        pop   y
2d3d: d6 fa 82  mov   $82fa+y,a
2d40: da 00     movw  $00,ya
2d42: 30 12     bmi   $2d56
2d44: 7f        reti
2d45: 64 f3     cmp   a,$f3
2d47: 00        nop
2d48: 0d        push  psw
2d49: 42 fc     set2  $fc
2d4b: 62 01     set3  $01
2d4d: fe 64     dbnz  y,$2db3
2d4f: 2d        push  a
2d50: b0 7d     bcs   $2dcf
2d52: 3c        rol   a
2d53: f3 00 14  bbc7  $00,$2d6a
2d56: 42 fc     set2  $fc
2d58: e6        mov   a,(x)
2d59: 00        nop
2d5a: b0 a8     bcs   $2d04
2d5c: f3 00 14  bbc7  $00,$2d73
2d5f: 42 fc     set2  $fc
2d61: e6        mov   a,(x)
2d62: 00        nop
2d63: ff        stop
2d64: 30 32     bmi   $2d98
2d66: f8 f3     mov   x,$f3
2d68: 00        nop
2d69: 14 42     or    a,$42+x
2d6b: fc        inc   y
2d6c: e6        mov   a,(x)
2d6d: 00        nop
2d6e: ff        stop
2d6f: 73 2d 88  bbc3  $2d,$2cfa
2d72: 2d        push  a
2d73: ea fa ee  not1  $1ddf,2
2d76: ce        pop   x
2d77: e2 0c     set7  $0c
2d79: e3 0c fa  bbs7  $0c,$2d76
2d7c: 82 d2     set4  $d2
2d7e: 00        nop
2d7f: e6        mov   a,(x)
2d80: fe a6     dbnz  y,$2d28
2d82: 2d        push  a
2d83: e7 05     mov   a,($05+x)
2d85: ee        pop   y
2d86: 00        nop
2d87: ff        stop
2d88: ea fa ee  not1  $1ddf,2
2d8b: c6        mov   (x),a
2d8c: e2 0c     set7  $0c
2d8e: e3 07 fa  bbs7  $07,$2d8b
2d91: 82 d2     set4  $d2
2d93: 00        nop
2d94: f2 06     clr7  $06
2d96: 35 08 7f  and   a,$7f08+x
2d99: 78 39 04  cmp   $04,#$39
2d9c: f8 e6     mov   x,$e6
2d9e: fe a6     dbnz  y,$2d46
2da0: 2d        push  a
2da1: e7 05     mov   a,($05+x)
2da3: ee        pop   y
2da4: 00        nop
2da5: ff        stop
2da6: 61        tcall 6
2da7: 35 08 7f  and   a,$7f08+x
2daa: 78 b9 f8  cmp   $f8,#$b9
2dad: bc        inc   a
2dae: f8 c1     mov   x,$c1
2db0: f8 c5     mov   x,$c5
2db2: f8 c8     mov   x,$c8
2db4: f8 ff     mov   x,$ff
2db6: ba 2d     movw  ya,$2d
2db8: e2 2d     set7  $2d
2dba: ea 23 eb  not1  $1d64,3
2dbd: 32 5a     clr1  $5a
2dbf: ee        pop   y
2dc0: ce        pop   x
2dc1: e2 06     set7  $06
2dc3: e3 10 f8  bbs7  $10,$2dbe
2dc6: ff        stop
2dc7: 06        or    a,(x)
2dc8: e6        mov   a,(x)
2dc9: fe 13     dbnz  y,$2dde
2dcb: 2e e7 0a  cbne  $e7,$2dd8
2dce: 00        nop
2dcf: 14 ec     or    a,$ec+x
2dd1: 06        or    a,(x)
2dd2: eb ff     mov   y,$ff
2dd4: 82 fa     set4  $fa
2dd6: 96 dd 00  adc   a,$00dd+y
2dd9: e6        mov   a,(x)
2dda: fe 13     dbnz  y,$2def
2ddc: 2e e7 23  cbne  $e7,$2e02
2ddf: ff        stop
2de0: 1e ff f2  cmp   x,$f2ff
2de3: 0a ea 23  or1   c,$047d,2
2de6: eb 32     mov   y,$32
2de8: 5a ee     cmpw  ya,$ee
2dea: ba e2     movw  ya,$e2
2dec: 06        or    a,(x)
2ded: e3 06 f8  bbs7  $06,$2de8
2df0: ff        stop
2df1: 10 27     bpl   $2e1a
2df3: 03 7f 7f  bbs0  $7f,$2e75
2df6: 29 01 ff  and   ($ff),($01)
2df9: e6        mov   a,(x)
2dfa: fe 13     dbnz  y,$2e0f
2dfc: 2e e7 0a  cbne  $e7,$2e09
2dff: 00        nop
2e00: 14 ec     or    a,$ec+x
2e02: 06        or    a,(x)
2e03: eb ff     mov   y,$ff
2e05: 82 fa     set4  $fa
2e07: 96 dd 00  adc   a,$00dd+y
2e0a: e6        mov   a,(x)
2e0b: fe 13     dbnz  y,$2e20
2e0d: 2e e7 23  cbne  $e7,$2e33
2e10: ff        stop
2e11: 1e ff 61  cmp   x,$61ff
2e14: 29 01 7f  and   ($7f),($01)
2e17: 7f        reti
2e18: ab ff     inc   $ff
2e1a: ad ff     cmp   y,#$ff
2e1c: af        mov   (x)+,a
2e1d: ff        stop
2e1e: b1        tcall 11
2e1f: ff        stop
2e20: b3 ff b5  bbc5  $ff,$2dd8
2e23: ff        stop
2e24: ff        stop
2e25: 29 2e 51  and   ($51),($2e)
2e28: 2e ea 23  cbne  $ea,$2e4e
2e2b: eb e6     mov   y,$e6
2e2d: 78 ee dc  cmp   $dc,#$ee
2e30: e2 0e     set7  $0e
2e32: e3 10 f8  bbs7  $10,$2e2d
2e35: ff        stop
2e36: 06        or    a,(x)
2e37: e4 00     mov   a,$00
2e39: 5a 08     cmpw  ya,$08
2e3b: e6        mov   a,(x)
2e3c: fe 81     dbnz  y,$2dbf
2e3e: 2e e7 0c  cbne  $e7,$2e4d
2e41: 00        nop
2e42: 14 eb     or    a,$eb+x
2e44: ff        stop
2e45: ff        stop
2e46: ec 07 e6  mov   y,$e607
2e49: fe 81     dbnz  y,$2dcc
2e4b: 2e e7 1e  cbne  $e7,$2e6c
2e4e: fd        mov   y,a
2e4f: 1e ff f2  cmp   x,$f2ff
2e52: 05 ea 23  or    a,$23ea
2e55: eb e6     mov   y,$e6
2e57: 78 ee d2  cmp   $d2,#$ee
2e5a: e2 0e     set7  $0e
2e5c: e3 05 f8  bbs7  $05,$2e57
2e5f: ff        stop
2e60: 0f        brk
2e61: e4 00     mov   a,$00
2e63: 5a 08     cmpw  ya,$08
2e65: 2c 05 7f  rol   $7f05
2e68: 7f        reti
2e69: ae        pop   a
2e6a: ff        stop
2e6b: e6        mov   a,(x)
2e6c: fe 81     dbnz  y,$2def
2e6e: 2e e7 0c  cbne  $e7,$2e7d
2e71: 00        nop
2e72: 14 eb     or    a,$eb+x
2e74: ff        stop
2e75: ff        stop
2e76: ec 07 e6  mov   y,$e607
2e79: fe 81     dbnz  y,$2dfc
2e7b: 2e e7 1e  cbne  $e7,$2e9c
2e7e: fd        mov   y,a
2e7f: 1e ff 61  cmp   x,$61ff
2e82: 2c 03 7f  rol   $7f03
2e85: 7f        reti
2e86: ae        pop   a
2e87: ff        stop
2e88: b0 ff     bcs   $2e89
2e8a: b2 ff     clr5  $ff
2e8c: b4 ff     sbc   a,$ff+x
2e8e: b6 ff b8  sbc   a,$b8ff+y
2e91: ff        stop
2e92: ff        stop
2e93: 97 2e     adc   a,($2e)+y
2e95: d6 2e ea  mov   $ea2e+y,a
2e98: ff        stop
2e99: ee        pop   y
2e9a: e2 e2     set7  $e2
2e9c: 0f        brk
2e9d: e4 00     mov   a,$00
2e9f: 64 3c     cmp   a,$3c
2ea1: fa 8c e1  mov   ($e1),($8c)
2ea4: 00        nop
2ea5: ec 00 26  mov   y,$2600
2ea8: 3c        rol   a
2ea9: 3f 7f f3  call  $f37f
2eac: 00        nop
2ead: 32 5b     clr1  $5b
2eaf: fc        inc   y
2eb0: 0f        brk
2eb1: 01        tcall 0
2eb2: fa 9d 3c  mov   ($3c),($9d)
2eb5: 00        nop
2eb6: 26        and   a,(x)
2eb7: 32 65     clr1  $65
2eb9: 55 f3 00  eor   a,$00f3+x
2ebc: 2d        push  a
2ebd: 5b fc     lsr   $fc+x
2ebf: 2d        push  a
2ec0: 01        tcall 0
2ec1: 26        and   a,(x)
2ec2: 2d        push  a
2ec3: 72 3c     clr3  $3c
2ec5: f3 00 28  bbc7  $00,$2ef0
2ec8: 5b fc     lsr   $fc+x
2eca: 53 01 a6  bbc2  $01,$2e73
2ecd: b2 f3     clr5  $f3
2ecf: 00        nop
2ed0: 28 5b     and   a,#$5b
2ed2: fc        inc   y
2ed3: 53 01 ff  bbc2  $01,$2ed5
2ed6: ea fa ee  not1  $1ddf,2
2ed9: 96 e2 05  adc   a,$05e2+y
2edc: 55 3c 45  eor   a,$453c+x
2edf: 7f        reti
2ee0: f3 00 3c  bbc7  $00,$2f1f
2ee3: 3e fc     cmp   x,$fc
2ee5: 9e        div   ya,x
2ee6: ff        stop
2ee7: fa 9d 3c  mov   ($3c),($9d)
2eea: 00        nop
2eeb: 55 32 5f  eor   a,$5f32+x
2eee: 55 f3 00  eor   a,$00f3+x
2ef1: 32 3e     clr1  $3e
2ef3: fc        inc   y
2ef4: 8b ff     dec   $ff
2ef6: 55 2d bc  eor   a,$bc2d+x
2ef9: f3 00 2d  bbc7  $00,$2f29
2efc: 3e fc     cmp   x,$fc
2efe: 7e ff     cmp   y,$ff
2f00: fa 9d 1e  mov   ($1e),($9d)
2f03: 00        nop
2f04: d5 bc f3  mov   $f3bc+x,a
2f07: 00        nop
2f08: 2d        push  a
2f09: 3e fc     cmp   x,$fc
2f0b: 7e ff     cmp   y,$ff
2f0d: ff        stop
2f0e: 12 2f     clr0  $2f
2f10: 21        tcall 2
2f11: 2f ee     bra   $2f01
2f13: f0 ea     beq   $2eff
2f15: a0        ei
2f16: e2 02     set7  $02
2f18: e3 03 f8  bbs7  $03,$2f13
2f1b: fa 12 fe  mov   ($fe),($12)
2f1e: 32 2f     clr1  $2f
2f20: ff        stop
2f21: f2 08     clr7  $08
2f23: ee        pop   y
2f24: de ea a0  cbne  $ea+x,$2ec7
2f27: e2 02     set7  $02
2f29: e3 12 f8  bbs7  $12,$2f24
2f2c: fa 03 fe  mov   ($fe),($03)
2f2f: 32 2f     clr1  $2f
2f31: ff        stop
2f32: e4 00     mov   a,$00
2f34: 23 6e f9  bbs1  $6e,$2f30
2f37: 50 fa     bvc   $2f33
2f39: 46        eor   a,(x)
2f3a: de 00 2f  cbne  $00+x,$2f6c
2f3d: fa 7d 7f  mov   ($7f),($7d)
2f40: f3 00 78  bbc7  $00,$2fbb
2f43: 35 fc 0c  and   a,$0cfc+x
2f46: 00        nop
2f47: f3 00 82  bbc7  $00,$2ecc
2f4a: 3f fc 13  call  $13fc
2f4d: 00        nop
2f4e: fa 96 e2  mov   ($e2),($96)
2f51: 00        nop
2f52: 31        tcall 3
2f53: 64 c1     cmp   a,$c1
2f55: f3 00 64  bbc7  $00,$2fbc
2f58: 3f fc 23  call  $23fc
2f5b: 00        nop
2f5c: e3 04 f8  bbs7  $04,$2f57
2f5f: fa 0f ff  mov   ($ff),($0f)
2f62: 66        cmp   a,(x)
2f63: 2f 7c     bra   $2fe1
2f65: 2f ee     bra   $2f55
2f67: f0 ef     beq   $2f58
2f69: c8 c2     cmp   x,#$c2
2f6b: ea a0 e2  not1  $1c54,0
2f6e: 02 e3     set0  $e3
2f70: 02 f8     set0  $f8
2f72: fa 0f fe  mov   ($fe),($0f)
2f75: 96 2f e6  adc   a,$e62f+y
2f78: fe a1     dbnz  y,$2f1b
2f7a: 2f ff     bra   $2f7b
2f7c: ee        pop   y
2f7d: e4 ef     mov   a,$ef
2f7f: c8 ae     cmp   x,#$ae
2f81: ea a0 e0  not1  $1c14,0
2f84: 01        tcall 0
2f85: f2 0c     clr7  $0c
2f87: e2 02     set7  $02
2f89: e3 0f f8  bbs7  $0f,$2f84
2f8c: fa 02 fe  mov   ($fe),($02)
2f8f: 96 2f e6  adc   a,$e62f+y
2f92: fe a1     dbnz  y,$2f35
2f94: 2f ff     bra   $2f95
2f96: e4 00     mov   a,$00
2f98: 28 6e     and   a,#$6e
2f9a: f9 50     mov   x,$50+y
2f9c: fa 46 de  mov   ($de),($46)
2f9f: 00        nop
2fa0: ff        stop
2fa1: 61        tcall 6
2fa2: 3e fa     cmp   x,$fa
2fa4: 7d        mov   a,x
2fa5: 7f        reti
2fa6: f3 14 3c  bbc7  $14,$2fe5
2fa9: 3d        inc   x
2faa: fc        inc   y
2fab: fc        inc   y
2fac: ff        stop
2fad: f3 00 32  bbc7  $00,$2fe2
2fb0: 39        and   (x),(y)
2fb1: fc        inc   y
2fb2: ec ff f3  mov   y,$f3ff
2fb5: 00        nop
2fb6: 6e 2b fc  dbnz  $2b,$2fb5
2fb9: e0        clrv
2fba: ff        stop
2fbb: ff        stop
2fbc: c0        di
2fbd: 2f dd     bra   $2f9c
2fbf: 2f ea     bra   $2fab
2fc1: c8 eb     cmp   x,#$eb
2fc3: ff        stop
2fc4: ff        stop
2fc5: ee        pop   y
2fc6: f0 e2     beq   $2faa
2fc8: 0b e3     asl   $e3
2fca: 12 f8     clr0  $f8
2fcc: ff        stop
2fcd: 07 ec     or    a,($ec+x)
2fcf: 09 fa 96  or    ($96),($fa)
2fd2: dd        mov   a,y
2fd3: 00        nop
2fd4: e6        mov   a,(x)
2fd5: fe 0c     dbnz  y,$2fe3
2fd7: 30 e7     bmi   $2fc0
2fd9: 0f        brk
2fda: ff        stop
2fdb: ec ff ea  mov   y,$eaff
2fde: c8 eb     cmp   x,#$eb
2fe0: ff        stop
2fe1: ff        stop
2fe2: ee        pop   y
2fe3: d6 e2 0b  mov   $0be2+y,a
2fe6: e3 04 f8  bbs7  $04,$2fe1
2fe9: ff        stop
2fea: 0f        brk
2feb: f2 0a     clr7  $0a
2fed: ec 07 46  mov   y,$4607
2ff0: 07 7f     or    a,($7f+x)
2ff2: 78 46 05  cmp   $05,#$46
2ff5: f5 43 07  mov   a,$0743+x
2ff8: f8 43     mov   x,$43
2ffa: 05 f5 40  or    a,$40f5
2ffd: 06        or    a,(x)
2ffe: f8 fa     mov   x,$fa
3000: 96 dd 00  adc   a,$00dd+y
3003: e6        mov   a,(x)
3004: fe 0c     dbnz  y,$3012
3006: 30 e7     bmi   $2fef
3008: 0f        brk
3009: ff        stop
300a: ec ff 61  mov   y,$61ff
300d: 48 07     eor   a,#$07
300f: 7f        reti
3010: 78 48 05  cmp   $05,#$48
3013: f5 45 07  mov   a,$0745+x
3016: f8 45     mov   x,$45
3018: 05 f5 42  or    a,$42f5
301b: 07 f8     or    a,($f8+x)
301d: 42 05     set2  $05
301f: f5 3f 07  mov   a,$073f+x
3022: f8 3f     mov   x,$3f
3024: 05 f5 ff  or    a,$fff5
3027: 2b 30     rol   $30
3029: 35 30 ee  and   a,$ee30+x
302c: fe e2     dbnz  y,$3010
302e: 0f        brk
302f: e3 09 fe  bbs7  $09,$3030
3032: 41        tcall 4
3033: 30 ff     bmi   $3034
3035: ec 04 ee  mov   y,$ee04
3038: ee        pop   y
3039: e2 0f     set7  $0f
303b: e3 0b fe  bbs7  $0b,$303c
303e: 41        tcall 4
303f: 30 ff     bmi   $3040
3041: ea 27 e4  not1  $1c84,7
3044: 00        nop
3045: d9 1a     mov   $1a+y,x
3047: fa 00 d2  mov   ($d2),($00)
304a: 00        nop
304b: 1d        dec   x
304c: eb 7f     mov   y,$7f
304e: 7f        reti
304f: f3 00 e1  bbc7  $00,$3033
3052: 3b fc     rol   $fc+x
3054: 22 00     set1  $00
3056: fa 96 de  mov   ($de),($96)
3059: 00        nop
305a: 3b 82     rol   $82+x
305c: ff        stop
305d: ff        stop
305e: 62 30     set3  $30
3060: 8e        pop   psw
3061: 30 ea     bmi   $304d
3063: a0        ei
3064: ee        pop   y
3065: ea e2 02  not1  $005c,2
3068: fa 8c d2  mov   ($d2),($8c)
306b: 00        nop
306c: f1        tcall 15
306d: 00        nop
306e: 04 fc     or    a,$fc
3070: fc        inc   y
3071: 00        nop
3072: ff        stop
3073: 39        and   (x),(y)
3074: 09 4c 73  or    ($73),($4c)
3077: fe ba     dbnz  y,$3033
3079: 30 fa     bmi   $3075
307b: 93 96 00  bbc4  $96,$307e
307e: c7 b7     mov   ($b7+x),a
3080: fa 7f 1e  mov   ($1e),($7f)
3083: 00        nop
3084: f1        tcall 15
3085: 00        nop
3086: 0a 06 fc  or1   c,$1f80,6
3089: 99        adc   (x),(y)
308a: 00        nop
308b: c7 b7     mov   ($b7+x),a
308d: ff        stop
308e: f2 0a     clr7  $0a
3090: ea a0 ee  not1  $1dd4,0
3093: be        das   a
3094: e2 02     set7  $02
3096: e0        clrv
3097: 0e fa 78  tset1 $78fa
309a: d2 00     clr6  $00
309c: f1        tcall 15
309d: 00        nop
309e: 04 fc     or    a,$fc
30a0: fc        inc   y
30a1: 00        nop
30a2: ff        stop
30a3: 39        and   (x),(y)
30a4: 09 4c 78  or    ($78),($4c)
30a7: fe ba     dbnz  y,$3063
30a9: 30 fa     bmi   $30a5
30ab: 61        tcall 6
30ac: 00        nop
30ad: 00        nop
30ae: c7 da     mov   ($da+x),a
30b0: f1        tcall 15
30b1: 00        nop
30b2: 0a 06 fc  or1   c,$1f80,6
30b5: 99        adc   (x),(y)
30b6: 00        nop
30b7: c7 da     mov   ($da+x),a
30b9: ff        stop
30ba: f1        tcall 15
30bb: 00        nop
30bc: 0a 06 fc  or1   c,$1f80,6
30bf: 99        adc   (x),(y)
30c0: 00        nop
30c1: 47 10     eor   a,($10+x)
30c3: f8 f1     mov   x,$f1
30c5: 00        nop
30c6: 0a 06 fc  or1   c,$1f80,6
30c9: 99        adc   (x),(y)
30ca: 00        nop
30cb: ff        stop
30cc: d0 30     bne   $30fe
30ce: dd        mov   a,y
30cf: 30 e2     bmi   $30b3
30d1: 10 ea     bpl   $30bd
30d3: c8 ee     cmp   x,#$ee
30d5: fe e3     dbnz  y,$30ba
30d7: 0d        push  psw
30d8: e6        mov   a,(x)
30d9: fe ee     dbnz  y,$30c9
30db: 30 ff     bmi   $30dc
30dd: ec ff e2  mov   y,$e2ff
30e0: 10 ea     bpl   $30cc
30e2: c8 ee     cmp   x,#$ee
30e4: fe e3     dbnz  y,$30c9
30e6: 07 e0     or    a,($e0+x)
30e8: 04 e6     or    a,$e6
30ea: fe ee     dbnz  y,$30da
30ec: 30 ff     bmi   $30ed
30ee: e4 00     mov   a,$00
30f0: 23 7f 61  bbs1  $7f,$3154
30f3: 17 0f     or    a,($0f)+y
30f5: 7d        mov   a,x
30f6: 78 f3 00  cmp   $00,#$f3
30f9: 32 1a     clr1  $1a
30fb: fc        inc   y
30fc: 0f        brk
30fd: 00        nop
30fe: e4 00     mov   a,$00
3100: 00        nop
3101: 00        nop
3102: 17 5a     or    a,($5a)+y
3104: ff        stop
3105: f3 00 32  bbc7  $00,$313a
3108: 14 fc     or    a,$fc+x
310a: f1        tcall 15
310b: ff        stop
310c: 17 0f     or    a,($0f)+y
310e: c6        mov   (x),a
310f: f3 00 32  bbc7  $00,$3144
3112: 1a fc     decw  $fc
3114: 0f        brk
3115: 00        nop
3116: e4 00     mov   a,$00
3118: 00        nop
3119: 00        nop
311a: 17 5a     or    a,($5a)+y
311c: bc        inc   a
311d: f3 00 32  bbc7  $00,$3152
3120: 14 fc     or    a,$fc+x
3122: f1        tcall 15
3123: ff        stop
3124: ff        stop
3125: 29 31 45  and   ($45),($31)
3128: 31        tcall 3
3129: e2 0e     set7  $0e
312b: e3 07 ee  bbs7  $07,$311c
312e: fa ea 7f  mov   ($7f),($ea)
3131: e4 00     mov   a,$00
3133: 96 ff e5  adc   a,$e5ff+y
3136: c8 1e     cmp   x,#$1e
3138: 00        nop
3139: fa 96 df  mov   ($df),($96)
313c: 00        nop
313d: ef        sleep
313e: ff        stop
313f: a0        ei
3140: 0e b4 7f  tset1 $7fb4
3143: 7f        reti
3144: ff        stop
3145: e2 02     set7  $02
3147: e3 0d ee  bbs7  $0d,$3138
314a: fa ea 78  mov   ($78),($ea)
314d: e4 00     mov   a,$00
314f: 96 ff ef  adc   a,$efff+y
3152: ff        stop
3153: 50 e6     bvc   $313b
3155: 61        tcall 6
3156: 30 0a     bmi   $3162
3158: 7d        mov   a,x
3159: 7f        reti
315a: f3 00 0a  bbc7  $00,$3167
315d: 0c fc 67  asl   $67fc
3160: fc        inc   y
3161: 32 0f     clr1  $0f
3163: ff        stop
3164: f3 00 0f  bbc7  $00,$3176
3167: 1a fc     decw  $fc
3169: 67 fe     cmp   a,($fe+x)
316b: 2d        push  a
316c: 14 ff     or    a,$ff+x
316e: f3 00 14  bbc7  $00,$3185
3171: 18 fc f4  or    $f4,#$fc
3174: fe 1f     dbnz  y,$3195
3176: 05 ff f3  or    a,$f3ff
3179: 00        nop
317a: 05 0c fc  or    a,$fc0c
317d: 34 fc     and   a,$fc+x
317f: e7 04     mov   a,($04+x)
3181: f6 00 ff  mov   a,$ff00+y
3184: 88 31     adc   a,#$31
3186: b5 31 e2  sbc   a,$e231+x
3189: 0e e3 07  tset1 $07e3
318c: ee        pop   y
318d: fe ea     dbnz  y,$3179
318f: 78 e4 00  cmp   $00,#$e4
3192: 96 7f e5  adc   a,$e57f+y
3195: c8 1e     cmp   x,#$1e
3197: 00        nop
3198: fa 1e d2  mov   ($d2),($1e)
319b: 00        nop
319c: 13 ff 7f  bbc0  $ff,$321e
319f: 7f        reti
31a0: e1        tcall 14
31a1: ff        stop
31a2: 7f        reti
31a3: e1        tcall 14
31a4: ff        stop
31a5: 7f        reti
31a6: e1        tcall 14
31a7: ff        stop
31a8: 7f        reti
31a9: e1        tcall 14
31aa: ff        stop
31ab: 7d        mov   a,x
31ac: fa 96 d2  mov   ($d2),($96)
31af: 00        nop
31b0: e7 00     mov   a,($00+x)
31b2: 00        nop
31b3: 00        nop
31b4: ff        stop
31b5: e2 0f     set7  $0f
31b7: e3 0d ea  bbs7  $0d,$31a4
31ba: 46        eor   a,(x)
31bb: eb 96     mov   y,$96
31bd: be        das   a
31be: ee        pop   y
31bf: 82 ef     set4  $ef
31c1: 32 c8     clr1  $c8
31c3: e4 00     mov   a,$00
31c5: d2 80     clr6  $80
31c7: f6 e6 61  mov   a,$61e6+y
31ca: 24 0f     and   a,$0f
31cc: 7d        mov   a,x
31cd: 7f        reti
31ce: f3 00 0d  bbc7  $00,$31de
31d1: 06        or    a,(x)
31d2: fc        inc   y
31d3: b2 fd     clr5  $fd
31d5: e7 00     mov   a,($00+x)
31d7: 00        nop
31d8: 00        nop
31d9: ff        stop
31da: de 31 fe  cbne  $31+x,$31db
31dd: 31        tcall 3
31de: e2 0e     set7  $0e
31e0: e3 07 ee  bbs7  $07,$31d1
31e3: e6        mov   a,(x)
31e4: ea 78 e4  not1  $1c8f,0
31e7: 00        nop
31e8: 96 7f e5  adc   a,$e57f+y
31eb: c8 1e     cmp   x,#$1e
31ed: 00        nop
31ee: fa 96 d2  mov   ($d2),($96)
31f1: 00        nop
31f2: ef        sleep
31f3: 78 50 13  cmp   $13,#$50
31f6: 73 7f 7f  bbc3  $7f,$3278
31f9: fa 82 d2  mov   ($d2),($82)
31fc: 00        nop
31fd: ff        stop
31fe: e2 0f     set7  $0f
3200: e3 0d ea  bbs7  $0d,$31ed
3203: be        das   a
3204: ee        pop   y
3205: b4 ef     sbc   a,$ef+x
3207: ff        stop
3208: 50 e4     bvc   $31ee
320a: 00        nop
320b: d2 80     clr6  $80
320d: ef        sleep
320e: 78 50 e6  cmp   $e6,#$50
3211: 61        tcall 6
3212: 24 0f     and   a,$0f
3214: 7d        mov   a,x
3215: 7f        reti
3216: f3 00 0d  bbc7  $00,$3226
3219: 06        or    a,(x)
321a: fc        inc   y
321b: b2 fd     clr5  $fd
321d: e7 0c     mov   a,($0c+x)
321f: 00        nop
3220: 02 ff     set0  $ff
3222: 26        and   a,(x)
3223: 32 55     clr1  $55
3225: 32 e2     clr1  $e2
3227: 0e e3 09  tset1 $09e3
322a: ee        pop   y
322b: fe ea     dbnz  y,$3217
322d: 78 e5 6e  cmp   $6e,#$e5
3230: 1e 00 fa  cmp   x,$fa00
3233: 28 e1     and   a,#$e1
3235: 00        nop
3236: 14 8f     or    a,$8f+x
3238: 7d        mov   a,x
3239: 7d        mov   a,x
323a: f3 00 96  bbc7  $00,$31d3
323d: 1a fc     decw  $fc
323f: 0a 00 e2  or1   c,$1c40,0
3242: 0e e3 08  tset1 $08e3
3245: e5 6e 0f  mov   a,$0f6e
3248: 00        nop
3249: fe 82     dbnz  y,$31cd
324b: 32 e6     clr1  $e6
324d: fe 92     dbnz  y,$31e1
324f: 32 e7     clr1  $e7
3251: 04 ea     or    a,$ea
3253: 00        nop
3254: ff        stop
3255: e2 11     set7  $11
3257: e3 0b ee  bbs7  $0b,$3248
325a: fe ea     dbnz  y,$3246
325c: 78 e5 6e  cmp   $6e,#$e5
325f: 14 00     or    a,$00+x
3261: fa 28 d2  mov   ($d2),($28)
3264: 00        nop
3265: 13 8f 7d  bbc0  $8f,$32e5
3268: 7f        reti
3269: f3 00 96  bbc7  $00,$3202
326c: 34 fc     and   a,$fc+x
326e: 38 00 ec  and   $ec,#$00
3271: 09 e5 6e  or    ($6e),($e5)
3274: 0e 00 fe  tset1 $fe00
3277: 82 32     set4  $32
3279: e6        mov   a,(x)
327a: fe 92     dbnz  y,$320e
327c: 32 e7     clr1  $e7
327e: 04 ea     or    a,$ea
3280: 00        nop
3281: ff        stop
3282: fa 96 d2  mov   ($d2),($96)
3285: 00        nop
3286: f1        tcall 15
3287: 00        nop
3288: 0c f5 fc  asl   $fcf5
328b: 16 ff 13  or    a,$13ff+y
328e: 0c 65 71  asl   $7165
3291: ff        stop
3292: f1        tcall 15
3293: 00        nop
3294: 23 f2 fc  bbs1  $f2,$3293
3297: 9a ff     subw  ya,$ff
3299: 61        tcall 6
329a: 13 23 7d  bbc0  $23,$331a
329d: 7f        reti
329e: ff        stop
329f: af        mov   (x)+,a
32a0: 32 b9     clr1  $b9
32a2: 32 c9     clr1  $c9
32a4: 32 d9     clr1  $d9
32a6: 32 e9     clr1  $e9
32a8: 32 f9     clr1  $f9
32aa: 32 0b     clr1  $0b
32ac: 33 19 33  bbc1  $19,$32e2
32af: ea 69 e2  not1  $1c4d,1
32b2: 01        tcall 0
32b3: ee        pop   y
32b4: f8 fe     mov   x,$fe
32b6: 25 33 ff  and   a,$ff33
32b9: f2 04     clr7  $04
32bb: ea 69 e2  not1  $1c4d,1
32be: 01        tcall 0
32bf: ee        pop   y
32c0: de e3 0b  cbne  $e3+x,$32ce
32c3: e0        clrv
32c4: 06        or    a,(x)
32c5: fe 5f     dbnz  y,$3326
32c7: 33 ff f2  bbc1  $ff,$32bc
32ca: 06        or    a,(x)
32cb: ea 69 e2  not1  $1c4d,1
32ce: 01        tcall 0
32cf: ee        pop   y
32d0: dc        dec   y
32d1: e3 08 e0  bbs7  $08,$32b4
32d4: 11        tcall 1
32d5: fe 5f     dbnz  y,$3336
32d7: 33 ff f2  bbc1  $ff,$32cc
32da: 21        tcall 2
32db: ea 69 e2  not1  $1c4d,1
32de: 01        tcall 0
32df: ee        pop   y
32e0: ce        pop   x
32e1: e3 0f e0  bbs7  $0f,$32c4
32e4: 01        tcall 0
32e5: fe 5f     dbnz  y,$3346
32e7: 33 ff ec  bbc1  $ff,$32d6
32ea: 18 f2 0d  or    $0d,#$f2
32ed: ea 69 e2  not1  $1c4d,1
32f0: 0e ee b4  tset1 $b4ee
32f3: e3 0b fe  bbs7  $0b,$32f4
32f6: 5f 33 ff  jmp   $ff33

32f9: ec 18 f2  mov   y,$f218
32fc: 10 ea     bpl   $32e8
32fe: 69 e2 0e  cmp   ($0e),($e2)
3301: ee        pop   y
3302: 9a e3     subw  ya,$e3
3304: 0a e0 0c  or1   c,$019c,0
3307: fe 5f     dbnz  y,$3368
3309: 33 ff ec  bbc1  $ff,$32f8
330c: f4 ea     mov   a,$ea+x
330e: 69 e2 05  cmp   ($05),($e2)
3311: ee        pop   y
3312: d2 e3     clr6  $e3
3314: 0a fe 5f  or1   c,$0bff,6
3317: 33 ff ec  bbc1  $ff,$3306
331a: f4 ea     mov   a,$ea+x
331c: 69 e2 06  cmp   ($06),($e2)
331f: ee        pop   y
3320: b4 fe     sbc   a,$fe+x
3322: 25 33 ff  and   a,$ff33
3325: e3 08 fa  bbs7  $08,$3322
3328: 8e        pop   psw
3329: c4 00     mov   $00,a
332b: 29 0a 7d  and   ($7d),($0a)
332e: 6e aa ee  dbnz  $aa,$331f
3331: ad ee     cmp   y,#$ee
3333: ae        pop   a
3334: ee        pop   y
3335: ad ee     cmp   y,#$ee
3337: aa ee a9  mov1  c,$153d,6
333a: ee        pop   y
333b: aa ee a9  mov1  c,$153d,6
333e: ee        pop   y
333f: a7 ee     sbc   a,($ee+x)
3341: a6        sbc   a,(x)
3342: ee        pop   y
3343: a7 ee     sbc   a,($ee+x)
3345: a6        sbc   a,(x)
3346: ee        pop   y
3347: a3 ee a2  bbs5  $ee,$32ec
334a: ee        pop   y
334b: a3 ee fa  bbs5  $ee,$3348
334e: 8e        pop   psw
334f: ba 00     movw  ya,$00
3351: a2 7f     set5  $7f
3353: 6e fa 96  dbnz  $fa,$32ec
3356: e4 00     mov   a,$00
3358: 22 32     set1  $32
335a: 7d        mov   a,x
335b: 6e e0 1e  dbnz  $e0,$337c
335e: ff        stop
335f: fa 8e c4  mov   ($c4),($8e)
3362: 00        nop
3363: 29 0a 7d  and   ($7d),($0a)
3366: 6e aa ee  dbnz  $aa,$3357
3369: ad ee     cmp   y,#$ee
336b: ae        pop   a
336c: ee        pop   y
336d: ad ee     cmp   y,#$ee
336f: aa ee a9  mov1  c,$153d,6
3372: ee        pop   y
3373: aa ee a9  mov1  c,$153d,6
3376: ee        pop   y
3377: a7 ee     sbc   a,($ee+x)
3379: a6        sbc   a,(x)
337a: ee        pop   y
337b: a7 ee     sbc   a,($ee+x)
337d: a6        sbc   a,(x)
337e: ee        pop   y
337f: a3 ee a2  bbs5  $ee,$3324
3382: ee        pop   y
3383: a3 ee fa  bbs5  $ee,$3380
3386: 8e        pop   psw
3387: ba 00     movw  ya,$00
3389: a2 7f     set5  $7f
338b: 6e fa 96  dbnz  $fa,$3324
338e: e4 00     mov   a,$00
3390: 22 32     set1  $32
3392: 7d        mov   a,x
3393: 6e e0 1e  dbnz  $e0,$33b4
3396: ff        stop
