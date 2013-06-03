0300: 42 00     set2  $00
0302: 46        eor   a,(x)
0303: 00        nop
0304: 4b 00     lsr   $00
0306: 4f 00     pcall $00
0308: 54 00     eor   a,$00+x
030a: 59        eor   (x),(y)
030b: 00        nop
030c: 5e 00 64  cmp   y,$6400
030f: 00        nop
0310: 6a 00 70  and1  c,!($0e00,0)
0313: 00        nop
0314: 77 00     cmp   a,($00)+y
0316: 7e 00     cmp   y,$00
0318: 85 00 8d  adc   a,$8d00
031b: 00        nop
031c: 96 00 9f  adc   a,$9f00+y
031f: 00        nop
0320: a8 00     sbc   a,#$00
0322: b2 00     clr5  $00
0324: bd        mov   sp,x
0325: 00        nop
0326: c8 00     cmp   x,#$00
0328: d4 00     mov   $00+x,a
032a: e1        tcall 14
032b: 00        nop
032c: ee        pop   y
032d: 00        nop
032e: fc        inc   y
032f: 00        nop
0330: 0b 01     asl   $01
0332: 1b 01     asl   $01+x
0334: 2c 01 3e  rol   $3e01
0337: 01        tcall 0
0338: 51        tcall 5
0339: 01        tcall 0
033a: 65 01 7a  cmp   a,$7a01
033d: 01        tcall 0
033e: 91        tcall 9
033f: 01        tcall 0
0340: a9 01 c2  sbc   ($c2),($01)
0343: 01        tcall 0
0344: dd        mov   a,y
0345: 01        tcall 0
0346: f9 01     mov   x,$01+y
0348: 17 02     or    a,($02)+y
034a: 37 02     and   a,($02)+y
034c: 59        eor   (x),(y)
034d: 02 7d     set0  $7d
034f: 02 a3     set0  $a3
0351: 02 cb     set0  $cb
0353: 02 f5     set0  $f5
0355: 02 22     set0  $22
0357: 03 52 03  bbs0  $52,$035d
035a: 85 03 ba  adc   a,$ba03
035d: 03 f3 03  bbs0  $f3,$0363
0360: 2f 04     bra   $0366
0362: 6f        ret

0363: 04 b2     or    a,$b2
0365: 04 fa     or    a,$fa
0367: 04 46     or    a,$46
0369: 05 96 05  or    a,$0596
036c: eb 05     mov   y,$05
036e: 45 06 a5  eor   a,$a506
0371: 06        or    a,(x)
0372: 0a 07 75  or1   c,$0ea0,7
0375: 07 e6     or    a,($e6+x)
0377: 07 5f     or    a,($5f+x)
0379: 08 de     or    a,#$de
037b: 08 65     or    a,#$65
037d: 09 f4 09  or    ($09),($f4)
0380: 8c 0a 2c  dec   $2c0a
0383: 0b d6     asl   $d6
0385: 0b 8b     asl   $8b
0387: 0c 4a 0d  asl   $0d4a
038a: 14 0e     or    a,$0e+x
038c: ea 0e cd  not1  $19a1,6
038f: 0f        brk
0390: be        das   a
0391: 10 bd     bpl   $0350
0393: 11        tcall 1
0394: cb 12     mov   $12,y
0396: e9 13 18  mov   x,$1813
0399: 15 59 16  or    a,$1659+x
039c: ad 17     cmp   y,#$17
039e: 16 19 94  or    a,$9419+y
03a1: 1a 28     decw  $28
03a3: 1c        asl   a
03a4: d5 1d 9b  mov   $9b1d+x,a
03a7: 1f 7c 21  jmp   ($217c+x)

03aa: 7a 23     addw  ya,$23
03ac: 96 25 d2  adc   a,$d225+y
03af: 27 30     and   a,($30+x)
03b1: 2a b2 2c  or1   c,!($0596,2)
03b4: 5a 2f     cmpw  ya,$2f
03b6: 2c 32 28  rol   $2832
03b9: 35 50 38  and   a,$3850+x
03bc: ac 3b 36  inc   $363b
03bf: 3f

03c0: 0b 00
03c2: 0c 00 0d  asl   $0d00
03c5: 00        nop
03c6: 0e 00 0e  tset1 $0e00
03c9: 00        nop
03ca: 0f        brk
03cb: 00        nop
03cc: 10 00     bpl   $03ce
03ce: 10 00     bpl   $03d0
03d0: 12 00     clr0  $00
03d2: 13 00 14  bbc0  $00,$03e9
03d5: 00        nop
03d6: 15 00 16  or    a,$1600+x
03d9: 00        nop
03da: 18 00 19  or    $19,#$00
03dd: 00        nop
03de: 1b 00     asl   $00+x
03e0: 1c        asl   a
03e1: 00        nop
03e2: 1e 00 20  cmp   x,$2000
03e5: 00        nop
03e6: 21        tcall 2
03e7: 00        nop
03e8: 23 00 26  bbs1  $00,$0411
03eb: 00        nop
03ec: 28 00     and   a,#$00
03ee: 2a 00 2d  or1   c,!($05a0,0)
03f1: 00        nop
03f2: 2f 00     bra   $03f4
03f4: 32 00     clr1  $00
03f6: 35 00 38  and   a,$3800+x
03f9: 00        nop
03fa: 3c        rol   a
03fb: 00        nop
03fc: 3f 00 42  call  $4200
03ff: 00        nop
0400: 00        nop
0401: 61        tcall 6
0402: 48 61     eor   a,#$61
0404: 87 61     adc   a,($61+x)
0406: bd        mov   sp,x
0407: 61        tcall 6
0408: fc        inc   y
0409: 61        tcall 6
040a: 05 62 0e  or    a,$0e62
040d: 62 17     set3  $17
040f: 62 20     set3  $20
0411: 62 29     set3  $29
0413: 62 29     set3  $29
0415: 62 32     set3  $32
0417: 62 3b     set3  $3b
0419: 62 44     set3  $44
041b: 62 4d     set3  $4d
041d: 62 56     set3  $56
041f: 62 5f     set3  $5f
0421: 62 68     set3  $68
0423: 62 7a     set3  $7a
0425: 62 83     set3  $83
0427: 62 95     set3  $95
0429: 62 b0     set3  $b0
042b: 62 b9     set3  $b9
042d: 62 c2     set3  $c2
042f: 62 c2     set3  $c2
0431: 62 dd     set3  $dd
0433: 62 ef     set3  $ef
0435: 62 f8     set3  $f8
0437: 62 f8     set3  $f8
0439: 62 01     set3  $01
043b: 63 01 63  bbs3  $01,$04a1
043e: 1c        asl   a
043f: 63 25 63  bbs3  $25,$04a5
0442: 40        setp
0443: 63 7f 63  bbs3  $7f,$04a9
0446: 88 63     adc   a,#$63
0448: 88 63     adc   a,#$63
044a: a3 63 be  bbs5  $63,$040b
044d: 63 d0 63  bbs3  $d0,$04b3
0450: eb 63     mov   y,$63
0452: 06        or    a,(x)
0453: 64 21     cmp   a,$21
0455: 64 3c     cmp   a,$3c
0457: 64 45     cmp   a,$45
0459: 64 60     cmp   a,$60
045b: 64 7b     cmp   a,$7b
045d: 64 96     cmp   a,$96
045f: 64 b1     cmp   a,$b1
0461: 64 5d     cmp   a,$5d
0463: 67 5d     cmp   a,($5d+x)
0465: 67 9e     cmp   a,($9e+x)
0467: 6b 3e     ror   $3e
0469: 71        tcall 7
046a: 4d        push  x
046b: 74 4d     cmp   a,$4d+x
046d: 74 27     cmp   a,$27+x
046f: 79        cmp   (x),(y)
0470: 42 79     set2  $79
0472: 12 7c     clr0  $7c
0474: 12 7c     clr0  $7c
0476: c4 81     mov   $81,a
0478: 15 82 69  or    a,$6982+x
047b: 88 80     adc   a,#$80
047d: 89 b7 8b  adc   ($8b),($b7)
0480: e4 8b     mov   a,$8b
0482: 4e 91 69  tclr1 $6991
0485: 91        tcall 9
0486: 5d        mov   x,a
0487: 94 81     adc   a,$81+x
0489: 94 24     adc   a,$24+x
048b: 97 90     adc   a,($90)+y
048d: 97 e3     adc   a,($e3)+y
048f: 9b e3     dec   $e3+x
0491: 9b fc     dec   $fc+x
0493: a0        ei
0494: 3c        rol   a
0495: a3 4e a3  bbs5  $4e,$043b
0498: f3 b2 7c  bbc7  $b2,$0517
049b: b7 03     sbc   a,($03)+y
049d: b8 94 ba  sbc   $ba,#$94
04a0: af        mov   (x)+,a
04a1: ba 80     movw  ya,$80
04a3: bf        mov   a,(x)+
04a4: cd c0     mov   x,#$c0
04a6: 1d        dec   x
04a7: c8 41     cmp   x,#$41
04a9: c8 e1     cmp   x,#$e1
04ab: cd a9     mov   x,#$a9
04ad: d2 b3     clr6  $b3
04af: dd        mov   a,y
04b0: 63 df 78  bbs3  $df,$052b
04b3: e5 aa e6  mov   a,$e6aa
04b6: 51        tcall 5
04b7: e8 00     mov   a,#$00
04b9: f1        tcall 15
04ba: a3 f3 0f  bbs5  $f3,$04cc
04bd: f4 c3     mov   a,$c3+x
04bf: fd        mov   y,a
04c0: af        mov   (x)+,a
04c1: ba de     movw  ya,$de
04c3: be        das   a
04c4: fe bf     dbnz  y,$0485
04c6: cb c5     mov   $c5,y
04c8: fa c9 7f  mov   ($7f),($c9)
04cb: d8 f8     mov   $f8,x
04cd: e0        clrv
04ce: 02 ec     set0  $ec
04d0: b2 ed     clr5  $ed
04d2: 88 f3     adc   a,#$f3
04d4: e7 f4     mov   a,($f4+x)
04d6: 9b fe     dec   $fe+x
04d8: af        mov   (x)+,a
04d9: ba 56     movw  ya,$56
04db: bc        inc   a
04dc: 05 c5 18  or    a,$18c5
04df: c7 2a     mov   ($2a+x),a
04e1: c7 af     mov   ($af+x),a
04e3: cc 24 cd  mov   $cd24,y
04e6: 52 cf     clr2  $cf
04e8: 21        tcall 2
04e9: d0 a1     bne   $048c
04eb: d4 cf     mov   $cf+x,a
04ed: d6 0a d8  mov   $d80a+y,a
04f0: 8e        pop   psw
04f1: db 66     mov   $66+x,y
04f3: e5 af ba  mov   a,$baaf
04f6: ff        stop
04f7: c1        tcall 12
04f8: 23 c2 51  bbs1  $c2,$054c
04fb: c4 20     mov   $20,a
04fd: c5 a0 c9  mov   $c9a0,a
0500: ce        pop   x
0501: cb 6a     mov   $6a,y
0503: db 3e     mov   $3e+x,y
0505: dd        mov   a,y
0506: 14 e3     or    a,$e3+x
0508: 73 e4 1b  bbc3  $e4,$0526
050b: e8 36     mov   a,#$36
050d: e8 e6     mov   a,#$e6
050f: e9 af ba  mov   x,$baaf
0512: 57 be     eor   a,($be)+y
0514: 72 be     clr3  $be
0516: 6b c2     ror   $c2
0518: 8f c2 3f  mov   $3f,#$c2
051b: c4 5a     mov   $5a,a
051d: c4 27     mov   $27,a
051f: ca 56 ce  mov1  $19ca,6,c
0522: 9b d1     dec   $d1+x
0524: a9 d2 5d  sbc   ($5d),($d2)
0527: dc        dec   y
0528: 62 dd     set3  $dd
052a: 90 df     bcc   $050b
052c: 5f e0 df  jmp   $dfe0

052f: e4 0d     mov   a,$0d
0531: e7 4b     mov   a,($4b+x)
0533: ee        pop   y
0534: 7d        mov   a,x
0535: ef        sleep
0536: 6a f6 f6  and1  c,!($1ede,6)
0539: f7 9a     mov   a,($9a)+y
053b: fc        inc   y
053c: 77 fe     cmp   a,($fe)+y
053e: 80        setc
053f: fe af     dbnz  y,$04f0
0541: ba 57     movw  ya,$57
0543: be        das   a
0544: 72 be     clr3  $be
0546: 2f bf     bra   $0507
0548: 53 bf cf  bbc2  $bf,$051a
054b: c4 3d     mov   $3d,a
054d: c9 8c ce  mov   $ce8c,x
0550: cb ce     mov   $ce,y
0552: 31        tcall 3
0553: d5 31 d5  mov   $d531+x,a
0556: c9 dc c9  mov   $c9dc,x
0559: dc        dec   y
055a: f7 de     mov   a,($de)+y
055c: c6        mov   (x),a
055d: df        daa   a
055e: 46        eor   a,(x)
055f: e4 74     mov   a,$74
0561: e6        mov   a,(x)
0562: 45 eb af  eor   a,$afeb
0565: ba 8b     movw  ya,$8b
0567: d5 51 d6  mov   $d651+x,a
056a: f5 da 25  mov   a,$25da+x
056d: e1        tcall 14
056e: dd        mov   a,y
056f: e9 0f eb  mov   x,$eb0f
0572: 18 eb 2a  or    $2a,#$eb
0575: eb 3d     mov   y,$3d
0577: ed        notc
0578: 4f ed     pcall $ed
057a: 58 ed 61  eor   $61,#$ed
057d: ed        notc
057e: 37 f3     and   a,($f3)+y
0580: af        mov   (x)+,a
0581: ba 98     movw  ya,$98
0583: c2 98     set6  $98
0585: c2 13     set6  $13
0587: c6        mov   (x),a
0588: 13 c6 48  bbc0  $c6,$05d3
058b: cd 48     mov   x,#$48
058d: cd 34     mov   x,#$34
058f: d2 34     clr6  $34
0591: d2 b2     clr6  $b2
0593: db b2     mov   $b2+x,y
0595: db bd     mov   $bd+x,y
0597: e8 bd     mov   a,#$bd
0599: e8 84     mov   a,#$84
059b: eb fe     mov   y,$fe
059d: ec e8 f6  mov   y,$f6e8
05a0: 05 fb b5  or    a,$b5fb
05a3: fc        inc   y
05a4: af        mov   (x)+,a
05a5: ba 0e     movw  ya,$0e
05a7: f2 0e     clr7  $0e
05a9: f2 be     clr7  $be
05ab: f3 af ba  bbc7  $af,$0568
05ae: bc        inc   a
05af: c2 e3     set6  $e3
05b1: da d5     movw  $d5,ya
05b3: e2 6d     set7  $6d
05b5: ea 9c ee  not1  $1dd3,4
05b8: bc        inc   a
05b9: ef        sleep
05ba: 9e        div   ya,x
05bb: f2 9e     clr7  $9e
05bd: f2 4e     clr7  $4e
05bf: f4 69     mov   a,$69+x
05c1: f4 e5     mov   a,$e5+x
05c3: f9 af     mov   x,$af+y
05c5: ba b4     movw  ya,$b4
05c7: c4 69     mov   $69,a
05c9: d0 09     bne   $05d4
05cb: d6 d1 da  mov   $dad1+y,a
05ce: 9e        div   ya,x
05cf: e0        clrv
05d0: cd e4     mov   x,#$e4
05d2: ab e8     inc   $e8
05d4: 8f ef 00  mov   $00,#$ef
05d7: fa af ba  mov   ($ba),($af)
05da: da bf     movw  $bf,ya
05dc: 39        and   (x),(y)
05dd: c1        tcall 12
05de: c2 c5     set6  $c5
05e0: c2 c5     set6  $c5
05e2: 3d        inc   x
05e3: d2 3d     clr6  $3d
05e5: d2 c4     clr6  $c4
05e7: d2 c4     clr6  $c4
05e9: d2 a5     clr6  $a5
05eb: d3 a5 d3  bbc6  $a5,$05c1
05ee: 7b d9     ror   $d9+x
05f0: 7b d9     ror   $d9+x
05f2: a1        tcall 10
05f3: dd        mov   a,y
05f4: a1        tcall 10
05f5: dd        mov   a,y
05f6: 88 e1     adc   a,#$e1
05f8: 88 e1     adc   a,#$e1
05fa: 23 ef 23  bbs1  $ef,$0620
05fd: ef        sleep
05fe: 01        tcall 0
05ff: f3 e5 f9  bbc7  $e5,$05fb
0602: de fd af  cbne  $fd+x,$05b4
0605: ba 8a     movw  ya,$8a
0607: c1        tcall 12
0608: 79        cmp   (x),(y)
0609: d5 40 e1  mov   $e140+x,a
060c: 76 f3 69  cmp   a,$69f3+y
060f: fd        mov   y,a
0610: af        mov   (x)+,a
0611: ba 7f     movw  ya,$7f
0613: c6        mov   (x),a
0614: 7c        ror   a
0615: d2 64     clr6  $64
0617: ea

0618: 00 43  not1  $0860,0
061a: 0b 47     asl   $47
061c: 00        nop
061d: 52 4a     clr2  $4a
061f: 56 ae 06  eor   a,$06ae+y
0622: 49 0b 37  eor   ($37),($0b)
0625: 0b 21     asl   $21
0627: 0c 01 0b  asl   $0b01
062a: 11        tcall 1
062b: 10 7e     bpl   $06ab
062d: 09 27 0f  or    ($0f),($27)
0630: 17 0a     or    a,($0a)+y
0632: 41        tcall 4
0633: 0d        push  psw
0634: 81        tcall 8
0635: 06        or    a,(x)
0636: 30 0f     bmi   $0647
0638: 6b 07     ror   $07
063a: 7e 09     cmp   y,$09
063c: 13 14 7d  bbc0  $14,$06bc
063f: 10 9b     bpl   $05dc
0641: 0d        push  psw
0642: 5a 12     cmpw  ya,$12
0644: 41        tcall 4
0645: 16 ba 0c  or    a,$0cba+y
0648: dd        mov   a,y
0649: 0a ed 0f  or1   c,$01fd,5
064c: db 06     mov   $06+x,y
064e: d9 14     mov   $14+y,x
0650: 00        nop
0651: 1b db     asl   $db+x
0653: 06        or    a,(x)
0654: db 06     mov   $06+x,y
0656: db 06     mov   $06+x,y
0658: 44 1c     eor   a,$1c
065a: 09 00 e3  or    ($e3),($00)
065d: 04 4a     or    a,$4a
065f: 04 0b     or    a,$0b
0661: 04 8a     or    a,$8a
0663: 06        or    a,(x)
0664: 0e 0a 5a  tset1 $5a0a
0667: 00        nop
0668: 31        tcall 3
0669: 08 17     or    a,#$17
066b: 01        tcall 0
066c: 99        adc   (x),(y)
066d: 09 87 00  or    ($00),($87)
0670: 10 05     bpl   $0677
0672: 23 07 b2  bbs1  $07,$0627
0675: 05 7a 01  or    a,$017a
0678: d7 07     mov   ($07)+y,a
067a: d1        tcall 13
067b: 04 4c     or    a,$4c
067d: 08 bd     or    a,#$bd
067f: 00        nop
0680: bc        inc   a
0681: 07 ab     or    a,($ab+x)
0683: 00        nop
0684: 4d        push  x
0685: 01        tcall 0
0686: 64 02     cmp   a,$02
0688: 09 00 b6  or    ($b6),($00)
068b: 04 a7     or    a,$a7
068d: 01        tcall 0
068e: 35 07 12  and   a,$1207+x
0691: 00        nop
0692: 55 08 a8  eor   a,$a808+x
0695: 03 03 06  bbs0  $03,$069e
0698: 74 07     cmp   a,$07+x
069a: 09 00 de  or    ($de),($00)
069d: 03 d5 03  bbs0  $d5,$06a3
06a0: c7 02     mov   ($02+x),a
06a2: 5f 01 5a  jmp   $5a01

06a5: 00        nop
06a6: da 04     movw  $04,ya
06a8: 82 08     set4  $08
06aa: 4b 06     lsr   $06
06ac: f3 09 77  bbc7  $09,$0726
06af: 04 09     or    a,$09
06b1: 00        nop
06b2: 17 01     or    a,($01)+y
06b4: e9 07 bd  mov   x,$bd07
06b7: 00        nop
06b8: 1a 07     decw  $07
06ba: 9c        dec   a
06bb: 06        or    a,(x)
06bc: fe 04     dbnz  y,$06c2
06be: 4f 05     pcall $05
06c0: 86        adc   a,(x)
06c1: 07 24     or    a,($24+x)
06c3: 00        nop
06c4: d2 06     clr6  $06
06c6: 88 0b     adc   a,#$0b
06c8: 09 00 98  or    ($98),($00)
06cb: 07 0a     or    a,($0a+x)
06cd: 0b 1f     asl   $1f
06cf: 08 85     or    a,#$85
06d1: 0e 77 04  tset1 $0477
06d4: 77 04     cmp   a,($04)+y
06d6: 33 03 19  bbc1  $03,$06f2
06d9: 05 e9 07  or    a,$07e9
06dc: 98 07 24  adc   $24,#$07
06df: 00        nop
06e0: 98 07 cc  adc   $cc,#$07
06e3: 03 20 01  bbs0  $20,$06e7
06e6: 24 00     and   a,$00
06e8: 5e 08 24  cmp   y,$2408
06eb: 00        nop
06ec: 44 01     eor   a,$01
06ee: 88 0b     adc   a,#$0b
06f0: 41        tcall 4
06f1: 04 60     or    a,$60
06f3: 0c 61 05  asl   $0561
06f6: 1f 08 87  jmp   ($8708+x)

06f9: 09 8d 03  or    ($03),($8d)
06fc: e9 07 20  mov   x,$2007
06ff: 0a 69 0c  or1   c,$018d,1
0702: 0c 06 5b  asl   $5b06
0705: 02 f1     set0  $f1
0707: 0e f2 07  tset1 $07f2
070a: 1f 08 f3  jmp   ($f308+x)

070d: 09 f3 09  or    ($09),($f3)
0710: 1f 08 20  jmp   ($2008+x)

0713: 0a 79 08  or1   c,$010f,1
0716: a0        ei
0717: 0e f3 09  tset1 $09f3
071a: 1a 07     decw  $07
071c: 5c        lsr   a
071d: 0d        push  psw
071e: 77 04     cmp   a,($04)+y
0720: 00        nop
0721: 00        nop
0722: 8f e0 14  mov   $14,#$e0
0725: 00        nop
0726: 00        nop
0727: 00        nop
0728: 8f e0 14  mov   $14,#$e0
072b: 00        nop
072c: 00        nop
072d: 00        nop
072e: 8f e0 14  mov   $14,#$e0
0731: 00        nop
0732: 00        nop
0733: 00        nop
0734: 8f e0 14  mov   $14,#$e0
0737: 00        nop
0738: 00        nop
0739: 00        nop
073a: 8f e0 14  mov   $14,#$e0
073d: 00        nop
073e: 00        nop
073f: 00        nop
0740: 8f e0 14  mov   $14,#$e0
0743: 00        nop
0744: 00        nop
0745: 00        nop
0746: 8f e0 14  mov   $14,#$e0
0749: 00        nop
074a: 00        nop
074b: 00        nop
074c: 8f e0 14  mov   $14,#$e0
074f: 00        nop
0750: 00        nop
0751: 00        nop
0752: 8f e0 14  mov   $14,#$e0
0755: 00        nop
0756: 00        nop
0757: 00        nop
0758: 8f e0 14  mov   $14,#$e0
075b: 00        nop
075c: 00        nop
075d: 00        nop
075e: 8f e0 14  mov   $14,#$e0
0761: 00        nop
0762: 00        nop
0763: 00        nop
0764: 8f e0 14  mov   $14,#$e0
0767: 00        nop
0768: 00        nop
0769: 00        nop
076a: 8f e0 14  mov   $14,#$e0
076d: 00        nop
076e: 00        nop
076f: 00        nop
0770: 8f e0 14  mov   $14,#$e0
0773: 00        nop
0774: 00        nop
0775: 00        nop
0776: 8f e0 14  mov   $14,#$e0
0779: 00        nop
077a: 00        nop
077b: 00        nop
077c: 8f e0 14  mov   $14,#$e0
077f: 00        nop
0780: 00        nop
0781: 00        nop
0782: 8f e0 14  mov   $14,#$e0
0785: 00        nop
0786: 00        nop
0787: 00        nop
0788: 8f e0 14  mov   $14,#$e0
078b: 00        nop
078c: 00        nop
078d: 00        nop
078e: 8f e0 14  mov   $14,#$e0
0791: 00        nop
0792: 00        nop
0793: 00        nop
0794: 8f e0 14  mov   $14,#$e0
0797: 00        nop
0798: 00        nop
0799: 00        nop
079a: 8f e0 14  mov   $14,#$e0
079d: 00        nop
079e: 00        nop
079f: 00        nop
07a0: 8f e0 14  mov   $14,#$e0
07a3: 00        nop
07a4: 00        nop
07a5: 00        nop
07a6: 8f e0 14  mov   $14,#$e0
07a9: 00        nop
07aa: 00        nop
07ab: 00        nop
07ac: 8f e0 14  mov   $14,#$e0
07af: 00        nop
07b0: 00        nop
07b1: 00        nop
07b2: 8f e0 14  mov   $14,#$e0
07b5: 00        nop
07b6: 00        nop
07b7: 00        nop
07b8: 8f e0 14  mov   $14,#$e0
07bb: 00        nop
07bc: 00        nop
07bd: 00        nop
07be: 8f e0 14  mov   $14,#$e0
07c1: 00        nop
07c2: 00        nop
07c3: 00        nop
07c4: 8f e0 14  mov   $14,#$e0
07c7: 00        nop
07c8: 00        nop
07c9: 00        nop
07ca: 8f e0 14  mov   $14,#$e0
07cd: 00        nop
07ce: 00        nop
07cf: 00        nop
07d0: 8f e0 14  mov   $14,#$e0
07d3: 00        nop
07d4: 00        nop
07d5: 00        nop
07d6: 8f e0 14  mov   $14,#$e0
07d9: 00        nop
07da: 00        nop
07db: 00        nop
07dc: 8f e0 14  mov   $14,#$e0
07df: 00        nop
07e0: 00        nop
07e1: 00        nop
07e2: 8f e0 14  mov   $14,#$e0
07e5: 00        nop
07e6: 00        nop
07e7: 00        nop
07e8: 8f e0 14  mov   $14,#$e0
07eb: 00        nop
07ec: 00        nop
07ed: 00        nop
07ee: 8f e0 14  mov   $14,#$e0
07f1: 00        nop
07f2: 00        nop
07f3: 00        nop
07f4: 8f e0 14  mov   $14,#$e0
07f7: 00        nop
07f8: 00        nop
07f9: 00        nop
07fa: 8f e0 14  mov   $14,#$e0
07fd: 00        nop
07fe: 00        nop
07ff: 00        nop
0800: 8f e0 14  mov   $14,#$e0
0803: 00        nop
0804: 00        nop
0805: 00        nop
0806: 8f e0 14  mov   $14,#$e0
0809: 00        nop
080a: 00        nop
080b: 00        nop
080c: 8f e0 14  mov   $14,#$e0
080f: 00        nop
0810: 00        nop
0811: 00        nop
0812: 8f e0 14  mov   $14,#$e0
0815: 00        nop
0816: 00        nop
0817: 00        nop
0818: 8f e0 14  mov   $14,#$e0
081b: 00        nop
081c: 00        nop
081d: 00        nop
081e: 8f e0 14  mov   $14,#$e0
0821: 00        nop
0822: 00        nop
0823: 00        nop
0824: 8f e0 14  mov   $14,#$e0
0827: 00        nop
0828: 00        nop
0829: 00        nop
082a: 8f e0 14  mov   $14,#$e0
082d: 00        nop
082e: 00        nop
082f: 00        nop
0830: 8f e0 14  mov   $14,#$e0
0833: 00        nop
0834: 00        nop
0835: 00        nop
0836: 8f e0 14  mov   $14,#$e0
0839: 00        nop
083a: 00        nop
083b: 00        nop
083c: 8f e0 14  mov   $14,#$e0
083f: 00        nop
0840: 00        nop
0841: 00        nop
0842: 8f e0 14  mov   $14,#$e0
0845: 00        nop
0846: 00        nop
0847: 00        nop
0848: 8f e0 14  mov   $14,#$e0
084b: 00        nop
084c: 00        nop
084d: 00        nop
084e: 8f e0 14  mov   $14,#$e0
0851: 00        nop
0852: 00        nop
0853: 00        nop
0854: 8f e0 14  mov   $14,#$e0
0857: 00        nop
0858: 00        nop
0859: 00        nop
085a: 8f e0 14  mov   $14,#$e0
085d: 00        nop
085e: 00        nop
085f: 00        nop
0860: 8f e0 14  mov   $14,#$e0
0863: 00        nop
0864: 00        nop
0865: 00        nop
0866: 8f e0 14  mov   $14,#$e0
0869: 00        nop
086a: 00        nop
086b: 00        nop
086c: 8f e0 14  mov   $14,#$e0
086f: 00        nop
0870: 00        nop
0871: 00        nop
0872: 8f e0 14  mov   $14,#$e0
0875: 00        nop
0876: 00        nop
0877: 00        nop
0878: 8f e0 14  mov   $14,#$e0
087b: 00        nop
087c: 00        nop
087d: 00        nop
087e: 8f e0 14  mov   $14,#$e0
0881: 00        nop
0882: 00        nop
0883: 00        nop
0884: 8f e0 14  mov   $14,#$e0
0887: 00        nop
0888: 00        nop
0889: 00        nop
088a: 8f e0 14  mov   $14,#$e0
088d: 00        nop
088e: 00        nop
088f: 00        nop
0890: 8f e0 14  mov   $14,#$e0
0893: 00        nop
0894: 00        nop
0895: 00        nop
0896: 8f e0 14  mov   $14,#$e0
0899: 00        nop
089a: 00        nop
089b: 00        nop
089c: 8f e0 14  mov   $14,#$e0
089f: 00        nop
08a0: 00        nop
08a1: 00        nop
08a2: 8f e0 14  mov   $14,#$e0
08a5: 00        nop
08a6: 00        nop
08a7: 00        nop
08a8: 8f e0 14  mov   $14,#$e0
08ab: 00        nop
08ac: 00        nop
08ad: 00        nop
08ae: 8f e0 14  mov   $14,#$e0
08b1: 00        nop
08b2: 00        nop
08b3: 00        nop
08b4: 8f e0 14  mov   $14,#$e0
08b7: 00        nop
08b8: 00        nop
08b9: 00        nop
08ba: 8f e0 14  mov   $14,#$e0
08bd: 00        nop
08be: 00        nop
08bf: 00        nop
08c0: 8f e0 14  mov   $14,#$e0
08c3: 00        nop
08c4: 00        nop
08c5: 00        nop
08c6: 8f e0 14  mov   $14,#$e0
08c9: 00        nop
08ca: 00        nop
08cb: 00        nop
08cc: 8f e0 14  mov   $14,#$e0
08cf: 00        nop
08d0: 00        nop
08d1: 00        nop
08d2: 8f e0 14  mov   $14,#$e0
08d5: 00        nop
08d6: 00        nop
08d7: 00        nop
08d8: 8f e0 14  mov   $14,#$e0
08db: 00        nop
08dc: 00        nop
08dd: 00        nop
08de: 8f e0 14  mov   $14,#$e0
08e1: 00        nop
08e2: 00        nop
08e3: 00        nop
08e4: 8f e0 14  mov   $14,#$e0
08e7: 00        nop
08e8: 00        nop
08e9: 00        nop
08ea: 8f e0 14  mov   $14,#$e0
08ed: 00        nop
08ee: 00        nop
08ef: 00        nop
08f0: 8f e0 14  mov   $14,#$e0
08f3: 00        nop
08f4: 00        nop
08f5: 00        nop
08f6: 8f e0 14  mov   $14,#$e0
08f9: 00        nop
08fa: 16 af 8f  or    a,$8faf+y
08fd: f1        tcall 15
08fe: 14 00     or    a,$00+x
0900: 00        nop
0901: 00        nop
0902: 8f e0 14  mov   $14,#$e0
0905: 00        nop
0906: 00        nop
0907: 00        nop
0908: 8f e0 14  mov   $14,#$e0
090b: 00        nop
090c: 04 00     or    a,$00
090e: 8f e0 14  mov   $14,#$e0
0911: 00        nop
0912: 00        nop
0913: 00        nop
0914: 8f e0 14  mov   $14,#$e0
0917: 00        nop
0918: 00        nop
0919: 00        nop
091a: 8f e0 14  mov   $14,#$e0
091d: 00        nop
091e: 02 80     set0  $80
0920: 8f e0 14  mov   $14,#$e0
0923: 00        nop
0924: 00        nop
0925: 00        nop
0926: 8f e0 14  mov   $14,#$e0
0929: 00        nop
092a: 00        nop
092b: 00        nop
092c: 8f e0 14  mov   $14,#$e0
092f: 00        nop
0930: 00        nop
0931: 00        nop
0932: 8f e0 14  mov   $14,#$e0
0935: 00        nop
0936: 00        nop
0937: 00        nop
0938: 8f e0 14  mov   $14,#$e0
093b: 00        nop
093c: 00        nop
093d: 00        nop
093e: 8f e0 14  mov   $14,#$e0
0941: 00        nop
0942: 00        nop
0943: 00        nop
0944: 8f e0 14  mov   $14,#$e0
0947: 00        nop
0948: 00        nop
0949: 00        nop
094a: 8f e0 14  mov   $14,#$e0
094d: 00        nop
094e: 00        nop
094f: 00        nop
0950: 8f e0 14  mov   $14,#$e0
0953: 00        nop
0954: 02 80     set0  $80
0956: 8f e0 14  mov   $14,#$e0
0959: 00        nop
095a: 00        nop
095b: 00        nop
095c: 8f e0 14  mov   $14,#$e0
095f: 00        nop
0960: 02 80     set0  $80
0962: 8f e0 14  mov   $14,#$e0
0965: 00        nop
0966: 00        nop
0967: 00        nop
0968: 8f e0 14  mov   $14,#$e0
096b: 00        nop
096c: 00        nop
096d: 00        nop
096e: 8f e0 14  mov   $14,#$e0
0971: 00        nop
0972: 00        nop
0973: 00        nop
0974: 8f e0 14  mov   $14,#$e0
0977: 00        nop
0978: 00        nop
0979: 00        nop
097a: 8f e0 14  mov   $14,#$e0
097d: 00        nop
097e: 00        nop
097f: 00        nop
0980: 8f e0 14  mov   $14,#$e0
0983: 00        nop
0984: 00        nop
0985: 00        nop
0986: 8f f1 14  mov   $14,#$f1
0989: 00        nop
098a: 00        nop
098b: 00        nop
098c: 8f e0 14  mov   $14,#$e0
098f: 00        nop
0990: 00        nop
0991: 00        nop
0992: 8f e0 14  mov   $14,#$e0
0995: 00        nop
0996: 00        nop
0997: 00        nop
0998: 8f e0 14  mov   $14,#$e0
099b: 00        nop
099c: 00        nop
099d: 00        nop
099e: 8f e0 14  mov   $14,#$e0
09a1: 00        nop
09a2: 00        nop
09a3: 00        nop
09a4: 8f e0 14  mov   $14,#$e0
09a7: 00        nop
09a8: 00        nop
09a9: 00        nop
09aa: 8f e0 14  mov   $14,#$e0
09ad: 00        nop
09ae: 00        nop
09af: 00        nop
09b0: 8f e0 14  mov   $14,#$e0
09b3: 00        nop
09b4: 00        nop
09b5: 00        nop
09b6: 8f e0 14  mov   $14,#$e0
09b9: 00        nop
09ba: 00        nop
09bb: 00        nop
09bc: 8f e0 14  mov   $14,#$e0
09bf: 00        nop
09c0: 00        nop
09c1: 00        nop
09c2: 8f e0 14  mov   $14,#$e0
09c5: 00        nop
09c6: 00        nop
09c7: 00        nop
09c8: 8f e0 14  mov   $14,#$e0
09cb: 00        nop
09cc: 00        nop
09cd: 00        nop
09ce: 8f e0 14  mov   $14,#$e0
09d1: 00        nop
09d2: 00        nop
09d3: 00        nop
09d4: 8f e0 14  mov   $14,#$e0
09d7: 00        nop
09d8: 00        nop
09d9: 00        nop
09da: 8f e0 14  mov   $14,#$e0
09dd: 00        nop
09de: 00        nop
09df: 00        nop
09e0: 8f e0 14  mov   $14,#$e0
09e3: 00        nop
09e4: 00        nop
09e5: 00        nop
09e6: 8f e0 14  mov   $14,#$e0
09e9: 00        nop
09ea: 00        nop
09eb: 00        nop
09ec: 8f e0 14  mov   $14,#$e0
09ef: 00        nop
09f0: 00        nop
09f1: 00        nop
09f2: 8f e0 14  mov   $14,#$e0
09f5: 00        nop
09f6: 00        nop
09f7: 00        nop
09f8: 8f e0 14  mov   $14,#$e0
09fb: 00        nop
09fc: 00        nop
09fd: 00        nop
09fe: 8f e0 14  mov   $14,#$e0
0a01: 00        nop
0a02: 00        nop
0a03: 00        nop
0a04: 8f e0 14  mov   $14,#$e0
0a07: 00        nop
0a08: 00        nop
0a09: 00        nop
0a0a: 8f e0 14  mov   $14,#$e0
0a0d: 00        nop
0a0e: 00        nop
0a0f: 00        nop
0a10: 8f e0 14  mov   $14,#$e0
0a13: 00        nop
0a14: 00        nop
0a15: 00        nop
0a16: 8f e0 14  mov   $14,#$e0
0a19: 00        nop
0a1a: 00        nop
0a1b: 00        nop
0a1c: 8f e0 14  mov   $14,#$e0
0a1f: 00        nop

0a20: dw $0b54
0a22: dw $0b8c
0a24: dw $0bb6
0a26: dw $0be7
0a28: dw $0c18
0a2a: dw $0c6c
0a2c: dw $0cab
0a2e: dw $0cdc
0a30: dw $0d1b
0a32: dw $0d29
0a34: dw $0d53
0a36: dw $0d76
0a38: dw $0dc3
0a3a: dw $0dd8

0a3c: 00        nop
0a3d: 16 b2 8f  or    a,$8fb2+y
0a40: e0        clrv
0a41: 14 00     or    a,$00+x
0a43: 01        tcall 0
0a44: 0a b0 8f  or1   c,$11f6,0
0a47: e0        clrv
0a48: 14 00     or    a,$00+x
0a4a: 02 00     set0  $00
0a4c: 00        nop
0a4d: 8f e0 14  mov   $14,#$e0
0a50: 00        nop
0a51: 03 00 00  bbs0  $00,$0a54
0a54: 8f e0 14  mov   $14,#$e0
0a57: 00        nop
0a58: 04 fb     or    a,$fb
0a5a: 00        nop
0a5b: 8f e0 14  mov   $14,#$e0
0a5e: 00        nop
0a5f: 05 00 00  or    a,$0000
0a62: 8f e0 14  mov   $14,#$e0
0a65: 00        nop
0a66: 06        or    a,(x)
0a67: 00        nop
0a68: 00        nop
0a69: 8f e0 14  mov   $14,#$e0
0a6c: 00        nop
0a6d: 07 00     or    a,($00+x)
0a6f: 00        nop
0a70: 8f e0 14  mov   $14,#$e0
0a73: 00        nop
0a74: 08 00     or    a,#$00
0a76: 00        nop
0a77: 8f e0 14  mov   $14,#$e0
0a7a: 00        nop
0a7b: 09 00 00  or    ($00),($00)
0a7e: 8f e0 14  mov   $14,#$e0
0a81: 00        nop
0a82: 0a 00 01  or1   c,$0020,0
0a85: 8f e0 14  mov   $14,#$e0
0a88: 00        nop
0a89: 0b fb     asl   $fb
0a8b: 00        nop
0a8c: 8f e0 14  mov   $14,#$e0
0a8f: 00        nop
0a90: 0c 00 23  asl   $2300
0a93: 8f e0 14  mov   $14,#$e0
0a96: 00        nop
0a97: 0d        push  psw
0a98: fb 00     mov   y,$00+x
0a9a: 8f e0 14  mov   $14,#$e0
0a9d: 00        nop
0a9e: 0e fb 00  tset1 $00fb
0aa1: 8f e0 14  mov   $14,#$e0
0aa4: 00        nop
0aa5: 0f        brk
0aa6: 07 04     or    a,($04+x)
0aa8: 8f e0 14  mov   $14,#$e0
0aab: 00        nop
0aac: 10 22     bpl   $0ad0
0aae: a8 8f     sbc   a,#$8f
0ab0: e0        clrv
0ab1: 14 00     or    a,$00+x
0ab3: 11        tcall 1
0ab4: fb 00     mov   y,$00+x
0ab6: 8f e0 14  mov   $14,#$e0
0ab9: 00        nop
0aba: 12 07     clr0  $07
0abc: ff        stop
0abd: 8f e0 14  mov   $14,#$e0
0ac0: 00        nop
0ac1: 13 07 02  bbc0  $07,$0ac6
0ac4: 8f e0 14  mov   $14,#$e0
0ac7: 00        nop
0ac8: 14 07     or    a,$07+x
0aca: 00        nop
0acb: 8f e0 14  mov   $14,#$e0
0ace: 00        nop
0acf: 15 00 00  or    a,$0000+x
0ad2: 8f e0 14  mov   $14,#$e0
0ad5: 00        nop
0ad6: 16 07 00  or    a,$0007+y
0ad9: 8f e0 14  mov   $14,#$e0
0adc: 00        nop
0add: 17 13     or    a,($13)+y
0adf: 05 8f e0  or    a,$e08f
0ae2: 14 00     or    a,$00+x
0ae4: 18 00 00  or    $00,#$00
0ae7: 8f e0 14  mov   $14,#$e0
0aea: 00        nop
0aeb: 19        or    (x),(y)
0aec: 00        nop
0aed: 00        nop
0aee: 8f e0 14  mov   $14,#$e0
0af1: 00        nop
0af2: 1a 00     decw  $00
0af4: 00        nop
0af5: 8f e0 14  mov   $14,#$e0
0af8: 00        nop
0af9: 1b 13     asl   $13+x
0afb: 00        nop
0afc: 8f e0 14  mov   $14,#$e0
0aff: 00        nop
0b00: 1c        asl   a
0b01: 00        nop
0b02: 00        nop
0b03: 8f e0 12  mov   $12,#$e0
0b06: 00        nop
0b07: 1d        dec   x
0b08: 04 00     or    a,$00
0b0a: bf        mov   a,(x)+
0b0b: 15 14 00  or    a,$0014+x
0b0e: 1e 05 83  cmp   x,$8305
0b11: 8f f1 14  mov   $14,#$f1
0b14: 00        nop
0b15: 1f 1c d7  jmp   ($d71c+x)

0b18: 8f e0 14  mov   $14,#$e0
0b1b: 00        nop
0b1c: 20        clrp
0b1d: 13 fd 8f  bbc0  $fd,$0aaf
0b20: e0        clrv
0b21: 14 00     or    a,$00+x
0b23: 21        tcall 2
0b24: 18 fe 8f  or    $8f,#$fe
0b27: e0        clrv
0b28: 14 00     or    a,$00+x
0b2a: 22 13     set1  $13
0b2c: 00        nop
0b2d: 8f e0 14  mov   $14,#$e0
0b30: 00        nop
0b31: 23 00 00  bbs1  $00,$0b34
0b34: 8f e0 16  mov   $16,#$e0
0b37: 00        nop
0b38: 24 0b     and   a,$0b
0b3a: 00        nop
0b3b: 8f f7 14  mov   $14,#$f7
0b3e: 00        nop
0b3f: 25 0b 39  and   a,$390b
0b42: 8f e0 14  mov   $14,#$e0
0b45: 00        nop
0b46: 26        and   a,(x)
0b47: 17 e6     or    a,($e6)+y
0b49: 8f f4 14  mov   $14,#$f4
0b4c: 00        nop
0b4d: 27 13     and   a,($13+x)
0b4f: ff        stop
0b50: 8f e0 14  mov   $14,#$e0
0b53: 00        nop
0b54: 28 04     and   a,#$04
0b56: 00        nop
0b57: cf        mul   ya
0b58: 35 14 00  and   a,$0014+x
0b5b: 29 18 fc  and   ($fc),($18)
0b5e: 8f e0 14  mov   $14,#$e0
0b61: 00        nop
0b62: 2a 00 00  or1   c,!($0000,0)
0b65: bf        mov   a,(x)+
0b66: d1        tcall 13
0b67: 0a 00 2b  or1   c,$0560,0
0b6a: 06        or    a,(x)
0b6b: 00        nop
0b6c: 9f        xcn   a
0b6d: d4 14     mov   $14+x,a
0b6f: 00        nop
0b70: 2c 1e 14  rol   $141e
0b73: cf        mul   ya
0b74: 13 14 00  bbc0  $14,$0b77
0b77: 2d        push  a
0b78: 12 46     clr0  $46
0b7a: 8f e0 14  mov   $14,#$e0
0b7d: 00        nop
0b7e: 2e 13 00  cbne  $13,$0b81
0b81: 8f e0 14  mov   $14,#$e0
0b84: 00        nop
0b85: 2f 0c     bra   $0b93
0b87: 00        nop
0b88: bf        mov   a,(x)+
0b89: d0 14     bne   $0b9f
0b8b: 00        nop
0b8c: 30 06     bmi   $0b94
0b8e: 00        nop
0b8f: 8f f6 14  mov   $14,#$f6
0b92: 00        nop
0b93: 31        tcall 3
0b94: 15 1e 8f  or    a,$8f1e+x
0b97: e0        clrv
0b98: 14 00     or    a,$00+x
0b9a: 32 15     clr1  $15
0b9c: 24 8f     and   a,$8f
0b9e: e0        clrv
0b9f: 14 00     or    a,$00+x
0ba1: 33 06 00  bbc1  $06,$0ba4
0ba4: 9f        xcn   a
0ba5: d4 14     mov   $14+x,a
0ba7: 00        nop
0ba8: 34 05     and   a,$05+x
0baa: 00        nop
0bab: 9f        xcn   a
0bac: 92 14     clr4  $14
0bae: 00        nop
0baf: 35 0c 00  and   a,$000c+x
0bb2: bf        mov   a,(x)+
0bb3: d0 14     bne   $0bc9
0bb5: 00        nop
0bb6: 36 12 46  and   a,$4612+y
0bb9: 8f e0 14  mov   $14,#$e0
0bbc: 00        nop
0bbd: 37 18     and   a,($18)+y
0bbf: f6 8f e0  mov   a,$e08f+y
0bc2: 14 00     or    a,$00+x
0bc4: 38 14 54  and   $54,#$14
0bc7: 8f e0 14  mov   $14,#$e0
0bca: 00        nop
0bcb: 39        and   (x),(y)
0bcc: 07 00     or    a,($00+x)
0bce: ff        stop
0bcf: 37 14     and   a,($14)+y
0bd1: 00        nop
0bd2: 3a 0d     incw  $0d
0bd4: 00        nop
0bd5: df        daa   a
0bd6: f7 14     mov   a,($14)+y
0bd8: 00        nop
0bd9: 3b 05     rol   $05+x
0bdb: 00        nop
0bdc: 8f ef 14  mov   $14,#$ef
0bdf: 00        nop
0be0: 3c        rol   a
0be1: 17 d9     or    a,($d9)+y
0be3: 8f e0 14  mov   $14,#$e0
0be6: 00        nop
0be7: 3d        inc   x
0be8: 18 fc 8f  or    $8f,#$fc
0beb: e0        clrv
0bec: 14 00     or    a,$00+x
0bee: 3e 07     cmp   x,$07
0bf0: 00        nop
0bf1: ff        stop
0bf2: 37 14     and   a,($14)+y
0bf4: 00        nop
0bf5: 3f 0d 00  call  $000d
0bf8: df        daa   a
0bf9: f7 14     mov   a,($14)+y
0bfb: 00        nop
0bfc: 40        setp
0bfd: 06        or    a,(x)
0bfe: 59        eor   (x),(y)
0bff: 8f e0 14  mov   $14,#$e0
0c02: 00        nop
0c03: 41        tcall 4
0c04: 05 00 9f  or    a,$9f00
0c07: 92 14     clr4  $14
0c09: 00        nop
0c0a: 42 13     set2  $13
0c0c: fc        inc   y
0c0d: 8f f3 14  mov   $14,#$f3
0c10: 00        nop
0c11: 43 07 ff  bbs2  $07,$0c13
0c14: 8f e0 14  mov   $14,#$e0
0c17: 00        nop
0c18: 44 13     eor   a,$13
0c1a: fc        inc   y
0c1b: 8f f3 14  mov   $14,#$f3
0c1e: 00        nop
0c1f: 45 18 00  eor   a,$0018
0c22: 8f e0 14  mov   $14,#$e0
0c25: 00        nop
0c26: 46        eor   a,(x)
0c27: 07 ff     or    a,($ff+x)
0c29: 8f e0 14  mov   $14,#$e0
0c2c: 00        nop
0c2d: 47 15     eor   a,($15+x)
0c2f: 1e 8f e0  cmp   x,$e08f
0c32: 14 00     or    a,$00+x
0c34: 48 23     eor   a,#$23
0c36: da 8f     movw  $8f,ya
0c38: e0        clrv
0c39: 14 00     or    a,$00+x
0c3b: 49 0c 00  eor   ($00),($0c)
0c3e: bf        mov   a,(x)+
0c3f: d0 14     bne   $0c55
0c41: 00        nop
0c42: 4a 07 00  and1  c,$0000,7
0c45: ff        stop
0c46: 37 14     and   a,($14)+y
0c48: 00        nop
0c49: 4b 0d     lsr   $0d
0c4b: 00        nop
0c4c: df        daa   a
0c4d: f7 14     mov   a,($14)+y
0c4f: 00        nop
0c50: 4c 0c 00  lsr   $000c
0c53: 8f f7 14  mov   $14,#$f7
0c56: 00        nop
0c57: 4d        push  x
0c58: 0e 00 bf  tset1 $bf00
0c5b: d7 14     mov   ($14)+y,a
0c5d: 00        nop
0c5e: 4e 11 f2  tclr1 $f211
0c61: 8f f1 14  mov   $14,#$f1
0c64: 00        nop
0c65: 4f 12     pcall $12
0c67: 00        nop
0c68: 8f fe 10  mov   $10,#$fe
0c6b: 00        nop
0c6c: 50 13     bvc   $0c81
0c6e: fc        inc   y
0c6f: 8f f3 14  mov   $14,#$f3
0c72: 00        nop
0c73: 51        tcall 5
0c74: 0c f4 8f  asl   $8ff4
0c77: e0        clrv
0c78: 14 00     or    a,$00+x
0c7a: 52 00     clr2  $00
0c7c: 00        nop
0c7d: 8f f0 14  mov   $14,#$f0
0c80: 00        nop
0c81: 53 16 af  bbc2  $16,$0c33
0c84: 8f f1 14  mov   $14,#$f1
0c87: 00        nop
0c88: 54 00     eor   a,$00+x
0c8a: 00        nop
0c8b: 8f e0 0e  mov   $0e,#$e0
0c8e: 00        nop
0c8f: 55 02 80  eor   a,$8002+x
0c92: 8f e0 14  mov   $14,#$e0
0c95: 00        nop
0c96: 56 07 00  eor   a,$0007+y
0c99: ff        stop
0c9a: 37 14     and   a,($14)+y
0c9c: 00        nop
0c9d: 57 0d     eor   a,($0d)+y
0c9f: 00        nop
0ca0: df        daa   a
0ca1: f7 14     mov   a,($14)+y
0ca3: 00        nop
0ca4: 58 1a 2a  eor   $2a,#$1a
0ca7: 8f e0 14  mov   $14,#$e0
0caa: 00        nop
0cab: 59        eor   (x),(y)
0cac: 36 81 8f  and   a,$8f81+y
0caf: e0        clrv
0cb0: 14 00     or    a,$00+x
0cb2: 5a 15     cmpw  ya,$15
0cb4: 3f 8f e0  call  $e08f
0cb7: 14 00     or    a,$00+x
0cb9: 5b 0b     lsr   $0b+x
0cbb: 00        nop
0cbc: 8f e0 14  mov   $14,#$e0
0cbf: 00        nop
0cc0: 5c        lsr   a
0cc1: 12 00     clr0  $00
0cc3: 8f fe 10  mov   $10,#$fe
0cc6: 00        nop
0cc7: 5d        mov   x,a
0cc8: 18 f6 8f  or    $8f,#$f6
0ccb: e0        clrv
0ccc: 14 00     or    a,$00+x
0cce: 5e 00 00  cmp   y,$0000
0cd1: 8f e0 14  mov   $14,#$e0
0cd4: 00        nop
0cd5: 5f 05 00  jmp   $0005

0cd8: 9f        xcn   a
0cd9: 92 14     clr4  $14
0cdb: 00        nop
0cdc: 60        clrc
0cdd: fa 00 8f  mov   ($8f),($00)
0ce0: e0        clrv
0ce1: 14 00     or    a,$00+x
0ce3: 61        tcall 6
0ce4: fa 00 8f  mov   ($8f),($00)
0ce7: e0        clrv
0ce8: 14 00     or    a,$00+x
0cea: 62 fa     set3  $fa
0cec: 00        nop
0ced: 8f e0 14  mov   $14,#$e0
0cf0: 00        nop
0cf1: 63 fa 00  bbs3  $fa,$0cf4
0cf4: 8f e0 14  mov   $14,#$e0
0cf7: 00        nop
0cf8: 64 fa     cmp   a,$fa
0cfa: 00        nop
0cfb: 8f e0 14  mov   $14,#$e0
0cfe: 00        nop
0cff: 65 fa 00  cmp   a,$00fa
0d02: 8f e0 14  mov   $14,#$e0
0d05: 00        nop
0d06: 66        cmp   a,(x)
0d07: 00        nop
0d08: 00        nop
0d09: 8d f8     mov   y,#$f8
0d0b: 14 00     or    a,$00+x
0d0d: 67 00     cmp   a,($00+x)
0d0f: 00        nop
0d10: 8f 10 14  mov   $14,#$10
0d13: 00        nop
0d14: 68 07     cmp   a,#$07
0d16: ff        stop
0d17: 8f e0 14  mov   $14,#$e0
0d1a: 00        nop
0d1b: 69 00 00  cmp   ($00),($00)
0d1e: 8f e0 14  mov   $14,#$e0
0d21: 00        nop
0d22: 6a 07 ff  and1  c,!($1fe0,7)
0d25: 8f e0 14  mov   $14,#$e0
0d28: 00        nop
0d29: 6b 00     ror   $00
0d2b: 00        nop
0d2c: 8f e0 14  mov   $14,#$e0
0d2f: 00        nop
0d30: 6c 12 5e  ror   $5e12
0d33: 8f e0 14  mov   $14,#$e0
0d36: 00        nop
0d37: 6d        push  y
0d38: 06        or    a,(x)
0d39: 00        nop
0d3a: 8f f6 14  mov   $14,#$f6
0d3d: 00        nop
0d3e: 6e f3 00  dbnz  $f3,$0d41
0d41: 8f e0 14  mov   $14,#$e0
0d44: 00        nop
0d45: 6f        ret

0d46: 07 ff     or    a,($ff+x)
0d48: 8f e0 14  mov   $14,#$e0
0d4b: 00        nop
0d4c: 70 00     bvs   $0d4e
0d4e: 00        nop
0d4f: 8f f0 14  mov   $14,#$f0
0d52: 00        nop
0d53: 71        tcall 7
0d54: 06        or    a,(x)
0d55: 35 8f e0  and   a,$e08f+x
0d58: 14 00     or    a,$00+x
0d5a: 72 00     clr3  $00
0d5c: 00        nop
0d5d: bf        mov   a,(x)+
0d5e: d1        tcall 13
0d5f: 0a 00 73  or1   c,$0e60,0
0d62: 15 1e 8f  or    a,$8f1e+x
0d65: e0        clrv
0d66: 14 00     or    a,$00+x
0d68: 74 15     cmp   a,$15+x
0d6a: 32 8f     clr1  $8f
0d6c: e0        clrv
0d6d: 14 00     or    a,$00+x
0d6f: 75 15 00  cmp   a,$0015+x
0d72: 9f        xcn   a
0d73: f0 14     beq   $0d89
0d75: 00        nop
0d76: 76 00 00  cmp   a,$0000+y
0d79: 8f f2 14  mov   $14,#$f2
0d7c: 00        nop
0d7d: 77 06     cmp   a,($06)+y
0d7f: 00        nop
0d80: 8f e0 14  mov   $14,#$e0
0d83: 00        nop
0d84: 78 00 00  cmp   $00,#$00
0d87: 8f e0 18  mov   $18,#$e0
0d8a: 00        nop
0d8b: 79        cmp   (x),(y)
0d8c: 00        nop
0d8d: 00        nop
0d8e: 8f e0 18  mov   $18,#$e0
0d91: 00        nop
0d92: 7a 00     addw  ya,$00
0d94: 00        nop
0d95: 8f e0 18  mov   $18,#$e0
0d98: 00        nop
0d99: 7b 00     ror   $00+x
0d9b: 00        nop
0d9c: 8f e0 18  mov   $18,#$e0
0d9f: 00        nop
0da0: 7c        ror   a
0da1: 00        nop
0da2: 00        nop
0da3: 8f e0 18  mov   $18,#$e0
0da6: 00        nop
0da7: 7d        mov   a,x
0da8: 00        nop
0da9: 00        nop
0daa: 8f e0 18  mov   $18,#$e0
0dad: 00        nop
0dae: 7e 00     cmp   y,$00
0db0: 00        nop
0db1: 8f e0 18  mov   $18,#$e0
0db4: 00        nop
0db5: 7f        reti
0db6: 15 32 8f  or    a,$8f32+x
0db9: e0        clrv
0dba: 14 00     or    a,$00+x
0dbc: 80        setc
0dbd: 18 00 8f  or    $8f,#$00
0dc0: e0        clrv
0dc1: 14 00     or    a,$00+x
0dc3: 81        tcall 8
0dc4: 00        nop
0dc5: 00        nop
0dc6: 8f e0 14  mov   $14,#$e0
0dc9: 00        nop
0dca: 82 00     set4  $00
0dcc: 00        nop
0dcd: 8f e0 14  mov   $14,#$e0
0dd0: 00        nop
0dd1: 83 00 00  bbs4  $00,$0dd4
0dd4: 8f e0 14  mov   $14,#$e0
0dd7: 00        nop
0dd8: 84 00     adc   a,$00
0dda: 00        nop
0ddb: 8f e0 14  mov   $14,#$e0
0dde: 00        nop
0ddf: 85 00 00  adc   a,$0000
0de2: 8f e0 14  mov   $14,#$e0
0de5: 00        nop
0de6: 23 00 00  bbs1  $00,$0de9
0de9: 8f e0 16  mov   $16,#$e0
0dec: 00        nop
0ded: 23 ff 00  bbs1  $ff,$0df0
0df0: 8f e0 14  mov   $14,#$e0
0df3: 00        nop
0df4: 24 0b     and   a,$0b
0df6: 00        nop
0df7: 8f f7 14  mov   $14,#$f7
0dfa: 00        nop
0dfb: 24 0b     and   a,$0b
0dfd: 00        nop
0dfe: 9f        xcn   a
0dff: d7 14     mov   ($14)+y,a
0e01: 00        nop
0e02: 14 00     or    a,$00+x
0e04: 25 14 88  and   a,$8814
0e07: 25 14 2d  and   a,$2d14
0e0a: 28 14     and   a,#$14
0e0c: 5f 28 14  jmp   $1428

0e0f: 33 25 14  bbc1  $25,$0e26
0e12: 5d        mov   x,a
0e13: 26        and   a,(x)
0e14: 14 6e     or    a,$6e+x
0e16: 26        and   a,(x)
0e17: 0f        brk
0e18: e8 26     mov   a,#$26
0e1a: 14 f9     or    a,$f9+x
0e1c: 26        and   a,(x)
0e1d: 14 bc     or    a,$bc+x
0e1f: 2b 14     rol   $14
0e21: 11        tcall 1
0e22: 2c 0f 33  rol   $330f
0e25: 2c 0d ef  rol   $ef0d
0e28: 24 0d     and   a,$0d
0e2a: 66        cmp   a,(x)
0e2b: 25 14 bb  and   a,$bb14
0e2e: 25 0f de  and   a,$de0f
0e31: 28 14     and   a,#$14
0e33: d9 2d     mov   $2d+y,x
0e35: 0f        brk
0e36: fb 2d     mov   y,$2d+x
0e38: 14 7f     or    a,$7f+x
0e3a: 2a 3f ad  or1   c,!($15a7,7)
0e3d: 2c 34 a8  rol   $a834
0e40: 28 0b     and   a,#$0b
0e42: 34 2b     and   a,$2b+x
0e44: 1f 52 29  jmp   ($2952+x)

0e47: 15 dc 29  or    a,$29dc+x
0e4a: 0f        brk
0e4b: fc        inc   y
0e4c: 25 0a 69  and   a,$690a
0e4f: 29 0a 8e  and   ($8e),($0a)
0e52: 29 01 b3  and   ($b3),($01)
0e55: 29 0f 73  and   ($73),($0f)
0e58: 2d        push  a
0e59: 00        nop
0e5a: 00        nop
0e5b: 00        nop
0e5c: 0f        brk
0e5d: 22 25     set1  $25
0e5f: 09 e9 27  or    ($27),($e9)
0e62: 09 fa 27  or    ($27),($fa)
0e65: 0f        brk
0e66: 0b 28     asl   $28
0e68: 09 1c 28  or    ($28),($1c)
0e6b: 09 3e 28  or    ($28),($3e)
0e6e: 0f        brk
0e6f: 4e 28 09  tclr1 $0928
0e72: 73 28 14  bbc3  $28,$0e89
0e75: 44 25     eor   a,$25
0e77: 09 55 25  or    ($25),($55)
0e7a: 0a 82 26  or1   c,$04d0,2
0e7d: 09 93 26  or    ($26),($93)
0e80: 0f        brk
0e81: a4 26     sbc   a,$26
0e83: 09 b5 26  or    ($26),($b5)
0e86: 09 c6 26  or    ($26),($c6)
0e89: 09 d7 26  or    ($26),($d7)
0e8c: 0f        brk
0e8d: cd 28     mov   x,#$28
0e8f: 09 77 25  or    ($25),($77)
0e92: 14 cd     or    a,$cd+x
0e94: 2b 09     rol   $09
0e96: de 2b 09  cbne  $2b+x,$0ea2
0e99: ef        sleep
0e9a: 2b 0f     rol   $0f
0e9c: 00        nop
0e9d: 2c 14 22  rol   $2214
0ea0: 2c 09 44  rol   $4409
0ea3: 2c 09 55  rol   $5509
0ea6: 2c 14 11  rol   $1114
0ea9: 25 09 99  and   a,$9909
0eac: 25 14 aa  and   a,$aa14
0eaf: 25 09 cc  and   a,$cc09
0eb2: 25 0f 71  and   a,$710f
0eb5: 2a 0f de  or1   c,!($1bc1,7)
0eb8: 24 09     and   a,$09
0eba: 84 2d     adc   a,$2d
0ebc: 0f        brk
0ebd: 95 2d 09  adc   a,$092d+x
0ec0: a6        sbc   a,(x)
0ec1: 2d        push  a
0ec2: 14 b7     or    a,$b7+x
0ec4: 2d        push  a
0ec5: 09 c8 2d  or    ($2d),($c8)
0ec8: 09 ea 2d  or    ($2d),($ea)
0ecb: 14 0c     or    a,$0c+x
0ecd: 2e 06 0a  cbne  $06,$0eda
0ed0: 27 08     and   a,($08+x)
0ed2: c3 29 0a  bbs6  $29,$0edf
0ed5: d3 23 0b  bbc6  $23,$0ee3
0ed8: fd        mov   y,a
0ed9: 23 0b 3b  bbs1  $0b,$0f17
0edc: 2d        push  a
0edd: 09 b8 2a  or    ($2a),($b8)
0ee0: 09 1d 2e  or    ($2e),($1d)
0ee3: 0d        push  psw
0ee4: 66        cmp   a,(x)
0ee5: 2c 09 55  rol   $5509
0ee8: 27 07     and   a,($07+x)
0eea: c8 2a     cmp   x,#$2a
0eec: 0d        push  psw
0eed: 0d        push  psw
0eee: 2a 2a 36  or1   c,!($06c5,2)
0ef1: 2a 0b 5b  or1   c,!($0b61,3)
0ef4: 24 2b     and   a,$2b
0ef6: da 2a     movw  $2a,ya
0ef8: 0d        push  psw
0ef9: dd        mov   a,y
0efa: 25 0b 9d  and   a,$9d0b
0efd: 2b 07     rol   $07
0eff: 10 2d     bpl   $0f2e
0f01: 07 a3     or    a,($a3+x)
0f03: 24 0d     and   a,$0d
0f05: ef        sleep
0f06: 28 2a     and   a,#$2a
0f08: 66        cmp   a,(x)
0f09: 27 0d     and   a,($0d+x)
0f0b: d4 24     mov   $24+x,a
0f0d: 2a 04 29  or1   c,!($0520,4)
0f10: 2a 26 29  or1   c,!($0524,6)
0f13: 0a 13 24  or1   c,$0482,3
0f16: 07 78     or    a,($78+x)
0f18: 23 05 38  bbs1  $05,$0f53
0f1b: 2e 05 17  cbne  $05,$0f35
0f1e: 2a 14 1b  or1   c,!($0362,4)
0f21: 27 07     and   a,($07+x)
0f23: b3 24 0b  bbc5  $24,$0f31
0f26: df        daa   a
0f27: 2c 07 f1  rol   $f107
0f2a: 29 0d 00  and   ($00),($0d)
0f2d: 2b 0d     rol   $0d
0f2f: 84 28     adc   a,$28
0f31: 0d        push  psw
0f32: 0e 2b 07  tset1 $072b
0f35: 9c        dec   a
0f36: 28 0f     and   a,#$0f
0f38: b0 2b     bcs   $0f65
0f3a: 0a 87 23  or1   c,$0470,7
0f3d: 07 9f     or    a,($9f+x)
0f3f: 23 0a 4a  bbs1  $0a,$0f8c
0f42: 26        and   a,(x)
0f43: 0a 90 27  or1   c,$04f2,0
0f46: 0a d2 29  or1   c,$053a,2
0f49: 08 ed     or    a,#$ed
0f4b: 25 0b 51  and   a,$510b
0f4e: 2e 2b 45  cbne  $2b,$0f96
0f51: 2b 0b     rol   $0b
0f53: 1d        dec   x
0f54: 2b 0a     rol   $0a
0f56: 86        adc   a,(x)
0f57: 24 0d     and   a,$0d
0f59: 6a 24 2a  and1  c,!($0544,4)
0f5c: 13 26 07  bbc0  $26,$0f66
0f5f: b9        sbc   (x),(y)
0f60: 23 0b a5  bbs1  $0b,$0f08
0f63: 27 0a     and   a,($0a+x)
0f65: e1        tcall 14
0f66: 23 09 ef  bbs1  $09,$0f58
0f69: 23 0b c6  bbs1  $0b,$0f32
0f6c: 27 0a     and   a,($0a+x)
0f6e: 82 2b     set4  $2b
0f70: 0a c3 24  or1   c,$0498,3
0f73: 14 90     or    a,$90+x
0f75: 2c 2c 2e  rol   $2e2c
0f78: 27 0a     and   a,($0a+x)
0f7a: 7f        reti
0f7b: 2c 2a 93  rol   $932a
0f7e: 2a 2a 37  or1   c,!($06e5,2)
0f81: 24 03     and   a,$03
0f83: e7 30     mov   a,($30+x)
0f85: 01        tcall 0
0f86: ca 36 03  mov1  $0066,6,c
0f89: 1a 31     decw  $31
0f8b: 01        tcall 0
0f8c: e8 36     mov   a,#$36
0f8e: 06        or    a,(x)
0f8f: fb 36     mov   y,$36+x
0f91: 05 03 30  or    a,$3003
0f94: 05 14 30  or    a,$3014
0f97: 05 27 30  or    a,$3027
0f9a: 05 3a 30  or    a,$303a
0f9d: 05 77 32  or    a,$3277
0fa0: 05 8a 32  or    a,$328a
0fa3: 05 f5 32  or    a,$32f5
0fa6: 03 6e 31  bbs0  $6e,$0fda
0fa9: 06        or    a,(x)
0faa: c0        di
0fab: 32 05     clr1  $05
0fad: 3e 36     cmp   x,$36
0faf: 05 70 36  or    a,$3670
0fb2: 07 29     or    a,($29+x)
0fb4: 34 07     and   a,$07+x
0fb6: 56 34 08  eor   a,$0834+y
0fb9: 98 2f 07  adc   $07,#$2f
0fbc: ef        sleep
0fbd: 31        tcall 3
0fbe: 03 a4 35  bbs0  $a4,$0ff6
0fc1: 0a 89 2e  or1   c,$05d1,1
0fc4: 0a cc 2e  or1   c,$05d9,4
0fc7: 0a f1 2e  or1   c,$05de,1
0fca: 0a de 2f  or1   c,$05fb,6
0fcd: 08 6b     or    a,#$6b
0fcf: 33 09 19  bbc1  $09,$0feb
0fd2: 37 0a     and   a,($0a)+y
0fd4: 30 37     bmi   $100d
0fd6: 0b 1c     asl   $1c
0fd8: 32 04     clr1  $04
0fda: 99        adc   (x),(y)
0fdb: 30 04     bmi   $0fe1
0fdd: ae        pop   a
0fde: 30 07     bmi   $0fe7
0fe0: 62 32     set3  $32
0fe2: 07 83     or    a,($83+x)
0fe4: 31        tcall 3
0fe5: 2a a5 31  or1   c,!($0634,5)
0fe8: 2a a0 36  or1   c,!($06d4,0)
0feb: 2a 07 36  or1   c,!($06c0,7)
0fee: 0a 81 2f  or1   c,$05f0,1
0ff1: 0a c3 30  or1   c,$0618,3
0ff4: 0d        push  psw
0ff5: 5b 2f     lsr   $2f+x
0ff7: 06        or    a,(x)
0ff8: 85 30 05  adc   a,$0530
0ffb: 77 38     cmp   a,($38)+y
0ffd: 0f        brk
0ffe: 02 34     set0  $34
1000: 0b 12     asl   $12
1002: 38 05 3c  and   $3c,#$05
1005: 38 0a 4d  and   $4d,#$0a
1008: 38 08 03  and   $03,#$08
100b: 32 09     clr1  $09
100d: 66        cmp   a,(x)
100e: 2e 0f ef  cbne  $0f,$1000
1011: 33 05 1d  bbc1  $05,$1031
1014: 33 05 71  bbc1  $05,$1088
1017: 30 2b     bmi   $1044
1019: cc 37 34  mov   $3437,y
101c: 68 34     cmp   a,#$34
101e: 03 84 37  bbs0  $84,$1058
1021: 03 a7 37  bbs0  $a7,$105b
1024: 04 4d     or    a,$4d
1026: 31        tcall 3
1027: 04 b2     or    a,$b2
1029: 35 0a 67  and   a,$670a+x
102c: 35 0a 6d  and   a,$6d0a+x
102f: 2f 08     bra   $1039
1031: 60        clrc
1032: 38 03 d4  and   $d4,#$03
1035: 35 04 30  and   a,$3004+x
1038: 33 06 71  bbc1  $06,$10ac
103b: 37 05     and   a,($05)+y
103d: cc 2f 2a  mov   $2a2f,y
1040: 0e 2f 2a  tset1 $2a2f
1043: 1f 35 3f  jmp   ($3f35+x)

1046: a4 33     sbc   a,$33
1048: 2f 4b     bra   $1095
104a: 30 3e     bmi   $108a
104c: f5 2f

104e: 00 18 18 0a
1052: 0a 0c 0c
1055: 12 12
1057: 12 12
1059: 12 12
105b: 12 16
105d: 12 12
105f: 12 00
1061: 00
1062: 00
1063: 0a 00 00
1066: 08 08
1068: 08 16
106a: 02 06
106c: 0e 02 02
106f: 02 10
1071: 02 0a
1073: 0a 00 0a
1076: 0a 0a 0a
1079: 0a 08 0a
107c: 0a 12 00
107f: 12 12
1081: 04 0a
1083: 0a 0c 02
1086: 06
1087: 06
1088: 06
1089: 14 14
108b: 00
108c: 0a 0c 12
108f: 00
1090: 06
1091: 08 08
1093: 08 08
1095: 08 02
1097: 12 14
1099: 0a 14 14
109c: 12 16
109e: 00
109f: 0e 08 00
10a2: 00
10a3: 0c 0c 00
10a6: 00
10a7: 00
10a8: 1a 18
10aa: 1a

10ab: db $01,$04,$08,$04

10af: dw $baaf  ; 00
10b1: dw $baaf  ; 01
10b3: dw $baaf  ; 02
10b5: dw $baaf  ; 03
10b7: dw $baaf  ; 04
10b9: dw $baaf  ; 05
10bb: dw $baaf  ; 06
10bd: dw $baaf  ; 07
10bf: dw $baaf  ; 08
10c1: dw $baaf  ; 09
10c3: dw $baaf  ; 0a
10c5: dw $baaf  ; 0b
10c7: dw $baaf  ; 0c
10c9: dw $baaf  ; 0d
10cb: dw $3900  ; 0e

10cd: dw $114d  ; e2
10cf: dw $115b  ; e3
10d1: dw $10e9  ; e4
10d3: dw $10f4  ; e5
10d5: dw $1101  ; e6
10d7: dw $110e  ; e7
10d9: dw $111c  ; e8
10db: dw $111f  ; e9
10dd: dw $118a  ; ea
10df: dw $118a  ; eb
10e1: dw $118a  ; ec
10e3: dw $118a  ; ed
10e5: dw $1122  ; ee
10e7: dw $115b  ; ef






















10e9: e4 2c     mov   a,$2c
10eb: d0 06     bne   $10f3
10ed: 8f 04 2c  mov   $2c,#$04
10f0: 8f 08 2d  mov   $2d,#$08
10f3: 6f        ret

10f4: e4 2c     mov   a,$2c
10f6: f0 fb     beq   $10f3
10f8: 78 ff 2d  cmp   $2d,#$ff
10fb: f0 f6     beq   $10f3
10fd: 8f ff 2d  mov   $2d,#$ff
1100: 6f        ret

1101: 8f 3f 10  mov   $10,#$3f
1104: 8f 00 0e  mov   $0e,#$00
1107: 22 1f     set1  $1f
1109: e8 46     mov   a,#$46
110b: 5f 18 13  jmp   $1318

110e: 32 1f     clr1  $1f
1110: e4 0f     mov   a,$0f
1112: 78 49 d0  cmp   $d0,#$49
1115: b0 02     bcs   $1119
1117: 28 3f     and   a,#$3f
1119: c4 0e     mov   $0e,a
111b: 6f        ret

111c: 02 1f     set0  $1f
111e: 6f        ret

111f: 12 1f     clr0  $1f
1121: 6f        ret

1122: 3f f4 1e  call  $1ef4
1125: 8f 00 1c  mov   $1c,#$00
1128: 8f 00 04  mov   $04,#$00
112b: 8f 80 05  mov   $05,#$80
112e: 8d 10     mov   y,#$10
1130: f6 8f 01  mov   a,$018f+y
1133: 28 01     and   a,#$01
1135: d0 0c     bne   $1143
1137: 09 05 04  or    ($04),($05)
113a: d6 ce 00  mov   $00ce+y,a
113d: d6 9f 01  mov   $019f+y,a
1140: d6 8e 01  mov   $018e+y,a
1143: 4b 05     lsr   $05
1145: dc        dec   y
1146: fe e8     dbnz  y,$1130
1148: e4 04     mov   a,$04
114a: 5f 4b 14  jmp   $144b

114d: e4 1c     mov   a,$1c
114f: d0 09     bne   $115a
1151: 8f ff 1c  mov   $1c,#$ff
1154: e8 20     mov   a,#$20
1156: c4 1d     mov   $1d,a
1158: c4 1e     mov   $1e,a
115a: 6f        ret

115b: 8f 00 26  mov   $26,#$00
115e: f2 1f     clr7  $1f
1160: e8 7f     mov   a,#$7f
1162: 8f 0c f2  mov   $f2,#$0c
1165: c4 f3     mov   $f3,a             ; set #$7f to MVOL(L)
1167: 8f 1c f2  mov   $f2,#$1c
116a: c4 f3     mov   $f3,a             ; set #$7f to MVOL(R)
116c: 3f f4 1e  call  $1ef4
116f: e8 00     mov   a,#$00
1171: c4 1c     mov   $1c,a
1173: c4 2a     mov   $2a,a
1175: 32 1f     clr1  $1f
1177: 8d 10     mov   y,#$10
1179: d6 ce 00  mov   $00ce+y,a
117c: d6 9f 01  mov   $019f+y,a
117f: d6 8e 01  mov   $018e+y,a
1182: dc        dec   y
1183: fe f4     dbnz  y,$1179
1185: e8 ff     mov   a,#$ff
1187: 5f 4b 14  jmp   $144b

118a: e4 1c     mov   a,$1c
118c: d0 15     bne   $11a3
118e: dd        mov   a,y
118f: 80        setc
1190: a8 ea     sbc   a,#$ea
1192: 5d        mov   x,a
1193: f5 ab 10  mov   a,$10ab+x
1196: 8f ff 1c  mov   $1c,#$ff
1199: c4 1d     mov   $1d,a
119b: ad ed     cmp   y,#$ed
119d: d0 02     bne   $11a1
119f: 08 80     or    a,#$80
11a1: c4 1e     mov   $1e,a
11a3: 6f        ret

11a4: fd        mov   y,a
11a5: 80        setc
11a6: a8 e2     sbc   a,#$e2
11a8: 90 f9     bcc   $11a3
11aa: 1c        asl   a
11ab: 5d        mov   x,a
11ac: 1f cd 10  jmp   ($10cd+x)

11af: 68 a6     cmp   a,#$a6
11b1: b0 07     bcs   $11ba
11b3: 68 02     cmp   a,#$02
11b5: 90 ec     bcc   $11a3
11b7: 5f 18 13  jmp   $1318

11ba: 68 f0     cmp   a,#$f0
11bc: 90 e6     bcc   $11a4
11be: 68 ff     cmp   a,#$ff
11c0: d0 03     bne   $11c5
11c2: 5f 9c 12  jmp   $129c

11c5: 28 0f     and   a,#$0f
11c7: 1c        asl   a
11c8: fd        mov   y,a
11c9: f6 af 10  mov   a,$10af+y
11cc: c4 2e     mov   $2e,a
11ce: f6 b0 10  mov   a,$10b0+y
11d1: c4 2f     mov   $2f,a
11d3: 5f d7 12  jmp   $12d7

11d6: 68 81     cmp   a,#$81
11d8: b0 26     bcs   $1200
11da: 8d 86     mov   y,#$86
11dc: 64 27     cmp   a,$27
11de: d0 04     bne   $11e4
11e0: e2 2a     set7  $2a
11e2: 2f 07     bra   $11eb
11e4: 64 28     cmp   a,$28
11e6: d0 18     bne   $1200
11e8: c2 2a     set6  $2a
11ea: fc        inc   y
11eb: cb 09     mov   $09,y
11ed: 8f 01 0c  mov   $0c,#$01
11f0: c4 0d     mov   $0d,a
11f2: 8f ff 04  mov   $04,#$ff
11f5: 8f 0d 05  mov   $05,#$0d
11f8: 5f 2a 13  jmp   $132a

11fb: e2 1f     set7  $1f
11fd: 8f ff f7  mov   $f7,#$ff
1200: 6f        ret

1201: eb f5     mov   y,$f5
1203: d0 fc     bne   $1201
1205: cb f5     mov   $f5,y
1207: eb f6     mov   y,$f6
1209: cb 2b     mov   $2b,y
120b: 68 f1     cmp   a,#$f1
120d: f0 ec     beq   $11fb
120f: 68 ff     cmp   a,#$ff
1211: d0 16     bne   $1229
1213: c4 26     mov   $26,a
1215: 8f 00 29  mov   $29,#$00
1218: 3f 25 11  call  $1125
121b: 8f 5c f2  mov   $f2,#$5c
121e: c4 f3     mov   $f3,a             ; set KOF
1220: 8f 00 2e  mov   $2e,#$00
1223: 8f 39 2f  mov   $2f,#$39
1226: 5f 98 12  jmp   $1298

1229: 68 f0     cmp   a,#$f0
122b: d0 d3     bne   $1200
122d: 8f 01 26  mov   $26,#$01
1230: ad 1e     cmp   y,#$1e
1232: b0 05     bcs   $1239
1234: 8f 00 28  mov   $28,#$00
1237: 2f 1f     bra   $1258
1239: 7e 27     cmp   y,$27
123b: f0 04     beq   $1241
123d: 7e 28     cmp   y,$28
123f: d0 04     bne   $1245
1241: e2 1f     set7  $1f
1243: 2f 08     bra   $124d
1245: e4 2a     mov   a,$2a
1247: 1c        asl   a
1248: 90 0e     bcc   $1258
124a: 1c        asl   a
124b: 90 1c     bcc   $1269
124d: 8f 80 29  mov   $29,#$80
1250: 8f fe 2e  mov   $2e,#$fe
1253: 8f 42 2f  mov   $2f,#$42
1256: 2f 40     bra   $1298
1258: cb 27     mov   $27,y
125a: e2 2a     set7  $2a
125c: 8f fe 2e  mov   $2e,#$fe
125f: 8f 42 2f  mov   $2f,#$42
1262: cd 02     mov   x,#$02
1264: 8f 43 04  mov   $04,#$43
1267: 2f 0f     bra   $1278
1269: cb 28     mov   $28,y
126b: c2 2a     set6  $2a
126d: 8f fe 2e  mov   $2e,#$fe
1270: 8f 51 2f  mov   $2f,#$51
1273: cd 06     mov   x,#$06
1275: 8f 52 04  mov   $04,#$52
1278: e8 02     mov   a,#$02
127a: cf        mul   ya
127b: da 06     movw  $06,ya
127d: e8 1e     mov   a,#$1e
127f: 8d 06     mov   y,#$06
1281: 7a 06     addw  ya,$06
1283: da 06     movw  $06,ya
1285: 8d 00     mov   y,#$00
1287: f7 06     mov   a,($06)+y
1289: d5 18 06  mov   $0618+x,a
128c: fc        inc   y
128d: f7 06     mov   a,($06)+y
128f: 60        clrc
1290: 84 04     adc   a,$04
1292: d5 19 06  mov   $0619+x,a
1295: 8f 05 29  mov   $29,#$05
1298: 8f ff f7  mov   $f7,#$ff
129b: 6f        ret

129c: e8 cc     mov   a,#$cc
129e: 64 f4     cmp   a,$f4
12a0: d0 fa     bne   $129c
12a2: 2f 1f     bra   $12c3
12a4: eb f4     mov   y,$f4
12a6: d0 fc     bne   $12a4
12a8: 7e f4     cmp   y,$f4
12aa: 30 13     bmi   $12bf
12ac: d0 fa     bne   $12a8
12ae: 7e f4     cmp   y,$f4
12b0: d0 f6     bne   $12a8
12b2: e4 f5     mov   a,$f5
12b4: cb f4     mov   $f4,y
12b6: d7 04     mov   ($04)+y,a
12b8: fc        inc   y
12b9: d0 ed     bne   $12a8
12bb: ab 05     inc   $05
12bd: 2f e9     bra   $12a8
12bf: 7e f4     cmp   y,$f4
12c1: 10 e5     bpl   $12a8
12c3: e4 f6     mov   a,$f6
12c5: eb f7     mov   y,$f7
12c7: da 04     movw  $04,ya
12c9: e4 f4     mov   a,$f4
12cb: eb f5     mov   y,$f5
12cd: c4 f4     mov   $f4,a
12cf: dd        mov   a,y
12d0: 5d        mov   x,a
12d1: d0 d1     bne   $12a4
12d3: 8f 33 f1  mov   $f1,#$33
12d6: 6f        ret

12d7: e4 f5     mov   a,$f5
12d9: 68 cc     cmp   a,#$cc
12db: d0 fa     bne   $12d7
12dd: 64 f5     cmp   a,$f5
12df: d0 f6     bne   $12d7
12e1: c4 f5     mov   $f5,a
12e3: eb f5     mov   y,$f5
12e5: d0 fc     bne   $12e3
12e7: 7e f5     cmp   y,$f5
12e9: d0 f6     bne   $12e1
12eb: 2f 0a     bra   $12f7
12ed: 7e f5     cmp   y,$f5
12ef: 30 18     bmi   $1309
12f1: d0 fa     bne   $12ed
12f3: 7e f5     cmp   y,$f5
12f5: d0 f6     bne   $12ed
12f7: e4 f6     mov   a,$f6
12f9: d7 2e     mov   ($2e)+y,a
12fb: e4 f7     mov   a,$f7
12fd: cb f5     mov   $f5,y
12ff: fc        inc   y
1300: d7 2e     mov   ($2e)+y,a
1302: fc        inc   y
1303: d0 e8     bne   $12ed
1305: ab 2f     inc   $2f
1307: 2f e4     bra   $12ed
1309: 7e f5     cmp   y,$f5
130b: 10 e0     bpl   $12ed
130d: e4 f5     mov   a,$f5
130f: c4 f5     mov   $f5,a
1311: e4 f5     mov   a,$f5
1313: d0 fc     bne   $1311
1315: c4 f5     mov   $f5,a
1317: 6f        ret

1318: c4 0c     mov   $0c,a
131a: 8f 00 0d  mov   $0d,#$00
131d: 68 49     cmp   a,#$49
131f: 90 03     bcc   $1324
1321: 5f a0 13  jmp   $13a0

1324: 8f 73 04  mov   $04,#$73
1327: 8f 0f 05  mov   $05,#$0f
132a: 8d 03     mov   y,#$03
132c: cf        mul   ya
132d: 7a 04     addw  ya,$04
132f: da 04     movw  $04,ya
1331: 8d 00     mov   y,#$00
1333: f7 04     mov   a,($04)+y
1335: 9f        xcn   a
1336: 5c        lsr   a
1337: 28 07     and   a,#$07
1339: 5d        mov   x,a
133a: bc        inc   a
133b: c4 07     mov   $07,a
133d: f5 5e 14  mov   a,$145e+x
1340: c4 11     mov   $11,a
1342: f7 04     mov   a,($04)+y
1344: 28 1f     and   a,#$1f
1346: c4 06     mov   $06,a
1348: cd 0c     mov   x,#$0c
134a: 78 02 07  cmp   $07,#$02
134d: f0 36     beq   $1385
134f: b0 4b     bcs   $139c
1351: e4 0c     mov   a,$0c
1353: 68 01     cmp   a,#$01
1355: d0 14     bne   $136b
1357: e4 0d     mov   a,$0d
1359: 65 af 01  cmp   a,$01af
135c: d0 06     bne   $1364
135e: cd 0e     mov   x,#$0e
1360: 0b 11     asl   $11
1362: 2f 29     bra   $138d
1364: 65 ad 01  cmp   a,$01ad
1367: f0 24     beq   $138d
1369: 2f 0e     bra   $1379
136b: 64 de     cmp   a,$de
136d: d0 06     bne   $1375
136f: cd 0e     mov   x,#$0e
1371: 0b 11     asl   $11
1373: 2f 18     bra   $138d
1375: 64 dc     cmp   a,$dc
1377: f0 14     beq   $138d
1379: f5 92 01  mov   a,$0192+x
137c: 75 90 01  cmp   a,$0190+x
137f: b0 04     bcs   $1385
1381: cd 0e     mov   x,#$0e
1383: 0b 11     asl   $11
1385: e4 06     mov   a,$06
1387: 75 90 01  cmp   a,$0190+x
138a: b0 01     bcs   $138d
138c: 6f        ret

138d: fc        inc   y
138e: f7 04     mov   a,($04)+y
1390: c4 0a     mov   $0a,a
1392: fc        inc   y
1393: f7 04     mov   a,($04)+y
1395: c4 0b     mov   $0b,a
1397: 8f 01 08  mov   $08,#$01
139a: 2f 4a     bra   $13e6
139c: cd 00     mov   x,#$00
139e: 2f ed     bra   $138d
13a0: a8 49     sbc   a,#$49
13a2: fd        mov   y,a
13a3: f6 4e 10  mov   a,$104e+y
13a6: c4 25     mov   $25,a
13a8: 3f f4 1e  call  $1ef4
13ab: e4 0c     mov   a,$0c
13ad: 68 58     cmp   a,#$58
13af: 90 1a     bcc   $13cb
13b1: 68 5a     cmp   a,#$5a
13b3: 90 10     bcc   $13c5
13b5: 68 a6     cmp   a,#$a6
13b7: 90 06     bcc   $13bf
13b9: e8 05     mov   a,#$05
13bb: 8d 1f     mov   y,#$1f
13bd: 2f 10     bra   $13cf
13bf: e8 06     mov   a,#$06
13c1: 8d 3f     mov   y,#$3f
13c3: 2f 0a     bra   $13cf
13c5: e8 07     mov   a,#$07
13c7: 8d 7f     mov   y,#$7f
13c9: 2f 04     bra   $13cf
13cb: e8 08     mov   a,#$08
13cd: 8d ff     mov   y,#$ff
13cf: c4 07     mov   $07,a
13d1: cb 11     mov   $11,y
13d3: 8f 1e 06  mov   $06,#$1e
13d6: 8f 00 0a  mov   $0a,#$00
13d9: 8f 39 0b  mov   $0b,#$39          ; set header address $3900 to $0a/b
13dc: cd 00     mov   x,#$00
13de: d8 1c     mov   $1c,x
13e0: d8 08     mov   $08,x
13e2: d8 2c     mov   $2c,x
13e4: d8 2d     mov   $2d,x
13e6: 8d 00     mov   y,#$00
; repeat for ($07) times
13e8: f7 0a     mov   a,($0a)+y
13ea: d4 30     mov   $30+x,a
13ec: fc        inc   y
13ed: f7 0a     mov   a,($0a)+y
13ef: d4 31     mov   $31+x,a           ; set vcmd ptr to $30/1+x
13f1: e8 40     mov   a,#$40
13f3: d4 41     mov   $41+x,a
13f5: e8 c0     mov   a,#$c0
13f7: d4 40     mov   $40+x,a
13f9: e8 00     mov   a,#$00
13fb: d4 50     mov   $50+x,a
13fd: d4 51     mov   $51+x,a
13ff: d5 10 01  mov   $0110+x,a
1402: d5 11 01  mov   $0111+x,a
1405: d5 20 01  mov   $0120+x,a
1408: d5 21 01  mov   $0121+x,a
140b: d5 30 01  mov   $0130+x,a
140e: d5 31 01  mov   $0131+x,a
1411: d5 40 01  mov   $0140+x,a
1414: d5 41 01  mov   $0141+x,a
1417: d5 80 02  mov   $0280+x,a
141a: d5 81 02  mov   $0281+x,a
141d: d5 21 02  mov   $0221+x,a
1420: d4 90     mov   $90+x,a
1422: d5 20 02  mov   $0220+x,a
1425: d5 d1 01  mov   $01d1+x,a
1428: d5 31 02  mov   $0231+x,a
142b: bc        inc   a
142c: d4 60     mov   $60+x,a
142e: e4 0c     mov   a,$0c
1430: d4 d0     mov   $d0+x,a
1432: e4 0d     mov   a,$0d
1434: d5 a1 01  mov   $01a1+x,a
1437: e4 08     mov   a,$08
1439: d5 91 01  mov   $0191+x,a
143c: e4 06     mov   a,$06
143e: d5 90 01  mov   $0190+x,a
1441: e4 09     mov   a,$09
1443: d5 a0 01  mov   $01a0+x,a
1446: 6e 07 0f  dbnz  $07,$1458
1449: e4 11     mov   a,$11
144b: 0e 10 00  tset1 $0010
144e: 4e 0e 00  tclr1 $000e
1451: 4e 0f 00  tclr1 $000f
1454: 0e 1b 00  tset1 $001b
1457: 6f        ret
; repeat continue
1458: 3d        inc   x
1459: 3d        inc   x
145a: fc        inc   y
145b: 5f e8 13  jmp   $13e8

145e: db $40,$c0,$70,$f0,$1f,$3f,$7f,$ff

1466: e4 fd     mov   a,$fd
1468: f0 fc     beq   $1466
146a: 03 24 16  bbs0  $24,$1483
146d: e4 19     mov   a,$19
146f: d0 12     bne   $1483
1471: e4 f4     mov   a,$f4
1473: 64 f4     cmp   a,$f4
1475: d0 fa     bne   $1471
1477: 68 00     cmp   a,#$00
1479: f0 08     beq   $1483
147b: c4 f4     mov   $f4,a
147d: 8f 11 f1  mov   $f1,#$11
1480: 3f af 11  call  $11af
1483: f3 1f 07  bbc7  $1f,$148d
1486: f2 1f     clr7  $1f
1488: e4 2b     mov   a,$2b
148a: 3f d6 11  call  $11d6
148d: 8f 5c f2  mov   $f2,#$5c
1490: fa 10 f3  mov   ($f3),($10)       ; set KOF
1493: 8d ff     mov   y,#$ff
1495: e4 26     mov   a,$26
1497: d0 34     bne   $14cd
1499: e5 ad 01  mov   a,$01ad
149c: f0 04     beq   $14a2
149e: 68 1e     cmp   a,#$1e
14a0: 90 2b     bcc   $14cd
14a2: e5 af 01  mov   a,$01af
14a5: f0 04     beq   $14ab
14a7: 68 1e     cmp   a,#$1e
14a9: 90 22     bcc   $14cd
14ab: e4 27     mov   a,$27
14ad: f0 0a     beq   $14b9
14af: 65 ad 01  cmp   a,$01ad
14b2: f0 07     beq   $14bb
14b4: 65 af 01  cmp   a,$01af
14b7: f0 02     beq   $14bb
14b9: f2 2a     clr7  $2a
14bb: e4 28     mov   a,$28
14bd: f0 0a     beq   $14c9
14bf: 65 ad 01  cmp   a,$01ad
14c2: f0 07     beq   $14cb
14c4: 65 af 01  cmp   a,$01af
14c7: f0 02     beq   $14cb
14c9: d2 2a     clr6  $2a
14cb: eb 2a     mov   y,$2a
14cd: cb f7     mov   $f7,y
14cf: e8 10     mov   a,#$10
14d1: 9c        dec   a
14d2: d0 fd     bne   $14d1
14d4: 8f 2d f2  mov   $f2,#$2d
14d7: c4 f3     mov   $f3,a             ; set PMON
14d9: 8f 3d f2  mov   $f2,#$3d
14dc: c4 f3     mov   $f3,a             ; set NON
14de: e4 19     mov   a,$19
14e0: f0 04     beq   $14e6
14e2: 8b 19     dec   $19
14e4: 2f 20     bra   $1506
14e6: 8f 6c f2  mov   $f2,#$6c
14e9: fa 13 f3  mov   ($f3),($13)       ; set FLG
14ec: e4 1a     mov   a,$1a
14ee: f0 16     beq   $1506
14f0: 8b 1a     dec   $1a
14f2: d0 12     bne   $1506
14f4: 8f 2c f2  mov   $f2,#$2c
14f7: fa 15 f3  mov   ($f3),($15)       ; set EVOL(L)
14fa: 8f 3c f2  mov   $f2,#$3c
14fd: fa 16 f3  mov   ($f3),($16)       ; set EVOL(R)
1500: 8f 0d f2  mov   $f2,#$0d
1503: fa 17 f3  mov   ($f3),($17)       ; set EFB
1506: e8 00     mov   a,#$00
1508: 8f 5c f2  mov   $f2,#$5c
150b: c4 f3     mov   $f3,a             ; set KOF
150d: c4 10     mov   $10,a
150f: 8f 4c f2  mov   $f2,#$4c
1512: fa 0e f3  mov   ($f3),($0e)       ; set KOL
1515: c4 0e     mov   $0e,a
1517: 33 1f 07  bbc1  $1f,$1521
151a: cd 0c     mov   x,#$0c
151c: 8f 40 11  mov   $11,#$40
151f: 2f 3f     bra   $1560
1521: e4 2d     mov   a,$2d
1523: f0 1b     beq   $1540
1525: 68 ff     cmp   a,#$ff
1527: d0 09     bne   $1532
1529: 8b 2c     dec   $2c
152b: d0 13     bne   $1540
152d: 8f 00 2d  mov   $2d,#$00
1530: 2f 0e     bra   $1540
1532: 8b 2d     dec   $2d
1534: d0 0a     bne   $1540
1536: ab 2c     inc   $2c
1538: 78 28 2c  cmp   $2c,#$28
153b: b0 03     bcs   $1540
153d: 8f 08 2d  mov   $2d,#$08
1540: ab 24     inc   $24
1542: e4 1c     mov   a,$1c
1544: f0 15     beq   $155b
1546: 8b 1d     dec   $1d
1548: d0 11     bne   $155b
154a: fa 1e 1d  mov   ($1d),($1e)
154d: 38 7f 1d  and   $1d,#$7f
1550: 8b 1c     dec   $1c
1552: f0 04     beq   $1558
1554: 8b 1c     dec   $1c
1556: d0 03     bne   $155b
1558: 3f 22 11  call  $1122
155b: 8f 01 11  mov   $11,#$01
155e: cd 00     mov   x,#$00
1560: f4 d0     mov   a,$d0+x
1562: f0 5b     beq   $15bf
1564: e4 11     mov   a,$11
1566: 24 1b     and   a,$1b
1568: d0 55     bne   $15bf
156a: f5 91 01  mov   a,$0191+x
156d: 5c        lsr   a
156e: b0 08     bcs   $1578
1570: 7d        mov   a,x
1571: 5c        lsr   a
1572: 44 24     eor   a,$24
1574: 28 01     and   a,#$01
1576: d0 47     bne   $15bf
1578: d8 22     mov   $22,x
157a: 7d        mov   a,x
157b: 1c        asl   a
157c: 1c        asl   a
157d: 1c        asl   a
157e: c4 23     mov   $23,a
1580: f5 91 01  mov   a,$0191+x
1583: c4 20     mov   $20,a
1585: f4 40     mov   a,$40+x
1587: 03 20 0c  bbs0  $20,$1596
158a: 60        clrc
158b: 84 2c     adc   a,$2c
158d: 90 07     bcc   $1596
158f: 60        clrc
1590: 94 41     adc   a,$41+x
1592: d4 40     mov   $40+x,a
1594: 2f 07     bra   $159d
1596: 60        clrc
1597: 94 41     adc   a,$41+x
1599: d4 40     mov   $40+x,a
159b: 90 1a     bcc   $15b7
159d: 9b 60     dec   $60+x             ; decrease wait counter (tick)
159f: f0 05     beq   $15a6
15a1: 3f 3d 17  call  $173d
15a4: 2f 14     bra   $15ba
15a6: 3f e2 18  call  $18e2
15a9: a3 20 07  bbs5  $20,$15b3
15ac: e2 21     set7  $21
15ae: 3f fa 17  call  $17fa
15b1: 2f 07     bra   $15ba
15b3: b2 20     clr5  $20
15b5: 2f 03     bra   $15ba
15b7: 3f f1 16  call  $16f1
15ba: e4 20     mov   a,$20
15bc: d5 91 01  mov   $0191+x,a
15bf: e4 26     mov   a,$26
15c1: f0 20     beq   $15e3
15c3: e4 f5     mov   a,$f5
15c5: 68 cc     cmp   a,#$cc
15c7: d0 1a     bne   $15e3
15c9: 64 f5     cmp   a,$f5
15cb: d0 16     bne   $15e3
15cd: 4d        push  x
15ce: e3 26 06  bbs7  $26,$15d7
15d1: 3f 74 16  call  $1674
15d4: ce        pop   x
15d5: 2f 1d     bra   $15f4
15d7: fa 11 12  mov   ($12),($11)
15da: 3f 27 16  call  $1627
15dd: fa 12 11  mov   ($11),($12)
15e0: ce        pop   x
15e1: 2f 11     bra   $15f4
15e3: e4 f5     mov   a,$f5
15e5: 68 f0     cmp   a,#$f0
15e7: 90 0b     bcc   $15f4
15e9: 64 f5     cmp   a,$f5
15eb: d0 07     bne   $15f4
15ed: c4 f5     mov   $f5,a
15ef: 4d        push  x
15f0: 3f 01 12  call  $1201
15f3: ce        pop   x
15f4: 3d        inc   x
15f5: 3d        inc   x
15f6: 0b 11     asl   $11
15f8: f0 03     beq   $15fd
15fa: 5f 60 15  jmp   $1560

15fd: 8f 00 1b  mov   $1b,#$00
1600: e4 26     mov   a,$26
1602: d0 03     bne   $1607
1604: 5f 66 14  jmp   $1466

1607: e4 f5     mov   a,$f5
1609: 68 cc     cmp   a,#$cc
160b: f0 07     beq   $1614
160d: e4 fd     mov   a,$fd
160f: f0 f6     beq   $1607
1611: 5f 6a 14  jmp   $146a

1614: 64 f5     cmp   a,$f5
1616: d0 ef     bne   $1607
1618: e3 26 06  bbs7  $26,$1621
161b: 3f 74 16  call  $1674
161e: 5f 66 14  jmp   $1466

1621: 3f 27 16  call  $1627
1624: 5f 66 14  jmp   $1466

1627: c4 f5     mov   $f5,a
1629: eb f5     mov   y,$f5
162b: d0 fc     bne   $1629
162d: 7e f5     cmp   y,$f5
162f: d0 f6     bne   $1627
1631: 2f 0a     bra   $163d
1633: 7e f5     cmp   y,$f5
1635: 30 25     bmi   $165c
1637: d0 fa     bne   $1633
1639: 7e f5     cmp   y,$f5
163b: d0 f6     bne   $1633
163d: e4 f6     mov   a,$f6
163f: 64 f6     cmp   a,$f6
1641: d0 fa     bne   $163d
1643: d7 2e     mov   ($2e)+y,a
1645: e4 f7     mov   a,$f7
1647: 64 f7     cmp   a,$f7
1649: d0 fa     bne   $1645
164b: cb f5     mov   $f5,y
164d: fc        inc   y
164e: d7 2e     mov   ($2e)+y,a
1650: fc        inc   y
1651: d0 e0     bne   $1633
1653: ab 2f     inc   $2f
1655: e4 f5     mov   a,$f5
1657: d0 fc     bne   $1655
1659: c4 f5     mov   $f5,a
165b: 6f        ret

165c: 7e f5     cmp   y,$f5
165e: 10 d3     bpl   $1633
1660: e4 f5     mov   a,$f5
1662: c4 f5     mov   $f5,a
1664: e4 f5     mov   a,$f5
1666: d0 fc     bne   $1664
1668: c4 f5     mov   $f5,a
166a: c4 26     mov   $26,a
166c: 8f 00 f7  mov   $f7,#$00
166f: e4 2b     mov   a,$2b
1671: 5f 18 13  jmp   $1318

1674: c4 f5     mov   $f5,a
1676: eb f4     mov   y,$f4
1678: ad 02     cmp   y,#$02
167a: d0 fa     bne   $1676
167c: 7e f4     cmp   y,$f4
167e: d0 f4     bne   $1674
1680: 8f 00 f5  mov   $f5,#$00
1683: f3 29 08  bbc7  $29,$168e
1686: 2f 3e     bra   $16c6
1688: 7e f4     cmp   y,$f4
168a: 30 49     bmi   $16d5
168c: d0 fa     bne   $1688
168e: e4 f5     mov   a,$f5
1690: d7 2e     mov   ($2e)+y,a
1692: fc        inc   y
1693: e4 f6     mov   a,$f6
1695: d7 2e     mov   ($2e)+y,a
1697: e4 f7     mov   a,$f7
1699: dc        dec   y
169a: cb f4     mov   $f4,y
169c: fc        inc   y
169d: fc        inc   y
169e: d7 2e     mov   ($2e)+y,a
16a0: fc        inc   y
16a1: 10 e5     bpl   $1688
16a3: 60        clrc
16a4: 98 7e 2e  adc   $2e,#$7e
16a7: 98 00 2f  adc   $2f,#$00
16aa: e4 f4     mov   a,$f4
16ac: d0 fc     bne   $16aa
16ae: c4 f4     mov   $f4,a
16b0: e4 29     mov   a,$29
16b2: 30 0b     bmi   $16bf
16b4: f0 09     beq   $16bf
16b6: 8b 29     dec   $29
16b8: d0 05     bne   $16bf
16ba: 23 1f 02  bbs1  $1f,$16bf
16bd: e2 1f     set7  $1f
16bf: 6f        ret

16c0: 7e f4     cmp   y,$f4
16c2: 30 0b     bmi   $16cf
16c4: d0 fa     bne   $16c0
16c6: cb f4     mov   $f4,y
16c8: fc        inc   y
16c9: fc        inc   y
16ca: fc        inc   y
16cb: 10 f3     bpl   $16c0
16cd: 2f d4     bra   $16a3
16cf: 7e f4     cmp   y,$f4
16d1: 10 ed     bpl   $16c0
16d3: 2f 04     bra   $16d9
16d5: 7e f4     cmp   y,$f4
16d7: 10 af     bpl   $1688
16d9: e4 f4     mov   a,$f4
16db: c4 f4     mov   $f4,a
16dd: e4 f4     mov   a,$f4
16df: d0 fc     bne   $16dd
16e1: c4 f4     mov   $f4,a
16e3: c4 26     mov   $26,a
16e5: e4 29     mov   a,$29
16e7: 30 07     bmi   $16f0
16e9: f0 05     beq   $16f0
16eb: 23 1f 02  bbs1  $1f,$16f0
16ee: e2 1f     set7  $1f
16f0: 6f        ret

16f1: 12 21     clr0  $21
16f3: f4 80     mov   a,$80+x
16f5: f0 1d     beq   $1714
16f7: 93 20 05  bbc4  $20,$16ff
16fa: 3f 75 20  call  $2075
16fd: 2f 13     bra   $1712
16ff: f4 81     mov   a,$81+x
1701: d0 11     bne   $1714
1703: f4 e0     mov   a,$e0+x
1705: 60        clrc
1706: 95 b0 02  adc   a,$02b0+x
1709: d4 e0     mov   $e0+x,a
170b: f4 e1     mov   a,$e1+x
170d: 95 b1 02  adc   a,$02b1+x
1710: d4 e1     mov   $e1+x,a
1712: 02 21     set0  $21
1714: fb e1     mov   y,$e1+x
1716: f4 e0     mov   a,$e0+x
1718: da 0a     movw  $0a,ya
171a: f5 20 02  mov   a,$0220+x
171d: f0 07     beq   $1726
171f: f4 a0     mov   a,$a0+x
1721: d0 03     bne   $1726
1723: 3f bb 20  call  $20bb
1726: f5 d1 01  mov   a,$01d1+x
1729: f0 0a     beq   $1735
172b: 60        clrc
172c: 94 d1     adc   a,$d1+x
172e: d4 d1     mov   $d1+x,a
1730: 90 03     bcc   $1735
1732: 5f ff 20  jmp   $20ff

1735: 13 21 b8  bbc0  $21,$16f0
1738: ba 0a     movw  ya,$0a
173a: 5f 7f 1a  jmp   $1a7f

173d: 8f 00 21  mov   $21,#$00
1740: f5 31 02  mov   a,$0231+x
1743: 68 7f     cmp   a,#$7f
1745: b0 2f     bcs   $1776
1747: fd        mov   y,a
1748: f4 60     mov   a,$60+x
174a: 68 01     cmp   a,#$01
174c: f0 25     beq   $1773
174e: ad 7d     cmp   y,#$7d
1750: b0 24     bcs   $1776
1752: f4 61     mov   a,$61+x
1754: f0 20     beq   $1776
1756: 9b 61     dec   $61+x
1758: d0 1c     bne   $1776
175a: f5 c1 01  mov   a,$01c1+x
175d: 10 14     bpl   $1773
175f: e8 05     mov   a,#$05
1761: 04 23     or    a,$23
1763: c4 f2     mov   $f2,a
1765: 8f 00 f3  mov   $f3,#$00          ; set ADSR(1)
1768: bc        inc   a
1769: bc        inc   a
176a: c4 f2     mov   $f2,a
176c: f5 c1 01  mov   a,$01c1+x
176f: c4 f3     mov   $f3,a             ; set GAIN
1771: 2f 03     bra   $1776
1773: 09 11 10  or    ($10),($11)
1776: f4 80     mov   a,$80+x
1778: f0 31     beq   $17ab
177a: 93 20 05  bbc4  $20,$1782
177d: 3f 75 20  call  $2075
1780: 2f 27     bra   $17a9
1782: f4 81     mov   a,$81+x
1784: f0 04     beq   $178a
1786: 9b 81     dec   $81+x
1788: 2f 21     bra   $17ab
178a: 9b 80     dec   $80+x
178c: d0 0c     bne   $179a
178e: f5 c1 02  mov   a,$02c1+x
1791: d4 e1     mov   $e1+x,a
1793: f5 c0 02  mov   a,$02c0+x
1796: d4 e0     mov   $e0+x,a
1798: 2f 0f     bra   $17a9
179a: f4 e0     mov   a,$e0+x
179c: 60        clrc
179d: 95 b0 02  adc   a,$02b0+x
17a0: d4 e0     mov   $e0+x,a
17a2: f4 e1     mov   a,$e1+x
17a4: 95 b1 02  adc   a,$02b1+x
17a7: d4 e1     mov   $e1+x,a
17a9: 02 21     set0  $21
17ab: fb e1     mov   y,$e1+x
17ad: f4 e0     mov   a,$e0+x
17af: da 0a     movw  $0a,ya
17b1: f5 20 02  mov   a,$0220+x
17b4: f0 2b     beq   $17e1
17b6: f4 a0     mov   a,$a0+x
17b8: f0 04     beq   $17be
17ba: 9b a0     dec   $a0+x
17bc: 2f 23     bra   $17e1
17be: f4 a1     mov   a,$a1+x
17c0: f0 1c     beq   $17de
17c2: 9b a1     dec   $a1+x
17c4: d0 05     bne   $17cb
17c6: f5 20 02  mov   a,$0220+x
17c9: 2f 10     bra   $17db
17cb: f5 d0 02  mov   a,$02d0+x
17ce: 60        clrc
17cf: 95 e0 02  adc   a,$02e0+x
17d2: d5 d0 02  mov   $02d0+x,a
17d5: f5 d1 02  mov   a,$02d1+x
17d8: 95 e1 02  adc   a,$02e1+x
17db: d5 d1 02  mov   $02d1+x,a
17de: 3f bb 20  call  $20bb
17e1: f5 d1 01  mov   a,$01d1+x
17e4: f0 0c     beq   $17f2
17e6: 60        clrc
17e7: 94 d1     adc   a,$d1+x
17e9: d4 d1     mov   $d1+x,a
17eb: 90 05     bcc   $17f2
17ed: 3f ff 20  call  $20ff
17f0: 2f 08     bra   $17fa
17f2: 13 21 05  bbc0  $21,$17fa
17f5: ba 0a     movw  ya,$0a
17f7: 3f 7f 1a  call  $1a7f
17fa: f4 70     mov   a,$70+x
17fc: f0 1e     beq   $181c
17fe: e2 21     set7  $21
1800: c4 04     mov   $04,a
1802: f5 01 02  mov   a,$0201+x
1805: c4 05     mov   $05,a
1807: f5 a1 02  mov   a,$02a1+x
180a: fd        mov   y,a
180b: f5 a0 02  mov   a,$02a0+x
180e: 3f 46 20  call  $2046
1811: d5 a0 02  mov   $02a0+x,a
1814: dd        mov   a,y
1815: d5 a1 02  mov   $02a1+x,a
1818: e4 04     mov   a,$04
181a: d4 70     mov   $70+x,a
181c: f5 c0 01  mov   a,$01c0+x
181f: f0 19     beq   $183a
1821: c4 04     mov   $04,a
1823: f5 50 02  mov   a,$0250+x
1826: c4 05     mov   $05,a
1828: fb 41     mov   y,$41+x
182a: f5 51 02  mov   a,$0251+x
182d: 3f 46 20  call  $2046
1830: d5 51 02  mov   $0251+x,a
1833: db 41     mov   $41+x,y
1835: e4 04     mov   a,$04
1837: d5 c0 01  mov   $01c0+x,a
183a: f4 71     mov   a,$71+x
183c: f0 1e     beq   $185c
183e: e2 21     set7  $21
1840: c4 04     mov   $04,a
1842: f5 00 02  mov   a,$0200+x
1845: c4 05     mov   $05,a
1847: f5 91 02  mov   a,$0291+x
184a: fd        mov   y,a
184b: f5 90 02  mov   a,$0290+x
184e: 3f 46 20  call  $2046
1851: d5 90 02  mov   $0290+x,a
1854: dd        mov   a,y
1855: d5 91 02  mov   $0291+x,a
1858: e4 04     mov   a,$04
185a: d4 71     mov   $71+x,a
185c: e3 21 05  bbs7  $21,$1864
185f: e4 1c     mov   a,$1c
1861: d0 01     bne   $1864
1863: 6f        ret

1864: f5 b1 01  mov   a,$01b1+x
1867: f0 fa     beq   $1863
1869: f5 10 01  mov   a,$0110+x
186c: c4 08     mov   $08,a
186e: f5 11 01  mov   a,$0111+x
1871: c4 09     mov   $09,a
1873: f5 20 01  mov   a,$0120+x
1876: c4 0a     mov   $0a,a
1878: f5 21 01  mov   a,$0121+x
187b: c4 0b     mov   $0b,a
187d: f5 b1 01  mov   a,$01b1+x
1880: 8d 00     mov   y,#$00
1882: 7a 08     addw  ya,$08
1884: 7a 0a     addw  ya,$0a
1886: 30 21     bmi   $18a9
1888: ad 00     cmp   y,#$00
188a: d0 04     bne   $1890
188c: 08 00     or    a,#$00
188e: 10 02     bpl   $1892
1890: e8 7f     mov   a,#$7f
1892: fd        mov   y,a
1893: f5 91 02  mov   a,$0291+x
1896: cf        mul   ya
1897: e4 1c     mov   a,$1c
1899: f0 07     beq   $18a2
189b: e3 1e 03  bbs7  $1e,$18a1
189e: 03 20 01  bbs0  $20,$18a2
18a1: cf        mul   ya
18a2: dd        mov   a,y
18a3: 80        setc
18a4: b5 b0 01  sbc   a,$01b0+x
18a7: b0 02     bcs   $18ab
18a9: e8 00     mov   a,#$00
18ab: fd        mov   y,a
18ac: f6 1d 22  mov   a,$221d+y
18af: c4 05     mov   $05,a
18b1: e8 14     mov   a,#$14
18b3: 03 1f 09  bbs0  $1f,$18bf
18b6: f5 a1 02  mov   a,$02a1+x
18b9: 68 29     cmp   a,#$29
18bb: 90 02     bcc   $18bf
18bd: e8 28     mov   a,#$28
18bf: c4 04     mov   $04,a
18c1: fd        mov   y,a
18c2: f6 bd 22  mov   a,$22bd+y
18c5: eb 05     mov   y,$05
18c7: cf        mul   ya
18c8: e8 00     mov   a,#$00
18ca: 04 23     or    a,$23
18cc: c4 f2     mov   $f2,a
18ce: cb f3     mov   $f3,y             ; set VOL(L)
18d0: bc        inc   a
18d1: c4 f2     mov   $f2,a
18d3: e8 28     mov   a,#$28
18d5: 80        setc
18d6: a4 04     sbc   a,$04
18d8: fd        mov   y,a
18d9: f6 bd 22  mov   a,$22bd+y
18dc: eb 05     mov   y,$05
18de: cf        mul   ya
18df: cb f3     mov   $f3,y             ; set VOL(R)
18e1: 6f        ret

18e2: 3f 56 19  call  $1956             ; read a byte
18e5: c4 08     mov   $08,a
18e7: 28 7f     and   a,#$7f
18e9: 68 60     cmp   a,#$60
18eb: 90 72     bcc   $195f             ; 00-5f, 80-df
18ed: e4 08     mov   a,$08
18ef: 80        setc
18f0: a8 e0     sbc   a,#$e0
18f2: b0 53     bcs   $1947             ; e0-ff - vcmd
18f4: 68 82     cmp   a,#$82
18f6: b0 09     bcs   $1901             ; 62-7f
; vcmd 60,61
18f8: 22 20     set1  $20
18fa: 5c        lsr   a
18fb: 90 e5     bcc   $18e2
18fd: 32 20     clr1  $20
18ff: 2f e1     bra   $18e2
; (62-7f)
1901: d0 03     bne   $1906             ; 63-7f
1903: 5f ab 1f  jmp   $1fab

; vcmd 63-7f
1906: 68 85     cmp   a,#$85
1908: b0 19     bcs   $1923
190a: 68 83     cmp   a,#$83
190c: d0 05     bne   $1913
190e: 09 11 14  or    ($14),($11)
1911: 2f 08     bra   $191b
1913: e8 ff     mov   a,#$ff
1915: 44 11     eor   a,$11
1917: 24 14     and   a,$14
1919: c4 14     mov   $14,a
191b: 8f 4d f2  mov   $f2,#$4d
191e: fa 14 f3  mov   ($f3),($14)       ; set EON
1921: 2f bf     bra   $18e2
1923: 68 90     cmp   a,#$90
1925: 90 16     bcc   $193d
1927: 28 0f     and   a,#$0f
1929: 1c        asl   a
192a: 1c        asl   a
192b: 8d 00     mov   y,#$00
192d: 68 20     cmp   a,#$20
192f: 90 03     bcc   $1934
1931: 08 e0     or    a,#$e0
1933: dc        dec   y
1934: d5 80 02  mov   $0280+x,a
1937: dd        mov   a,y
1938: d5 81 02  mov   $0281+x,a
193b: 2f a5     bra   $18e2
193d: a2 20     set5  $20
193f: 3f 6c 11  call  $116c
1942: cd 00     mov   x,#$00
1944: d8 11     mov   $11,x
1946: 6f        ret

; dispatch vcmd (e0-ff)
1947: 1c        asl   a
1948: fd        mov   y,a
1949: f6 bc 1a  mov   a,$1abc+y
194c: 2d        push  a
194d: f6 bb 1a  mov   a,$1abb+y
1950: 2d        push  a                 ; push vcmd func address, as a return address
1951: f6 fb 1a  mov   a,$1afb+y
1954: f0 08     beq   $195e
; read next byte (vcmd/arg)
1956: e7 30     mov   a,($30+x)
1958: bb 30     inc   $30+x
195a: d0 02     bne   $195e
195c: bb 31     inc   $31+x
195e: 6f        ret

; vcmd 00-5f, 80-df ($08 = vcmd)
195f: 33 20 03  bbc1  $20,$1965
1962: 09 11 10  or    ($10),($11)
1965: f3 08 05  bbc7  $08,$196d
1968: f5 30 02  mov   a,$0230+x
196b: 2f 06     bra   $1973
196d: 3f 56 19  call  $1956             ; arg1 - length (tick)
1970: d5 30 02  mov   $0230+x,a
1973: d4 60     mov   $60+x,a
1975: f5 31 02  mov   a,$0231+x
1978: 68 7f     cmp   a,#$7f
197a: f0 06     beq   $1982
197c: 09 11 0e  or    ($0e),($11)
197f: 09 11 0f  or    ($0f),($11)
1982: 3f 56 19  call  $1956             ; arg2
1985: 08 00     or    a,#$00
1987: 30 06     bmi   $198f
1989: d5 31 02  mov   $0231+x,a
198c: 3f 56 19  call  $1956             ; arg3 (only available if arg2 < 0x80)
198f: 28 7f     and   a,#$7f
1991: d5 b1 01  mov   $01b1+x,a
1994: d0 08     bne   $199e
1996: e4 11     mov   a,$11
1998: 4e 0e 00  tclr1 $000e
199b: 4e 0f 00  tclr1 $000f
199e: 33 20 0a  bbc1  $20,$19ab
19a1: e4 08     mov   a,$08
19a3: 28 7f     and   a,#$7f
19a5: 3f e8 1b  call  $1be8
19a8: 8f 3c 08  mov   $08,#$3c
19ab: f5 30 02  mov   a,$0230+x
19ae: fd        mov   y,a
19af: f5 31 02  mov   a,$0231+x
19b2: 68 7f     cmp   a,#$7f
19b4: f0 06     beq   $19bc
19b6: 1c        asl   a
19b7: cf        mul   ya
19b8: dd        mov   a,y
19b9: d0 01     bne   $19bc
19bb: fc        inc   y
19bc: db 61     mov   $61+x,y
19be: fb e1     mov   y,$e1+x
19c0: f4 e0     mov   a,$e0+x
19c2: da 0c     movw  $0c,ya
19c4: f5 31 01  mov   a,$0131+x
19c7: c4 05     mov   $05,a
19c9: f5 30 01  mov   a,$0130+x
19cc: c4 04     mov   $04,a
19ce: f5 41 01  mov   a,$0141+x
19d1: c4 07     mov   $07,a
19d3: f5 40 01  mov   a,$0140+x
19d6: c4 06     mov   $06,a
19d8: f5 81 02  mov   a,$0281+x
19db: c4 0b     mov   $0b,a
19dd: f5 80 02  mov   a,$0280+x
19e0: c4 0a     mov   $0a,a
19e2: e4 08     mov   a,$08
19e4: 60        clrc
19e5: 95 21 02  adc   a,$0221+x
19e8: fd        mov   y,a
19e9: e8 00     mov   a,#$00
19eb: 7a 04     addw  ya,$04
19ed: 7a 06     addw  ya,$06
19ef: 7a 0a     addw  ya,$0a
19f1: d4 e0     mov   $e0+x,a
19f3: dd        mov   a,y
19f4: 28 7f     and   a,#$7f
19f6: d4 e1     mov   $e1+x,a
19f8: f5 20 02  mov   a,$0220+x
19fb: f0 26     beq   $1a23
19fd: e8 00     mov   a,#$00
19ff: d4 b0     mov   $b0+x,a
1a01: d4 c1     mov   $c1+x,a
1a03: f5 10 02  mov   a,$0210+x
1a06: d4 a0     mov   $a0+x,a
1a08: f5 11 02  mov   a,$0211+x
1a0b: d4 a1     mov   $a1+x,a
1a0d: f0 0b     beq   $1a1a
1a0f: f5 e0 02  mov   a,$02e0+x
1a12: d5 d0 02  mov   $02d0+x,a
1a15: f5 e1 02  mov   a,$02e1+x
1a18: 2f 06     bra   $1a20
1a1a: d5 d0 02  mov   $02d0+x,a
1a1d: f5 20 02  mov   a,$0220+x
1a20: d5 d1 02  mov   $02d1+x,a
1a23: f5 c1 01  mov   a,$01c1+x
1a26: 10 18     bpl   $1a40
1a28: e8 05     mov   a,#$05
1a2a: 04 23     or    a,$23
1a2c: c4 f2     mov   $f2,a
1a2e: f5 70 02  mov   a,$0270+x
1a31: c4 f3     mov   $f3,a             ; set ADSR(1)
1a33: 30 0b     bmi   $1a40
1a35: e8 07     mov   a,#$07
1a37: 04 23     or    a,$23
1a39: c4 f2     mov   $f2,a
1a3b: f5 71 02  mov   a,$0271+x
1a3e: c4 f3     mov   $f3,a             ; set GAIN
1a40: f4 90     mov   a,$90+x
1a42: d4 80     mov   $80+x,a
1a44: f0 2c     beq   $1a72
1a46: 83 20 17  bbs4  $20,$1a60
1a49: f4 91     mov   a,$91+x
1a4b: d4 81     mov   $81+x,a
1a4d: f4 e0     mov   a,$e0+x
1a4f: d5 c0 02  mov   $02c0+x,a
1a52: f4 e1     mov   a,$e1+x
1a54: d5 c1 02  mov   $02c1+x,a
1a57: 80        setc
1a58: b5 60 02  sbc   a,$0260+x
1a5b: d4 e1     mov   $e1+x,a
1a5d: fd        mov   y,a
1a5e: 2f 1d     bra   $1a7d
1a60: f4 e1     mov   a,$e1+x
1a62: d5 c1 02  mov   $02c1+x,a
1a65: f4 e0     mov   a,$e0+x
1a67: d5 c0 02  mov   $02c0+x,a
1a6a: ba 0c     movw  ya,$0c
1a6c: db e1     mov   $e1+x,y
1a6e: d4 e0     mov   $e0+x,a
1a70: 2f 0d     bra   $1a7f
1a72: e7 30     mov   a,($30+x)
1a74: 68 f3     cmp   a,#$f3
1a76: d0 03     bne   $1a7b
1a78: 3f 6a 1b  call  $1b6a
1a7b: fb e1     mov   y,$e1+x
1a7d: f4 e0     mov   a,$e0+x
1a7f: da 04     movw  $04,ya
1a81: f5 40 02  mov   a,$0240+x
1a84: fd        mov   y,a
1a85: f5 41 02  mov   a,$0241+x
1a88: 7a 04     addw  ya,$04
1a8a: c4 04     mov   $04,a
1a8c: dd        mov   a,y
1a8d: 1c        asl   a
1a8e: fd        mov   y,a
1a8f: f6 00 03  mov   a,$0300+y
1a92: c4 06     mov   $06,a
1a94: f6 01 03  mov   a,$0301+y
1a97: c4 07     mov   $07,a
1a99: f6 1e 21  mov   a,$211e+y
1a9c: 2d        push  a
1a9d: f6 1d 21  mov   a,$211d+y
1aa0: eb 04     mov   y,$04
1aa2: cf        mul   ya
1aa3: ae        pop   a
1aa4: cf        mul   ya
1aa5: 7a 06     addw  ya,$06
1aa7: c4 04     mov   $04,a
1aa9: e8 02     mov   a,#$02
1aab: 04 23     or    a,$23
1aad: c4 f2     mov   $f2,a
1aaf: fa 04 f3  mov   ($f3),($04)       ; set P(L)
1ab2: bc        inc   a
1ab3: c4 f2     mov   $f2,a
1ab5: dd        mov   a,y
1ab6: 28 3f     and   a,#$3f
1ab8: c4 f3     mov   $f3,a             ; set P(H)
1aba: 6f        ret

; vcmd dispatch table (e0-ff)
1abb: dw $1b3b  ; e0
1abd: dw $1b4f  ; e1
1abf: dw $1b95  ; e2
1ac1: dw $1c4f  ; e3
1ac3: dw $1c6b  ; e4
1ac5: dw $1ccd  ; e5
1ac7: dw $1ce6  ; e6 - start loop
1ac9: dw $1cf3  ; e7 - end loop
1acb: dw $1d64  ; e8 - start loop #2
1acd: dw $1d81  ; e9 - end loop #2
1acf: dw $1de2  ; ea
1ad1: dw $1dec  ; eb
1ad3: dw $1dfd  ; ec
1ad5: dw $1fca  ; ed
1ad7: dw $1e03  ; ee
1ad9: dw $1e0d  ; ef
1adb: dw $1e1d  ; f0
1add: dw $1e24  ; f1
1adf: dw $1e42  ; f2
1ae1: dw $1e5d  ; f3
1ae3: dw $1e6c  ; f4
1ae5: dw $1e92  ; f5
1ae7: dw $1f2d  ; f6 - start complexed loop
1ae9: dw $1f3f  ; f7 - end complexed loop
1aeb: dw $1f76  ; f8
1aed: dw $1cb2  ; f9
1aef: dw $1f86  ; fa
1af1: dw $1fe0  ; fb
1af3: dw $1feb  ; fc
1af5: dw $1ff8  ; fd - goto
1af7: dw $2006  ; fe
1af9: dw $201f  ; ff

; vcmd length table (e0-ff)
; this table only suggests which vcmd will not take any parameters.
; therefore, the table content is somewhat wrong. do not trust.
1afb: dw $0001,$0002,$0001,$0001,$0003,$0003,$0000,$0003
1b0b: dw $0000,$0003,$0001,$0002,$0001,$0001,$0001,$0002
1b1b: dw $0001,$0003,$0001,$0003,$0003,$0003,$0000,$0000
1b2b: dw $0002,$0001,$0003,$0001,$0001,$0002,$0002,$0000

; vcmd e0
1b3b: d5 30 02  mov   $0230+x,a
1b3e: d4 60     mov   $60+x,a
1b40: 09 11 10  or    ($10),($11)
1b43: e8 00     mov   a,#$00
1b45: d5 31 02  mov   $0231+x,a
1b48: d4 61     mov   $61+x,a
1b4a: d5 b1 01  mov   $01b1+x,a
1b4d: 2f 15     bra   $1b64

; vcmd e1
1b4f: d5 30 02  mov   $0230+x,a
1b52: d4 60     mov   $60+x,a
1b54: 3f 56 19  call  $1956             ; arg2
1b57: d5 31 02  mov   $0231+x,a
1b5a: 1c        asl   a
1b5b: fb 60     mov   y,$60+x
1b5d: cf        mul   ya
1b5e: dd        mov   a,y
1b5f: d0 01     bne   $1b62
1b61: bc        inc   a
1b62: d4 61     mov   $61+x,a
1b64: e7 30     mov   a,($30+x)
1b66: 68 f3     cmp   a,#$f3
1b68: d0 2a     bne   $1b94
; if the next is vcmd f3, handle it here somehow
1b6a: 92 20     clr4  $20
1b6c: 3f 58 19  call  $1958             ; strip $f3
1b6f: 3f 56 19  call  $1956             ; arg1
1b72: d4 81     mov   $81+x,a
1b74: 3f 56 19  call  $1956             ; arg2
1b77: d4 80     mov   $80+x,a
1b79: 3f 56 19  call  $1956             ; arg3
1b7c: 60        clrc
1b7d: 95 21 02  adc   a,$0221+x
1b80: d5 c1 02  mov   $02c1+x,a
1b83: e8 00     mov   a,#$00
1b85: d5 c0 02  mov   $02c0+x,a
1b88: 3f 56 19  call  $1956             ; arg4
1b8b: d5 b0 02  mov   $02b0+x,a
1b8e: 3f 56 19  call  $1956             ; arg5
1b91: d5 b1 02  mov   $02b1+x,a
1b94: 6f        ret

; vcmd e2
1b95: 09 11 10  or    ($10),($11)
1b98: fd        mov   y,a
1b99: f5 a1 01  mov   a,$01a1+x
1b9c: d0 27     bne   $1bc5
1b9e: dd        mov   a,y
1b9f: 68 28     cmp   a,#$28
1ba1: b0 0c     bcs   $1baf
1ba3: 8f 3c 04  mov   $04,#$3c
1ba6: 8f 0a 05  mov   $05,#$0a
1ba9: 3f ee 1b  call  $1bee
1bac: 5f e2 18  jmp   $18e2

1baf: a8 28     sbc   a,#$28
1bb1: 2d        push  a
1bb2: eb 25     mov   y,$25
1bb4: f6 20 0a  mov   a,$0a20+y
1bb7: c4 04     mov   $04,a
1bb9: f6 21 0a  mov   a,$0a21+y
1bbc: c4 05     mov   $05,a
1bbe: ae        pop   a
1bbf: 3f ee 1b  call  $1bee
1bc2: 5f e2 18  jmp   $18e2

1bc5: fd        mov   y,a
1bc6: e8 04     mov   a,#$04
1bc8: 04 23     or    a,$23
1bca: c4 06     mov   $06,a
1bcc: c4 f2     mov   $f2,a
1bce: f5 a0 01  mov   a,$01a0+x
1bd1: c4 f3     mov   $f3,a             ; set SRCN
1bd3: 8f 1a 04  mov   $04,#$1a
1bd6: 8f 07 05  mov   $05,#$07
1bd9: e8 06     mov   a,#$06
1bdb: cf        mul   ya
1bdc: 7a 04     addw  ya,$04
1bde: da 04     movw  $04,ya
1be0: 8d 00     mov   y,#$00
1be2: 3f 04 1c  call  $1c04
1be5: 5f e2 18  jmp   $18e2

1be8: 8f e6 04  mov   $04,#$e6
1beb: 8f 0d 05  mov   $05,#$0d
;
1bee: 8d 07     mov   y,#$07
1bf0: cf        mul   ya
1bf1: 7a 04     addw  ya,$04
1bf3: da 04     movw  $04,ya
1bf5: 8d 00     mov   y,#$00
1bf7: e8 04     mov   a,#$04
1bf9: 04 23     or    a,$23
1bfb: c4 06     mov   $06,a
1bfd: c4 f2     mov   $f2,a
1bff: f7 04     mov   a,($04)+y
1c01: c4 f3     mov   $f3,a             ; set SRCN
1c03: fc        inc   y
1c04: f7 04     mov   a,($04)+y
1c06: d5 40 02  mov   $0240+x,a
1c09: fc        inc   y
1c0a: f7 04     mov   a,($04)+y
1c0c: d5 41 02  mov   $0241+x,a
1c0f: 10 07     bpl   $1c18
1c11: f5 40 02  mov   a,$0240+x
1c14: 9c        dec   a
1c15: d5 40 02  mov   $0240+x,a
1c18: fc        inc   y
1c19: ab 06     inc   $06
1c1b: fa 06 f2  mov   ($f2),($06)
1c1e: ab 06     inc   $06
1c20: f7 04     mov   a,($04)+y
1c22: c4 f3     mov   $f3,a             ; set ADSR(1)
1c24: d5 70 02  mov   $0270+x,a
1c27: 30 02     bmi   $1c2b
1c29: ab 06     inc   $06
1c2b: fc        inc   y
1c2c: fa 06 f2  mov   ($f2),($06)
1c2f: f7 04     mov   a,($04)+y
1c31: c4 f3     mov   $f3,a             ; set GAIN
1c33: d5 71 02  mov   $0271+x,a
1c36: e8 00     mov   a,#$00
1c38: d5 c1 01  mov   $01c1+x,a
1c3b: fc        inc   y
1c3c: 63 20 09  bbs3  $20,$1c48
1c3f: f7 04     mov   a,($04)+y
1c41: d5 a1 02  mov   $02a1+x,a
1c44: e8 00     mov   a,#$00
1c46: d4 70     mov   $70+x,a
1c48: fc        inc   y
1c49: f7 04     mov   a,($04)+y
1c4b: d5 b0 01  mov   $01b0+x,a
1c4e: 6f        ret

; vcmd e3
1c4f: 68 2a     cmp   a,#$2a
1c51: f0 0e     beq   $1c61
1c53: 68 2c     cmp   a,#$2c
1c55: f0 0f     beq   $1c66
1c57: d5 a1 02  mov   $02a1+x,a
1c5a: e8 00     mov   a,#$00
1c5c: d4 70     mov   $70+x,a
1c5e: 5f e2 18  jmp   $18e2

1c61: 62 20     set3  $20
1c63: 5f e2 18  jmp   $18e2

1c66: 72 20     clr3  $20
1c68: 5f e2 18  jmp   $18e2

; vcmd e4
1c6b: 2d        push  a
1c6c: 3f 56 19  call  $1956
1c6f: 68 80     cmp   a,#$80
1c71: b0 13     bcs   $1c86
1c73: 1c        asl   a
1c74: 30 0c     bmi   $1c82
1c76: 1c        asl   a
1c77: 30 05     bmi   $1c7e
1c79: 1c        asl   a
1c7a: 8d 01     mov   y,#$01
1c7c: 2f 10     bra   $1c8e
1c7e: 8d 02     mov   y,#$02
1c80: 2f 0c     bra   $1c8e
1c82: 8d 04     mov   y,#$04
1c84: 2f 08     bra   $1c8e
1c86: 8d 08     mov   y,#$08
1c88: 68 ff     cmp   a,#$ff
1c8a: d0 02     bne   $1c8e
1c8c: 8d 10     mov   y,#$10
1c8e: d4 b1     mov   $b1+x,a
1c90: db c0     mov   $c0+x,y
1c92: 3f 56 19  call  $1956
1c95: d5 20 02  mov   $0220+x,a
1c98: ae        pop   a
1c99: 68 c8     cmp   a,#$c8
1c9b: b0 0b     bcs   $1ca8
1c9d: d5 10 02  mov   $0210+x,a
1ca0: e8 00     mov   a,#$00
1ca2: d5 11 02  mov   $0211+x,a
1ca5: 5f e2 18  jmp   $18e2

1ca8: fd        mov   y,a
1ca9: e8 00     mov   a,#$00
1cab: d5 10 02  mov   $0210+x,a
1cae: dd        mov   a,y
1caf: 80        setc
1cb0: a8 c7     sbc   a,#$c7

; vcmd f9
1cb2: d5 11 02  mov   $0211+x,a
1cb5: 2d        push  a
1cb6: 8d 00     mov   y,#$00
1cb8: f5 20 02  mov   a,$0220+x
1cbb: ce        pop   x
1cbc: 9e        div   ya,x
1cbd: 2d        push  a
1cbe: e8 00     mov   a,#$00
1cc0: 9e        div   ya,x
1cc1: f8 22     mov   x,$22
1cc3: d5 e0 02  mov   $02e0+x,a
1cc6: ae        pop   a
1cc7: d5 e1 02  mov   $02e1+x,a
1cca: 5f e2 18  jmp   $18e2

; vcmd e5
1ccd: d5 d1 01  mov   $01d1+x,a
1cd0: 3f 56 19  call  $1956
1cd3: d5 f1 02  mov   $02f1+x,a
1cd6: 3f 56 19  call  $1956
1cd9: d5 f0 02  mov   $02f0+x,a
1cdc: e8 00     mov   a,#$00
1cde: d4 d1     mov   $d1+x,a
1ce0: d5 d0 01  mov   $01d0+x,a
1ce3: 5f e2 18  jmp   $18e2

; vcmd e6 - start loop
1ce6: f4 30     mov   a,$30+x
1ce8: d5 50 01  mov   $0150+x,a
1ceb: f4 31     mov   a,$31+x
1ced: d5 51 01  mov   $0151+x,a         ; save return address
1cf0: 5f e2 18  jmp   $18e2

; vcmd e7 - end loop
1cf3: 68 00     cmp   a,#$00            ; arg1 - repeat count
1cf5: f0 1e     beq   $1d15
1cf7: bb 50     inc   $50+x
1cf9: de 50 19  cbne  $50+x,$1d15
; repeat end
1cfc: 3f 58 19  call  $1958
1cff: 3f 58 19  call  $1958
1d02: e8 00     mov   a,#$00
1d04: d4 50     mov   $50+x,a
1d06: d5 10 01  mov   $0110+x,a
1d09: d5 11 01  mov   $0111+x,a
1d0c: d5 30 01  mov   $0130+x,a
1d0f: d5 31 01  mov   $0131+x,a
1d12: 5f e2 18  jmp   $18e2
; repeat again
1d15: 3f 56 19  call  $1956
1d18: 8d 00     mov   y,#$00
1d1a: 08 00     or    a,#$00
1d1c: f0 15     beq   $1d33
1d1e: 10 01     bpl   $1d21
1d20: dc        dec   y
1d21: da 04     movw  $04,ya
1d23: f5 11 01  mov   a,$0111+x
1d26: fd        mov   y,a
1d27: f5 10 01  mov   a,$0110+x
1d2a: 7a 04     addw  ya,$04
1d2c: d5 10 01  mov   $0110+x,a
1d2f: dd        mov   a,y
1d30: d5 11 01  mov   $0111+x,a         ; add arg2 to $0110/1
1d33: 3f 56 19  call  $1956
1d36: 08 00     or    a,#$00
1d38: f0 1d     beq   $1d57
1d3a: 8d 00     mov   y,#$00
1d3c: 1c        asl   a
1d3d: 90 01     bcc   $1d40
1d3f: dc        dec   y
1d40: cb 04     mov   $04,y
1d42: 1c        asl   a
1d43: 2b 04     rol   $04
1d45: 1c        asl   a
1d46: 2b 04     rol   $04
1d48: 60        clrc
1d49: 95 30 01  adc   a,$0130+x
1d4c: d5 30 01  mov   $0130+x,a
1d4f: e4 04     mov   a,$04
1d51: 95 31 01  adc   a,$0131+x
1d54: d5 31 01  mov   $0131+x,a         ; add (arg3 * 8) to $0130/1+x
1d57: f5 50 01  mov   a,$0150+x
1d5a: d4 30     mov   $30+x,a
1d5c: f5 51 01  mov   a,$0151+x
1d5f: d4 31     mov   $31+x,a           ; back to return address
1d61: 5f e2 18  jmp   $18e2

; vcmd e8 - start loop #2
1d64: f4 30     mov   a,$30+x
1d66: d5 60 01  mov   $0160+x,a
1d69: f4 31     mov   a,$31+x
1d6b: d5 61 01  mov   $0161+x,a         ; save return address
1d6e: e8 00     mov   a,#$00
1d70: d4 51     mov   $51+x,a
1d72: d5 20 01  mov   $0120+x,a
1d75: d5 21 01  mov   $0121+x,a
1d78: d5 40 01  mov   $0140+x,a
1d7b: d5 41 01  mov   $0141+x,a
1d7e: 5f e2 18  jmp   $18e2

; vcmd e9 - end loop #2
1d81: 68 00     cmp   a,#$00
1d83: f0 0e     beq   $1d93
1d85: bb 51     inc   $51+x
1d87: de 51 09  cbne  $51+x,$1d93
; repeat end
1d8a: 3f 58 19  call  $1958
1d8d: 3f 58 19  call  $1958
1d90: 5f e2 18  jmp   $18e2
; repeat again
1d93: 3f 56 19  call  $1956
1d96: 8d 00     mov   y,#$00
1d98: 08 00     or    a,#$00
1d9a: f0 15     beq   $1db1
1d9c: 10 01     bpl   $1d9f
1d9e: dc        dec   y
1d9f: da 04     movw  $04,ya
1da1: f5 21 01  mov   a,$0121+x
1da4: fd        mov   y,a
1da5: f5 20 01  mov   a,$0120+x
1da8: 7a 04     addw  ya,$04
1daa: d5 20 01  mov   $0120+x,a
1dad: dd        mov   a,y
1dae: d5 21 01  mov   $0121+x,a         ; add arg2 to $0120/1
1db1: 3f 56 19  call  $1956
1db4: 08 00     or    a,#$00
1db6: f0 1d     beq   $1dd5
1db8: 8d 00     mov   y,#$00
1dba: 1c        asl   a
1dbb: 90 01     bcc   $1dbe
1dbd: dc        dec   y
1dbe: cb 04     mov   $04,y
1dc0: 1c        asl   a
1dc1: 2b 04     rol   $04
1dc3: 1c        asl   a
1dc4: 2b 04     rol   $04
1dc6: 60        clrc
1dc7: 95 40 01  adc   a,$0140+x
1dca: d5 40 01  mov   $0140+x,a
1dcd: e4 04     mov   a,$04
1dcf: 95 41 01  adc   a,$0141+x
1dd2: d5 41 01  mov   $0141+x,a         ; add (arg3 * 8) to $0140/1+x
1dd5: f5 60 01  mov   a,$0160+x
1dd8: d4 30     mov   $30+x,a
1dda: f5 61 01  mov   a,$0161+x
1ddd: d4 31     mov   $31+x,a           ; back to return address
1ddf: 5f e2 18  jmp   $18e2

; vcmd ea
1de2: d4 41     mov   $41+x,a
1de4: e8 00     mov   a,#$00
1de6: d5 c0 01  mov   $01c0+x,a
1de9: 5f e2 18  jmp   $18e2

; vcmd eb
1dec: d5 50 02  mov   $0250+x,a
1def: 3f 56 19  call  $1956
1df2: d5 c0 01  mov   $01c0+x,a
1df5: e8 00     mov   a,#$00
1df7: d5 51 02  mov   $0251+x,a
1dfa: 5f e2 18  jmp   $18e2

; vcmd ec
1dfd: d5 21 02  mov   $0221+x,a
1e00: 5f e2 18  jmp   $18e2

; vcmd ee
1e03: d5 91 02  mov   $0291+x,a
1e06: e8 00     mov   a,#$00
1e08: d4 71     mov   $71+x,a
1e0a: 5f e2 18  jmp   $18e2

; vcmd ef
1e0d: d5 00 02  mov   $0200+x,a
1e10: 3f 56 19  call  $1956
1e13: d4 71     mov   $71+x,a
1e15: e8 00     mov   a,#$00
1e17: d5 90 02  mov   $0290+x,a
1e1a: 5f e2 18  jmp   $18e2

; vcmd f0
1e1d: d4 90     mov   $90+x,a
1e1f: 82 20     set4  $20
1e21: 5f e2 18  jmp   $18e2

; vcmd f1
1e24: 92 20     clr4  $20
1e26: d4 91     mov   $91+x,a
1e28: 3f 56 19  call  $1956
1e2b: d4 90     mov   $90+x,a
1e2d: 3f 56 19  call  $1956
1e30: d5 60 02  mov   $0260+x,a
1e33: 3f 56 19  call  $1956
1e36: d5 b0 02  mov   $02b0+x,a
1e39: 3f 56 19  call  $1956
1e3c: d5 b1 02  mov   $02b1+x,a
1e3f: 5f e2 18  jmp   $18e2

; vcmd f2
1e42: 1c        asl   a
1e43: b0 08     bcs   $1e4d
1e45: 8d 00     mov   y,#$00
1e47: 1c        asl   a
1e48: 90 09     bcc   $1e53
1e4a: fc        inc   y
1e4b: 2f 06     bra   $1e53
1e4d: 8d ff     mov   y,#$ff
1e4f: 1c        asl   a
1e50: b0 01     bcs   $1e53
1e52: dc        dec   y
1e53: d5 80 02  mov   $0280+x,a
1e56: dd        mov   a,y
1e57: d5 81 02  mov   $0281+x,a
1e5a: 5f e2 18  jmp   $18e2

; vcmd f3
1e5d: 3f 58 19  call  $1958
1e60: 3f 58 19  call  $1958
1e63: 3f 58 19  call  $1958
1e66: 3f 58 19  call  $1958
1e69: 5f e2 18  jmp   $18e2

; vcmd f4
1e6c: 08 00     or    a,#$00
1e6e: f0 16     beq   $1e86
1e70: b2 13     clr5  $13
1e72: c4 14     mov   $14,a
1e74: 8f 4d f2  mov   $f2,#$4d
1e77: c4 f3     mov   $f3,a             ; set EON
1e79: 3f 56 19  call  $1956
1e7c: c4 15     mov   $15,a
1e7e: 3f 56 19  call  $1956
1e81: c4 16     mov   $16,a
1e83: 5f e2 18  jmp   $18e2

1e86: 3f f4 1e  call  $1ef4
1e89: 3f 58 19  call  $1958
1e8c: 3f 58 19  call  $1958
1e8f: 5f e2 18  jmp   $18e2

; vcmd f5
1e92: 78 00 14  cmp   $14,#$00
1e95: f0 f2     beq   $1e89
1e97: 28 0f     and   a,#$0f
1e99: 64 18     cmp   a,$18
1e9b: f0 31     beq   $1ece
1e9d: eb 18     mov   y,$18
1e9f: c4 18     mov   $18,a
1ea1: dd        mov   a,y
1ea2: 1c        asl   a
1ea3: 1c        asl   a
1ea4: 1c        asl   a
1ea5: 1c        asl   a
1ea6: 08 0f     or    a,#$0f
1ea8: 60        clrc
1ea9: 84 19     adc   a,$19
1eab: c4 19     mov   $19,a
1ead: 8f 6c f2  mov   $f2,#$6c
1eb0: e4 13     mov   a,$13
1eb2: 08 20     or    a,#$20
1eb4: c4 f3     mov   $f3,a             ; set FLG
1eb6: 8f 7d f2  mov   $f2,#$7d
1eb9: fa 18 f3  mov   ($f3),($18)       ; set EDL
1ebc: e4 18     mov   a,$18
1ebe: f0 09     beq   $1ec9
1ec0: 1c        asl   a
1ec1: 1c        asl   a
1ec2: 1c        asl   a
1ec3: 48 ff     eor   a,#$ff
1ec5: bc        inc   a
1ec6: 60        clrc
1ec7: 88 00     adc   a,#$00
1ec9: 8f 6d f2  mov   $f2,#$6d
1ecc: c4 f3     mov   $f3,a             ; set ESA
1ece: e4 18     mov   a,$18
1ed0: 1c        asl   a
1ed1: 1c        asl   a
1ed2: 1c        asl   a
1ed3: 1c        asl   a
1ed4: 08 0f     or    a,#$0f
1ed6: c4 1a     mov   $1a,a
1ed8: 3f 56 19  call  $1956
1edb: c4 17     mov   $17,a
1edd: 3f 58 19  call  $1958
1ee0: 8d 00     mov   y,#$00
1ee2: f6 e7 22  mov   a,$22e7+y
1ee5: c4 f2     mov   $f2,a
1ee7: f6 ef 22  mov   a,$22ef+y
1eea: c4 f3     mov   $f3,a             ; set FIR
1eec: fc        inc   y
1eed: ad 08     cmp   y,#$08
1eef: d0 f1     bne   $1ee2
1ef1: 5f e2 18  jmp   $18e2

1ef4: e4 14     mov   a,$14
1ef6: f0 34     beq   $1f2c
1ef8: e4 18     mov   a,$18
1efa: 1c        asl   a
1efb: 1c        asl   a
1efc: bc        inc   a
1efd: c4 19     mov   $19,a
1eff: e8 00     mov   a,#$00
1f01: 8f 2c f2  mov   $f2,#$2c
1f04: c4 f3     mov   $f3,a             ; set EVOL(L)
1f06: 8f 3c f2  mov   $f2,#$3c
1f09: c4 f3     mov   $f3,a             ; set EVOL(R)
1f0b: 8f 0d f2  mov   $f2,#$0d
1f0e: c4 f3     mov   $f3,a             ; set EFB
1f10: a2 13     set5  $13
1f12: 8f 6c f2  mov   $f2,#$6c
1f15: fa 13 f3  mov   ($f3),($13)       ; set FLG
1f18: c4 14     mov   $14,a
1f1a: c4 15     mov   $15,a
1f1c: c4 16     mov   $16,a
1f1e: c4 17     mov   $17,a
1f20: c4 18     mov   $18,a
1f22: 8f 7d f2  mov   $f2,#$7d
1f25: c4 f3     mov   $f3,a             ; set EDL
1f27: 8f 6d f2  mov   $f2,#$6d
1f2a: c4 f3     mov   $f3,a             ; set ESA
1f2c: 6f        ret

; vcmd f6 - start complexed loop
1f2d: f4 30     mov   a,$30+x
1f2f: d5 70 01  mov   $0170+x,a
1f32: f4 31     mov   a,$31+x
1f34: d5 71 01  mov   $0171+x,a         ; save current address to $0170/1
1f37: e8 c0     mov   a,#$c0
1f39: 4e 20 00  tclr1 $0020             ; reset "visited" flags
1f3c: 5f e2 18  jmp   $18e2

; vcmd f7 - end complexed loop
1f3f: c3 20 08  bbs6  $20,$1f4a
1f42: e3 20 20  bbs7  $20,$1f65
; first time, do nothing
1f45: c2 20     set6  $20
1f47: 5f e2 18  jmp   $18e2
; second time
1f4a: d2 20     clr6  $20
1f4c: e2 20     set7  $20
1f4e: f4 30     mov   a,$30+x
1f50: d5 80 01  mov   $0180+x,a
1f53: f4 31     mov   a,$31+x
1f55: d5 81 01  mov   $0181+x,a         ; save current address to $0180/1
1f58: f5 70 01  mov   a,$0170+x
1f5b: d4 30     mov   $30+x,a
1f5d: f5 71 01  mov   a,$0171+x
1f60: d4 31     mov   $31+x,a           ; back to $0170/1
1f62: 5f e2 18  jmp   $18e2
; third time
1f65: c2 20     set6  $20
1f67: f2 20     clr7  $20
1f69: f5 80 01  mov   a,$0180+x
1f6c: d4 30     mov   $30+x,a
1f6e: f5 81 01  mov   a,$0181+x
1f71: d4 31     mov   $31+x,a           ; back to $0180/1
1f73: 5f e2 18  jmp   $18e2

; vcmd f8
1f76: d5 01 02  mov   $0201+x,a
1f79: 3f 56 19  call  $1956
1f7c: d4 70     mov   $70+x,a
1f7e: e8 00     mov   a,#$00
1f80: d5 a0 02  mov   $02a0+x,a
1f83: 5f e2 18  jmp   $18e2

; vcmd fa
1f86: d5 70 02  mov   $0270+x,a
1f89: fd        mov   y,a
1f8a: e8 05     mov   a,#$05
1f8c: 04 23     or    a,$23
1f8e: c4 f2     mov   $f2,a
1f90: cb f3     mov   $f3,y             ; set ADSR(1)
1f92: bc        inc   a
1f93: ad 80     cmp   y,#$80
1f95: 90 09     bcc   $1fa0
1f97: c4 f2     mov   $f2,a
1f99: 3f 56 19  call  $1956
1f9c: c4 f3     mov   $f3,a             ; set ADSR(2)
1f9e: 2f 0b     bra   $1fab
1fa0: bc        inc   a
1fa1: c4 f2     mov   $f2,a
1fa3: 3f 56 19  call  $1956
1fa6: c4 f3     mov   $f3,a             ; set GAIN
1fa8: d5 71 02  mov   $0271+x,a
; vcmd 62
1fab: 3f 56 19  call  $1956             ; read a byte
1fae: 68 c8     cmp   a,#$c8
1fb0: b0 0c     bcs   $1fbe
1fb2: 68 64     cmp   a,#$64
1fb4: 90 0e     bcc   $1fc4
1fb6: a8 64     sbc   a,#$64
1fb8: 28 1f     and   a,#$1f
1fba: 08 80     or    a,#$80
1fbc: 2f 06     bra   $1fc4
1fbe: a8 c8     sbc   a,#$c8
1fc0: 28 1f     and   a,#$1f
1fc2: 08 a0     or    a,#$a0
1fc4: d5 c1 01  mov   $01c1+x,a
1fc7: 5f e2 18  jmp   $18e2

; vcmd ed
1fca: 2d        push  a
1fcb: f5 70 02  mov   a,$0270+x
1fce: ae        pop   a
1fcf: 10 0c     bpl   $1fdd
1fd1: d5 70 02  mov   $0270+x,a
1fd4: fd        mov   y,a
1fd5: e8 05     mov   a,#$05
1fd7: 04 23     or    a,$23
1fd9: c4 f2     mov   $f2,a
1fdb: cb f3     mov   $f3,y             ; set ADSR(1)
1fdd: 5f e2 18  jmp   $18e2

; vcmd fb
1fe0: 2d        push  a
1fe1: f5 70 02  mov   a,$0270+x
1fe4: ee        pop   y
1fe5: 10 f6     bpl   $1fdd
1fe7: e8 06     mov   a,#$06
1fe9: 2f ec     bra   $1fd7

; vcmd fc
1feb: d5 91 02  mov   $0291+x,a
1fee: e8 00     mov   a,#$00
1ff0: d4 71     mov   $71+x,a
1ff2: 3f 56 19  call  $1956
1ff5: 5f 95 1b  jmp   $1b95             ; redirect to vcmd e2

; vcmd fd - goto
1ff8: c4 04     mov   $04,a
1ffa: 3f 56 19  call  $1956
1ffd: d4 31     mov   $31+x,a
1fff: e4 04     mov   a,$04
2001: d4 30     mov   $30+x,a
2003: 5f e2 18  jmp   $18e2

; vcmd fe - call subroutine
2006: 2d        push  a
2007: 3f 56 19  call  $1956
200a: fd        mov   y,a
200b: f4 30     mov   a,$30+x
200d: d5 00 01  mov   $0100+x,a
2010: f4 31     mov   a,$31+x
2012: d5 01 01  mov   $0101+x,a
2015: ae        pop   a
2016: d4 30     mov   $30+x,a
2018: db 31     mov   $31+x,y
201a: 42 20     set2  $20
201c: 5f e2 18  jmp   $18e2

; vcmd ff - end of track / end subroutine
201f: 53 20 0f  bbc2  $20,$2031
; end subroutine
2022: 52 20     clr2  $20
2024: f5 00 01  mov   a,$0100+x
2027: d4 30     mov   $30+x,a
2029: f5 01 01  mov   a,$0101+x
202c: d4 31     mov   $31+x,a
202e: 5f e2 18  jmp   $18e2
; end of track
2031: e8 00     mov   a,#$00
2033: d4 d0     mov   $d0+x,a
2035: d5 a1 01  mov   $01a1+x,a
2038: d5 90 01  mov   $0190+x,a
203b: e4 11     mov   a,$11
203d: 0e 10 00  tset1 $0010
2040: 4e 0f 00  tclr1 $000f
2043: a2 20     set5  $20
2045: 6f        ret

2046: da 06     movw  $06,ya
2048: e4 04     mov   a,$04
204a: 9f        xcn   a
204b: 28 f0     and   a,#$f0
204d: 2d        push  a
204e: e4 04     mov   a,$04
2050: 30 0f     bmi   $2061
2052: 9f        xcn   a
2053: 28 07     and   a,#$07
2055: fd        mov   y,a
2056: ae        pop   a
2057: 60        clrc
2058: 7a 06     addw  ya,$06
205a: b0 13     bcs   $206f
205c: 7e 05     cmp   y,$05
205e: b0 0f     bcs   $206f
2060: 6f        ret

2061: 9f        xcn   a
2062: 08 f8     or    a,#$f8
2064: fd        mov   y,a
2065: ae        pop   a
2066: 60        clrc
2067: 7a 06     addw  ya,$06
2069: 90 04     bcc   $206f
206b: 7e 05     cmp   y,$05
206d: b0 05     bcs   $2074
206f: 8f 00 04  mov   $04,#$00
2072: eb 05     mov   y,$05
2074: 6f        ret

2075: f4 e0     mov   a,$e0+x
2077: fb e1     mov   y,$e1+x
2079: da 04     movw  $04,ya
207b: f5 c1 02  mov   a,$02c1+x
207e: fd        mov   y,a
207f: f5 c0 02  mov   a,$02c0+x
2082: 9a 04     subw  ya,$04
2084: f0 32     beq   $20b8
2086: da 06     movw  $06,ya
2088: 10 07     bpl   $2091
208a: e8 00     mov   a,#$00
208c: fd        mov   y,a
208d: 9a 06     subw  ya,$06
208f: c4 06     mov   $06,a
2091: f4 90     mov   a,$90+x
2093: cf        mul   ya
2094: da 08     movw  $08,ya
2096: eb 06     mov   y,$06
2098: f4 90     mov   a,$90+x
209a: cf        mul   ya
209b: dd        mov   a,y
209c: 8d 00     mov   y,#$00
209e: 7a 08     addw  ya,$08
20a0: d0 01     bne   $20a3
20a2: bc        inc   a
20a3: f3 07 0b  bbc7  $07,$20b1
20a6: da 06     movw  $06,ya
20a8: ba 04     movw  ya,$04
20aa: 9a 06     subw  ya,$06
20ac: d4 e0     mov   $e0+x,a
20ae: db e1     mov   $e1+x,y
20b0: 6f        ret

20b1: 7a 04     addw  ya,$04
20b3: d4 e0     mov   $e0+x,a
20b5: db e1     mov   $e1+x,y
20b7: 6f        ret

20b8: d4 80     mov   $80+x,a
20ba: 6f        ret

20bb: f4 b0     mov   a,$b0+x
20bd: 60        clrc
20be: 94 b1     adc   a,$b1+x
20c0: d4 b0     mov   $b0+x,a
20c2: 90 3a     bcc   $20fe
20c4: 02 21     set0  $21
20c6: f4 c1     mov   a,$c1+x
20c8: 60        clrc
20c9: 94 c0     adc   a,$c0+x
20cb: 28 3f     and   a,#$3f
20cd: c4 04     mov   $04,a
20cf: d4 c1     mov   $c1+x,a
20d1: 28 1f     and   a,#$1f
20d3: fd        mov   y,a
20d4: f6 9d 22  mov   a,$229d+y
20d7: fd        mov   y,a
20d8: f5 d1 02  mov   a,$02d1+x
20db: 30 09     bmi   $20e6
20dd: 1c        asl   a
20de: cf        mul   ya
20df: cb 08     mov   $08,y
20e1: 8f 00 09  mov   $09,#$00
20e4: 2f 0a     bra   $20f0
20e6: 80        setc
20e7: a8 7e     sbc   a,#$7e
20e9: cf        mul   ya
20ea: da 08     movw  $08,ya
20ec: 4b 09     lsr   $09
20ee: 6b 08     ror   $08
20f0: ba 0a     movw  ya,$0a
20f2: b3 04 05  bbc5  $04,$20fa
20f5: 9a 08     subw  ya,$08
20f7: da 0a     movw  $0a,ya
20f9: 6f        ret

20fa: 7a 08     addw  ya,$08
20fc: da 0a     movw  $0a,ya
20fe: 6f        ret

20ff: f5 d0 01  mov   a,$01d0+x
2102: fd        mov   y,a
2103: bc        inc   a
2104: 28 7f     and   a,#$7f
2106: d5 d0 01  mov   $01d0+x,a
2109: f6 f7 22  mov   a,$22f7+y
210c: 35 f0 02  and   a,$02f0+x
210f: 2d        push  a
2110: f6 f8 22  mov   a,$22f8+y
2113: 35 f1 02  and   a,$02f1+x
2116: fd        mov   y,a
2117: ae        pop   a
2118: 7a 0a     addw  ya,$0a
211a: 5f 7f 1a  jmp   $1a7f






















211d: db $04,$01
211f: db $05,$01
2121: db $04,$01
2123: db $05,$01
2125: db $05,$01
2127: db $05,$01
2129: db $06,$01
212b: db $06,$01
212d: db $06,$01
212f: db $07,$01
2131: db $07,$01
2133: db $07,$01
2135: db $08,$01
2137: db $09,$01
2139: db $09,$01
213b: db $09,$01
213d: db $0a,$01
213f: db $0b,$01
2141: db $0b,$01
2143: db $0c,$01
2145: db $0d,$01
2147: db $0d,$01
2149: db $0e,$01
214b: db $0f,$01
214d: db $10,$01
214f: db $11,$01
2151: db $12,$01
2153: db $13,$01
2155: db $14,$01
2157: db $15,$01
2159: db $17,$01
215b: db $18,$01
215d: db $19,$01
215f: db $1b,$01
2161: db $1c,$01
2163: db $1e,$01
2165: db $20,$01
2167: db $22,$01
2169: db $24,$01
216b: db $26,$01
216d: db $28,$01
216f: db $2a,$01
2171: db $2d,$01
2173: db $30,$01
2175: db $33,$01
2177: db $35,$01
2179: db $39,$01
217b: db $3c,$01
217d: db $40,$01
217f: db $43,$01
2181: db $48,$01
2183: db $4c,$01
2185: db $50,$01
2187: db $55,$01
2189: db $5a,$01
218b: db $60,$01
218d: db $65,$01
218f: db $6b,$01
2191: db $71,$01
2193: db $79,$01
2195: db $7f,$01
2197: db $87,$01
2199: db $8f,$01
219b: db $98,$01
219d: db $a0,$01
219f: db $aa,$01
21a1: db $b5,$01
21a3: db $bf,$01
21a5: db $ca,$01
21a7: db $d6,$01
21a9: db $e3,$01
21ab: db $f1,$01
21ad: db $ff,$01
21af: db $87,$02
21b1: db $8f,$02
21b3: db $97,$02
21b5: db $a0,$02
21b7: db $aa,$02
21b9: db $b4,$02
21bb: db $bf,$02
21bd: db $ca,$02
21bf: db $d6,$02
21c1: db $e3,$02
21c3: db $f0,$02
21c5: db $ff,$02
21c7: db $b4,$03
21c9: db $be,$03
21cb: db $ca,$03
21cd: db $d6,$03
21cf: db $e2,$03
21d1: db $f0,$03
21d3: db $fe,$03
21d5: db $ca,$04
21d7: db $d7,$04
21d9: db $e2,$04
21db: db $c9,$01

21dd: db $01,$01,$01,$01,$01,$01,$01,$01
21e5: db $01,$01,$01,$01,$01,$01,$02,$01
21ed: db $01,$01,$01,$01,$01,$01,$01,$01
21f5: db $02,$01,$01,$01,$02,$01,$01,$01
21fd: db $02,$01,$02,$01,$01,$01,$02,$01
2205: db $03,$01,$02,$01,$02,$01,$03,$01
220d: db $02,$01,$03,$01,$03,$01,$03,$01
2215: db $04,$01,$03,$01,$03,$01,$04,$01

221d: db $00,$01,$01,$01,$01,$01,$01,$01
2225: db $01,$01,$01,$01,$01,$01,$01,$01
222d: db $01,$01,$01,$01,$01,$01,$01,$01
2235: db $01,$01,$01,$01,$01,$01,$02,$02
223d: db $02,$02,$02,$02,$02,$02,$02,$02
2245: db $03,$03,$03,$03,$03,$03,$04,$04
224d: db $04,$04,$04,$04,$05,$05,$05,$05
2255: db $06,$06,$07,$07,$07,$07,$08,$08
225d: db $09,$09,$0a,$0a,$0a,$0a,$0b,$0b
2265: db $0c,$0c,$0d,$0d,$0e,$0f,$10,$10
226d: db $11,$12,$13,$14,$15,$15,$16,$17
2275: db $18,$19,$1b,$1c,$1d,$1e,$20,$22
227d: db $23,$24,$26,$28,$2a,$2c,$2d,$2f
2285: db $31,$33,$35,$38,$3a,$3d,$40,$43
228d: db $46,$49,$4c,$4f,$52,$56,$5a,$5e
2295: db $62,$66,$6b,$6f,$73,$77,$7b,$7f

229d: db $00,$20,$38,$50,$68,$80,$90,$a0
22a5: db $b0,$c0,$d0,$e0,$e8,$f0,$f0,$f8
22ad: db $ff,$f8,$f4,$f0,$e8,$e0,$d0,$c0
22b5: db $b0,$a0,$90,$80,$68,$50,$38,$20

22bd: db $00,$04,$08,$0e,$14,$1a,$20,$28
22c5: db $30,$38,$40,$48,$50,$5a,$64,$6e
22cd: db $78,$82,$8c,$96,$a0,$a8,$b0,$b8
22d5: db $c0,$c8,$d0,$d6,$dc,$e0,$e4,$e8
22dd: db $ec,$f0,$f4,$f6,$f8,$fa,$fc,$fe
22e5: db $fe,$fe

22e7: db $0f,$1f,$2f,$3f,$4f,$5f,$6f,$7f

22ef: db $7f,$00,$00,$00,$00,$00,$00,$00













22f7: db $9f,$3c,$b2,$52,$af,$45,$c7,$89
22ff: db $10,$7f,$e0,$9d,$dc,$1f,$61,$16
2307: db $39,$c9,$9c,$eb,$57,$08,$66,$f8
230f: db $5a,$24,$bf,$0e,$3e,$15,$4b,$db
2317: db $ab,$f5,$31,$0c,$43,$02,$55,$de
231f: db $41,$da,$bd,$ae,$19,$b0,$48,$57
2327: db $ba,$a3,$36,$0b,$f9,$df,$17,$a8
232f: db $04,$0c,$e0,$91,$18,$5d,$dd,$d3
2337: db $28,$8a,$f2,$11,$59,$6f

233d: 06        or    a,(x)
233e: 0a 34 2a  or1   c,$0546,4
2341: 79        cmp   (x),(y)
2342: ac 5e a7  inc   $a75e
2345: 83 c6 39  bbs4  $c6,$2381
2348: c1        tcall 12
2349: b4 3a     sbc   a,$3a+x
234b: 3f fe 4f  call  $4ffe
234e: ef        sleep
234f: 1f 00 30  jmp   ($3000+x)

2352: 99        adc   (x),(y)
2353: 4c 28 83  lsr   $8328
2356: ed        notc
2357: 8a 2f 2c  eor1  c,$0585,7
235a: 66        cmp   a,(x)
235b: 3f d6 6c  call  $6cd6
235e: b7 49     sbc   a,($49)+y
2360: 22 bc     set1  $bc
2362: 65 fa cf  cmp   a,$cffa
2365: 02 b1     set0  $b1
2367: 46        eor   a,(x)
2368: f0 9a     beq   $2304
236a: d7 e2     mov   ($e2)+y,a
236c: 0f        brk
236d: 11        tcall 1
236e: c5 74 f6  mov   $f674,a
2371: 7a 2c     addw  ya,$2c
2373: 8f fb 19  mov   $19,#$fb
2376: 6a e5 7a  and1  c,!($0f5c,5)
2379: 23 fc bc  bbs1  $fc,$2338
237c: 04 3a     or    a,$3a
237e: 1a 7f     decw  $7f
2380: 7f        reti
2381: fb d3     mov   y,$d3+x
2383: e1        tcall 14
2384: 28 7f     and   a,#$7f
2386: ff        stop
2387: 89 23 ea  adc   ($ea),($23)
238a: 78 fc ee  cmp   $ee,#$fc
238d: 00        nop
238e: 3a 14     incw  $14
2390: 7d        mov   a,x
2391: 7f        reti
2392: ed        notc
2393: 86        adc   a,(x)
2394: ba ff     movw  ya,$ff
2396: ba e9     movw  ya,$e9
2398: ba d5     movw  ya,$d5
239a: ba c1     movw  ya,$c1
239c: ba ad     movw  ya,$ad
239e: ff        stop
239f: a1        tcall 10
23a0: 23 ea 78  bbs1  $ea,$241b
23a3: fc        inc   y
23a4: d6 01 ed  mov   $ed01+y,a
23a7: 8b e4     dec   $e4
23a9: 00        nop
23aa: 35 7f 3d  and   a,$3d7f+x
23ad: 12 7d     clr0  $7d
23af: 7f        reti
23b0: bd        mov   sp,x
23b1: e9 bd d5  mov   x,$d5bd
23b4: bd        mov   sp,x
23b5: c1        tcall 12
23b6: bd        mov   sp,x
23b7: ad ff     cmp   y,#$ff
23b9: bb 23     inc   $23+x
23bb: ea a0 fc  not1  $1f94,0
23be: d0 0c     bne   $23cc
23c0: 29 05 7f  and   ($7f),($05)
23c3: 7f        reti
23c4: 2f 20     bra   $23e6
23c6: 7d        mov   a,x
23c7: 7f        reti
23c8: fb d2     mov   y,$d2+x
23ca: 29 05 7f  and   ($7f),($05)
23cd: 41        tcall 4
23ce: 2f 20     bra   $23f0
23d0: 7d        mov   a,x
23d1: 41        tcall 4
23d2: ff        stop
23d3: d5 23 fc  mov   $fc23+x,a
23d6: c8 01     cmp   x,#$01
23d8: 3a 11     incw  $11
23da: 7d        mov   a,x
23db: 7f        reti
23dc: ed        notc
23dd: 8a ba ad  eor1  c,$15b7,2
23e0: ff        stop
23e1: e3 23 fc  bbs7  $23,$23e0
23e4: e0        clrv
23e5: 0e 3c 11  tset1 $113c
23e8: 7d        mov   a,x
23e9: 7f        reti
23ea: ed        notc
23eb: 8a bc ad  eor1  c,$15b7,4
23ee: ff        stop
23ef: f1        tcall 15
23f0: 23 fc ce  bbs1  $fc,$23c1
23f3: 0f        brk
23f4: 39        and   (x),(y)
23f5: 0c 7d 7f  asl   $7f7d
23f8: ed        notc
23f9: 8a b9 ad  eor1  c,$15b7,1
23fc: ff        stop
23fd: ff        stop
23fe: 23 ea 64  bbs1  $ea,$2465
2401: fc        inc   y
2402: ec 02 fa  mov   y,$fa02
2405: 8e        pop   psw
2406: d2 00     clr6  $00
2408: 2b 5f     rol   $5f
240a: 7f        reti
240b: 7f        reti
240c: f3 28 05  bbc7  $28,$2414
240f: 23 56 ff  bbs1  $56,$2411
2412: ff        stop
2413: 15 24 ea  or    a,$ea24+x
2416: 78 fc e4  cmp   $e4,#$fc
2419: 03 0c 0d  bbs0  $0c,$2429
241c: 7d        mov   a,x
241d: 00        nop
241e: 45 10 7f  eor   a,$7f10
2421: 7f        reti
2422: fb d7     mov   y,$d7+x
2424: c5 7d 7f  mov   $7f7d,a
2427: f3 00 10  bbc7  $00,$243a
242a: 41        tcall 4
242b: e2 ff     set7  $ff
242d: fa 89 d3  mov   ($d3),($89)
2430: 00        nop
2431: 45 20 cb  eor   a,$cb20
2434: c5 b7 ff  mov   $ffb7,a
2437: 3b 24     rol   $24+x
2439: 4b 24     lsr   $24
243b: ea 64 fc  not1  $1f8c,4
243e: d8 04     mov   $04,x
2440: fa 8f d1  mov   ($d1),($8f)
2443: 00        nop
2444: e3 1a 2b  bbs7  $1a,$2472
2447: 69 7f 7f  cmp   ($7f),($7f)
244a: ff        stop
244b: ea 64 fc  not1  $1f8c,4
244e: d8 04     mov   $04,x
2450: fa 8f d1  mov   ($d1),($8f)
2453: 00        nop
2454: e3 0e 27  bbs7  $0e,$247e
2457: 69 7f 7f  cmp   ($7f),($7f)
245a: ff        stop
245b: 5d        mov   x,a
245c: 24 fc     and   a,$fc
245e: d6 0b 37  mov   $370b+y,a
2461: 23 7f 7f  bbs1  $7f,$24e3
2464: fb d2     mov   y,$d2+x
2466: 37 37     and   a,($37)+y
2468: ff        stop
2469: ff        stop
246a: 6c 24 fc  ror   $fc24
246d: dc        dec   y
246e: 0a 3b 08  or1   c,$0107,3
2471: 7d        mov   a,x
2472: 7f        reti
2473: 3b 14     rol   $14+x
2475: 7f        reti
2476: 7f        reti
2477: fb da     mov   y,$da+x
2479: 3b 05     rol   $05+x
247b: 7d        mov   a,x
247c: 7f        reti
247d: fa 8d d9  mov   ($d9),($8d)
2480: 00        nop
2481: 3b 0d     rol   $0d+x
2483: 7f        reti
2484: 37 ff     and   a,($ff)+y
2486: 88 24     adc   a,#$24
2488: ea ff fc  not1  $1f9f,7
248b: fe 09     dbnz  y,$2496
248d: ed        notc
248e: 8a 2a 1b  eor1  c,$0365,2
2491: 7d        mov   a,x
2492: 78 ed 8f  cmp   $8f,#$ed
2495: 2d        push  a
2496: 15 ff ed  or    a,$edff+x
2499: 89 ad e9  adc   ($e9),($ad)
249c: ad d5     cmp   y,#$d5
249e: ad c1     cmp   y,#$c1
24a0: ad b2     cmp   y,#$b2
24a2: ff        stop
24a3: a5 24 fc  sbc   a,$fc24
24a6: e8 10     mov   a,#$10
24a8: 32 18     clr1  $18
24aa: 7d        mov   a,x
24ab: 7f        reti
24ac: fa 86 d6  mov   ($d6),($86)
24af: 00        nop
24b0: b2 ad     clr5  $ad
24b2: ff        stop
24b3: b5 24 fc  sbc   a,$fc24+x
24b6: e2 08     set7  $08
24b8: 2d        push  a
24b9: 18 7d 7f  or    $7f,#$7d
24bc: fa 88 d6  mov   ($d6),($88)
24bf: 00        nop
24c0: ad b5     cmp   y,#$b5
24c2: ff        stop
24c3: c5 24 fc  mov   $fc24,a
24c6: f0 12     beq   $24da
24c8: 3b 28     rol   $28+x
24ca: 7f        reti
24cb: 7f        reti
24cc: fa 8f d5  mov   ($d5),($8f)
24cf: 00        nop
24d0: e1        tcall 14
24d1: 1e 7d ff  cmp   x,$ff7d
24d4: d6 24 fc  mov   $fc24+y,a
24d7: dc        dec   y
24d8: 00        nop
24d9: 3a 23     incw  $23
24db: 7f        reti
24dc: 7f        reti
24dd: ff        stop
24de: e0        clrv
24df: 24 ea     and   a,$ea
24e1: ff        stop
24e2: fc        inc   y
24e3: e4 00     mov   a,$00
24e5: 36 be 7f  and   a,$7fbe+y
24e8: 7f        reti
24e9: fb d2     mov   y,$d2+x
24eb: 36 78 e9  and   a,$e978+y
24ee: ff        stop
24ef: f1        tcall 15
24f0: 24 ea     and   a,$ea
24f2: ff        stop
24f3: fc        inc   y
24f4: f2 00     clr7  $00
24f6: 36 cf 7f  and   a,$7fcf+y
24f9: 7f        reti
24fa: fb d2     mov   y,$d2+x
24fc: 36 6e b7  and   a,$b76e+y
24ff: ff        stop
2500: 02 25     set0  $25
2502: ea ff fc  not1  $1f9f,7
2505: fa 00 36  mov   ($36),($00)
2508: c0        di
2509: 7f        reti
250a: 7f        reti
250b: fb d4     mov   y,$d4+x
250d: 36 6e b7  and   a,$b76e+y
2510: ff        stop
2511: 13 25 ea  bbc0  $25,$24fe
2514: ff        stop
2515: fc        inc   y
2516: ec 08 36  mov   y,$3608
2519: 7d        mov   a,x
251a: 7f        reti
251b: 7f        reti
251c: fb d4     mov   y,$d4+x
251e: 36 36 b7  and   a,$b736+y
2521: ff        stop
2522: 24 25     and   a,$25
2524: ea ff fc  not1  $1f9f,7
2527: fe 00     dbnz  y,$2529
2529: 36 c7 7f  and   a,$7fc7+y
252c: 7f        reti
252d: fb d2     mov   y,$d2+x
252f: 36 af e4  and   a,$e4af+y
2532: ff        stop
2533: 35 25 ea  and   a,$ea25+x
2536: ff        stop
2537: fc        inc   y
2538: fa 00 36  mov   ($36),($00)
253b: a8 7f     sbc   a,#$7f
253d: 7f        reti
253e: fb d4     mov   y,$d4+x
2540: 36 42 c1  and   a,$c142+y
2543: ff        stop
2544: 46        eor   a,(x)
2545: 25 ea ff  and   a,$ffea
2548: fc        inc   y
2549: fe 08     dbnz  y,$2553
254b: 36 c0 7f  and   a,$7fc0+y
254e: 7f        reti
254f: fb d4     mov   y,$d4+x
2551: 36 64 e4  and   a,$e464+y
2554: ff        stop
2555: 57 25     eor   a,($25)+y
2557: ea ff fc  not1  $1f9f,7
255a: fe 09     dbnz  y,$2565
255c: 36 3f 7f  and   a,$7f3f+y
255f: 7f        reti
2560: fb d4     mov   y,$d4+x
2562: 36 2d c6  and   a,$c62d+y
2565: ff        stop
2566: 68 25     cmp   a,#$25
2568: ea ff fc  not1  $1f9f,7
256b: e4 01     mov   a,$01
256d: 39        and   (x),(y)
256e: ce        pop   x
256f: 7f        reti
2570: 7f        reti
2571: fb d2     mov   y,$d2+x
2573: 39        and   (x),(y)
2574: 6e bc ff  dbnz  $bc,$2576
2577: 79        cmp   (x),(y)
2578: 25 ea ff  and   a,$ffea
257b: fc        inc   y
257c: fe 00     dbnz  y,$257e
257e: 36 46 7f  and   a,$7f46+y
2581: 7f        reti
2582: fb d4     mov   y,$d4+x
2584: 36 3e c1  and   a,$c13e+y
2587: ff        stop
2588: 8a 25 ea  eor1  c,$1d44,5
258b: ff        stop
258c: fc        inc   y
258d: fe 01     dbnz  y,$2590
258f: 36 bd 7f  and   a,$7fbd+y
2592: 7f        reti
2593: fb d4     mov   y,$d4+x
2595: 36 51 b4  and   a,$b451+y
2598: ff        stop
2599: 9b 25     dec   $25+x
259b: ea ff fc  not1  $1f9f,7
259e: ec 09 36  mov   y,$3609
25a1: 52 7f     clr2  $7f
25a3: 7f        reti
25a4: fb d4     mov   y,$d4+x
25a6: 36 4f ba  and   a,$ba4f+y
25a9: ff        stop
25aa: ac 25 ea  inc   $ea25
25ad: ff        stop
25ae: fc        inc   y
25af: f2 0a     clr7  $0a
25b1: 36 b4 7f  and   a,$7fb4+y
25b4: 7f        reti
25b5: fb d2     mov   y,$d2+x
25b7: 36 dc e9  and   a,$e9dc+y
25ba: ff        stop
25bb: bd        mov   sp,x
25bc: 25 ea ff  and   a,$ffea
25bf: fc        inc   y
25c0: f0 02     beq   $25c4
25c2: 38 f5 7f  and   $7f,#$f5
25c5: 7f        reti
25c6: fb d4     mov   y,$d4+x
25c8: 38 46 b2  and   $b2,#$46
25cb: ff        stop
25cc: ce        pop   x
25cd: 25 ea ff  and   a,$ffea
25d0: fc        inc   y
25d1: f0 0b     beq   $25de
25d3: 36 8a 7f  and   a,$7f8a+y
25d6: 7f        reti
25d7: fb d2     mov   y,$d2+x
25d9: 36 5a ba  and   a,$ba5a+y
25dc: ff        stop
25dd: df        daa   a
25de: 25 fc fe  and   a,$fefc
25e1: 0d        push  psw
25e2: fa 8f d5  mov   ($d5),($8f)
25e5: 00        nop
25e6: 30 0f     bmi   $25f7
25e8: 7d        mov   a,x
25e9: 7f        reti
25ea: b0 bc     bcs   $25a8
25ec: ff        stop
25ed: ef        sleep
25ee: 25 e2 05  and   a,$05e2
25f1: ee        pop   y
25f2: fe fa     dbnz  y,$25ee
25f4: 8c d4 00  dec   $00d4
25f7: 34 23     and   a,$23+x
25f9: 7f        reti
25fa: 7f        reti
25fb: ff        stop
25fc: fe 25     dbnz  y,$2623
25fe: ea 80 fc  not1  $1f90,0
2601: fe 06     dbnz  y,$2609
2603: 0c 32 7d  asl   $7d32
2606: 00        nop
2607: 3e 46     cmp   x,$46
2609: 7f        reti
260a: 7f        reti
260b: fa 8f cf  mov   ($cf),($8f)
260e: 00        nop
260f: 3e dc     cmp   x,$dc
2611: ff        stop
2612: ff        stop
2613: 17 26     or    a,($26)+y
2615: 36 26 ea  and   a,$ea26+y
2618: 41        tcall 4
2619: fc        inc   y
261a: d6 0b e3  mov   $e30b+y,a
261d: 10 f2     bpl   $2611
261f: 3c        rol   a
2620: 3e 05     cmp   x,$05
2622: 7d        mov   a,x
2623: 7f        reti
2624: f3 00 05  bbc7  $00,$262c
2627: 24 a2     and   a,$a2
2629: fe 38     dbnz  y,$2663
262b: 1d        dec   x
262c: ff        stop
262d: fa 8a d1  mov   ($d1),($8a)
2630: 00        nop
2631: b8 d5 b8  sbc   $b8,#$d5
2634: ad ff     cmp   y,#$ff
2636: ea 41 fc  not1  $1f88,1
2639: fe 0b     dbnz  y,$2646
263b: e3 18 38  bbs7  $18,$2676
263e: 1d        dec   x
263f: 7d        mov   a,x
2640: 7f        reti
2641: fa 8a d1  mov   ($d1),($8a)
2644: 00        nop
2645: b8 d5 b8  sbc   $b8,#$d5
2648: ad ff     cmp   y,#$ff
264a: 4c 26 fc  lsr   $fc26
264d: e2 02     set7  $02
264f: 3b 19     rol   $19+x
2651: 7f        reti
2652: 7f        reti
2653: 3b 0a     rol   $0a+x
2655: e9 bb d5  mov   x,$d5bb
2658: bb c1     inc   $c1+x
265a: bb ad     inc   $ad+x
265c: ff        stop
265d: 5f 26 ea  jmp   $ea26

2660: ff        stop
2661: fc        inc   y
2662: f4 01     mov   a,$01+x
2664: 36 d7 7f  and   a,$7fd7+y
2667: 7f        reti
2668: fb d4     mov   y,$d4+x
266a: 36 3e b3  and   a,$b33e+y
266d: ff        stop
266e: 70 26     bvs   $2696
2670: ea ff fc  not1  $1f9f,7
2673: fe 02     dbnz  y,$2677
2675: 36 38 7f  and   a,$7f38+y
2678: 7f        reti
2679: 36 8c fb  and   a,$fb8c+y
267c: fb d2     mov   y,$d2+x
267e: 36 64 b2  and   a,$b264+y
2681: ff        stop
2682: 84 26     adc   a,$26
2684: ea ff fc  not1  $1f9f,7
2687: f2 0a     clr7  $0a
2689: 36 8d 7f  and   a,$7f8d+y
268c: 7f        reti
268d: fb d4     mov   y,$d4+x
268f: 36 5a b2  and   a,$b25a+y
2692: ff        stop
2693: 95 26 ea  adc   a,$ea26+x
2696: ff        stop
2697: fc        inc   y
2698: f0 0b     beq   $26a5
269a: 36 89 7f  and   a,$7f89+y
269d: 7f        reti
269e: fb d4     mov   y,$d4+x
26a0: 36 46 b2  and   a,$b246+y
26a3: ff        stop
26a4: a6        sbc   a,(x)
26a5: 26        and   a,(x)
26a6: ea ff fc  not1  $1f9f,7
26a9: f0 0c     beq   $26b7
26ab: 36 8e 7f  and   a,$7f8e+y
26ae: 7f        reti
26af: fb d4     mov   y,$d4+x
26b1: 36 58 b2  and   a,$b258+y
26b4: ff        stop
26b5: b7 26     sbc   a,($26)+y
26b7: ea ff fc  not1  $1f9f,7
26ba: f0 0d     beq   $26c9
26bc: 36 53 7f  and   a,$7f53+y
26bf: 7f        reti
26c0: fb d4     mov   y,$d4+x
26c2: 36 44 b2  and   a,$b244+y
26c5: ff        stop
26c6: c8 26     cmp   x,#$26
26c8: ea ff fc  not1  $1f9f,7
26cb: f0 0e     beq   $26db
26cd: 36 8a 7f  and   a,$7f8a+y
26d0: 7f        reti
26d1: fb d4     mov   y,$d4+x
26d3: 36 46 c1  and   a,$c146+y
26d6: ff        stop
26d7: d9 26     mov   $26+y,x
26d9: ea ff fc  not1  $1f9f,7
26dc: f0 0f     beq   $26ed
26de: 36 5a 7f  and   a,$7f5a+y
26e1: 7f        reti
26e2: fb d4     mov   y,$d4+x
26e4: 36 2d b3  and   a,$b32d+y
26e7: ff        stop
26e8: ea 26 ea  not1  $1d44,6
26eb: ff        stop
26ec: fc        inc   y
26ed: ec 03 36  mov   y,$3603
26f0: dc        dec   y
26f1: 7f        reti
26f2: 7f        reti
26f3: fb d4     mov   y,$d4+x
26f5: 36 46 b4  and   a,$b446+y
26f8: ff        stop
26f9: fb 26     mov   y,$26+x
26fb: ea ff fc  not1  $1f9f,7
26fe: fc        inc   y
26ff: 04 36     or    a,$36
2701: ca 7f 7f  mov1  $0fef,7,c
2704: fb d4     mov   y,$d4+x
2706: 36 29 b7  and   a,$b729+y
2709: ff        stop
270a: 0c 27 ea  asl   $ea27
270d: ff        stop
270e: fc        inc   y
270f: e0        clrv
2710: 00        nop
2711: 36 9e 7f  and   a,$7f9e+y
2714: 7f        reti
2715: fb 34     mov   y,$34+x
2717: 36 78 ee  and   a,$ee78+y
271a: ff        stop
271b: 1d        dec   x
271c: 27 ea     and   a,($ea+x)
271e: 7d        mov   a,x
271f: fc        inc   y
2720: de 07 ed  cbne  $07+x,$2710
2723: 8a 39 5a  eor1  c,$0b47,1
2726: 7f        reti
2727: 7f        reti
2728: fb 10     mov   y,$10+x
272a: 39        and   (x),(y)
272b: 96 ff ff  adc   a,$ffff+y
272e: 32 27     clr1  $27
2730: 44 27     eor   a,$27
2732: fc        inc   y
2733: f0 02     beq   $2737
2735: e3 16 33  bbs7  $16,$276b
2738: 0c 7f 7f  asl   $7f7f
273b: fa 8f d3  mov   ($d3),($8f)
273e: 00        nop
273f: 33 32 7d  bbc1  $32,$27bf
2742: 7f        reti
2743: ff        stop
2744: fc        inc   y
2745: f0 02     beq   $2749
2747: e3 12 39  bbs7  $12,$2783
274a: 0c 7f 7f  asl   $7f7f
274d: fa 8f d3  mov   ($d3),($8f)
2750: 00        nop
2751: 39        and   (x),(y)
2752: 32 ff     clr1  $ff
2754: ff        stop
2755: 57 27     eor   a,($27)+y
2757: fc        inc   y
2758: e2 07     set7  $07
275a: 39        and   (x),(y)
275b: 0c 7f 7f  asl   $7f7f
275e: fa 8f d4  mov   ($d4),($8f)
2761: 00        nop
2762: 39        and   (x),(y)
2763: 1e ff ff  cmp   x,$ffff
2766: 6a 27 7d  and1  c,!($0fa4,7)
2769: 27 fc     and   a,($fc+x)
276b: d6 12 fa  mov   $fa12+y,a
276e: ff        stop
276f: e2 00     set7  $00
2771: e3 16 26  bbs7  $16,$279a
2774: 07 7f     or    a,($7f+x)
2776: 7f        reti
2777: fb ce     mov   y,$ce+x
2779: e1        tcall 14
277a: 82 7f     set4  $7f
277c: ff        stop
277d: fc        inc   y
277e: d6 12 fa  mov   $fa12+y,a
2781: ff        stop
2782: e2 00     set7  $00
2784: e3 12 32  bbs7  $12,$27b9
2787: 07 7f     or    a,($7f+x)
2789: 7f        reti
278a: fb ce     mov   y,$ce+x
278c: e1        tcall 14
278d: 82 7f     set4  $7f
278f: ff        stop
2790: 92 27     clr4  $27
2792: fc        inc   y
2793: e0        clrv
2794: 03 fa ff  bbs0  $fa,$2796
2797: e2 00     set7  $00
2799: 32 07     clr1  $07
279b: 7f        reti
279c: 7f        reti
279d: fa 8f ce  mov   ($ce),($8f)
27a0: 00        nop
27a1: e1        tcall 14
27a2: 82 7f     set4  $7f
27a4: ff        stop
27a5: a7 27     sbc   a,($27+x)
27a7: ea ff fc  not1  $1f9f,7
27aa: f0 0d     beq   $27b9
27ac: fa ff e2  mov   ($e2),($ff)
27af: 00        nop
27b0: 3c        rol   a
27b1: 0f        brk
27b2: 7f        reti
27b3: 7f        reti
27b4: f3 00 0f  bbc7  $00,$27c6
27b7: 20        clrp
27b8: 23 fe ea  bbs1  $fe,$27a5
27bb: 3c        rol   a
27bc: e4 00     mov   a,$00
27be: 3a 7f     incw  $7f
27c0: fb d0     mov   y,$d0+x
27c2: 20        clrp
27c3: 50 ff     bvc   $27c4
27c5: ff        stop
27c6: c8 27     cmp   x,#$27
27c8: fc        inc   y
27c9: e0        clrv
27ca: 10 fa     bpl   $27c6
27cc: ff        stop
27cd: c2 ad     set6  $ad
27cf: 32 0f     clr1  $0f
27d1: 4c 7f 32  lsr   $327f
27d4: 10 ff     bpl   $27d5
27d6: 32 11     clr1  $11
27d8: ff        stop
27d9: fa ff e2  mov   ($e2),($ff)
27dc: 00        nop
27dd: 32 07     clr1  $07
27df: 7f        reti
27e0: 7f        reti
27e1: fa 8f ce  mov   ($ce),($8f)
27e4: 00        nop
27e5: e1        tcall 14
27e6: 82 7f     set4  $7f
27e8: ff        stop
27e9: eb 27     mov   y,$27
27eb: ea ff fc  not1  $1f9f,7
27ee: e8 01     mov   a,#$01
27f0: 36 5d 7f  and   a,$7f5d+y
27f3: 7f        reti
27f4: fb d4     mov   y,$d4+x
27f6: 36 2d b3  and   a,$b32d+y
27f9: ff        stop
27fa: fc        inc   y
27fb: 27 ea     and   a,($ea+x)
27fd: ff        stop
27fe: fc        inc   y
27ff: e8 02     mov   a,#$02
2801: 36 6e 7f  and   a,$7f6e+y
2804: 7f        reti
2805: fb d4     mov   y,$d4+x
2807: 36 46 b4  and   a,$b446+y
280a: ff        stop
280b: 0d        push  psw
280c: 28 ea     and   a,#$ea
280e: ff        stop
280f: fc        inc   y
2810: ec 03 36  mov   y,$3603
2813: 71        tcall 7
2814: 7f        reti
2815: 7f        reti
2816: fb d4     mov   y,$d4+x
2818: 36 32 b3  and   a,$b332+y
281b: ff        stop
281c: 1e 28 ea  cmp   x,$ea28
281f: ff        stop
2820: fc        inc   y
2821: e6        mov   a,(x)
2822: 04 36     or    a,$36
2824: 8c 7f 7f  dec   $7f7f
2827: fb d4     mov   y,$d4+x
2829: 36 2d b4  and   a,$b42d+y
282c: ff        stop
282d: 2f 28     bra   $2857
282f: ea ff fc  not1  $1f9f,7
2832: e6        mov   a,(x)
2833: 02 36     set0  $36
2835: cf        mul   ya
2836: 7f        reti
2837: 7f        reti
2838: fb d4     mov   y,$d4+x
283a: 36 63 b7  and   a,$b763+y
283d: ff        stop
283e: 40        setp
283f: 28 ea     and   a,#$ea
2841: ff        stop
2842: fc        inc   y
2843: e8 05     mov   a,#$05
2845: 36 3c 7f  and   a,$7f3c+y
2848: 7f        reti
2849: fb d4     mov   y,$d4+x
284b: b6 b3 ff  sbc   a,$ffb3+y
284e: 50 28     bvc   $2878
2850: ea ff fc  not1  $1f9f,7
2853: e8 06     mov   a,#$06
2855: 36 8f 7f  and   a,$7f8f+y
2858: 7f        reti
2859: fb d4     mov   y,$d4+x
285b: 36 3c b3  and   a,$b33c+y
285e: ff        stop
285f: 61        tcall 6
2860: 28 ea     and   a,#$ea
2862: ff        stop
2863: fc        inc   y
2864: fe 03     dbnz  y,$2869
2866: 39        and   (x),(y)
2867: 6e 7f 7f  dbnz  $7f,$28e9
286a: 39        and   (x),(y)
286b: 78 f6 fb  cmp   $fb,#$f6
286e: d2 39     clr6  $39
2870: 84 b5     adc   a,$b5
2872: ff        stop
2873: 75 28 ea  cmp   a,$ea28+x
2876: ff        stop
2877: fc        inc   y
2878: ea 07 36  not1  $06c0,7
287b: 49 7f 7f  eor   ($7f),($7f)
287e: fb d4     mov   y,$d4+x
2880: 36 41 b3  and   a,$b341+y
2883: ff        stop
2884: 86        adc   a,(x)
2885: 28 ea     and   a,#$ea
2887: 78 fc fe  cmp   $fe,#$fc
288a: 0c fa ff  asl   $fffa
288d: d0 00     bne   $288f
288f: e4 18     mov   a,$18
2891: 3c        rol   a
2892: 41        tcall 4
2893: 0c 09 7d  asl   $7d09
2896: 00        nop
2897: 42 aa     set2  $aa
2899: 7f        reti
289a: 7f        reti
289b: ff        stop
289c: 9e        div   ya,x
289d: 28 fc     and   a,#$fc
289f: d8 0e     mov   $0e,x
28a1: fb d4     mov   y,$d4+x
28a3: 35 23 7f  and   a,$7f23+x
28a6: 7f        reti
28a7: ff        stop
28a8: ac 28 be  inc   $be28
28ab: 28 fc     and   a,#$fc
28ad: fe 02     dbnz  y,$28b1
28af: e3 10 75  bbs7  $10,$2927
28b2: e0        clrv
28b3: 03 34 1a  bbs0  $34,$28d0
28b6: 7f        reti
28b7: 7f        reti
28b8: fb cf     mov   y,$cf+x
28ba: 34 78     and   a,$78+x
28bc: ff        stop
28bd: ff        stop
28be: fc        inc   y
28bf: fe 02     dbnz  y,$28c3
28c1: e3 18 34  bbs7  $18,$28f8
28c4: 1a 7f     decw  $7f
28c6: 7f        reti
28c7: fb cf     mov   y,$cf+x
28c9: 34 78     and   a,$78+x
28cb: ff        stop
28cc: ff        stop
28cd: cf        mul   ya
28ce: 28 ea     and   a,#$ea
28d0: ff        stop
28d1: fc        inc   y
28d2: fe 10     dbnz  y,$28e4
28d4: 36 8c 7f  and   a,$7f8c+y
28d7: 7f        reti
28d8: fb d4     mov   y,$d4+x
28da: 36 3f b7  and   a,$b73f+y
28dd: ff        stop
28de: e0        clrv
28df: 28 ea     and   a,#$ea
28e1: ff        stop
28e2: fc        inc   y
28e3: e8 03     mov   a,#$03
28e5: 39        and   (x),(y)
28e6: d0 7f     bne   $2967
28e8: 7f        reti
28e9: fb d4     mov   y,$d4+x
28eb: 39        and   (x),(y)
28ec: 37 ba     and   a,($ba)+y
28ee: ff        stop
28ef: f1        tcall 15
28f0: 28 fc     and   a,#$fc
28f2: d2 11     clr6  $11
28f4: 35 26 7f  and   a,$7f26+x
28f7: 7f        reti
28f8: fb d2     mov   y,$d2+x
28fa: 35 35 ff  and   a,$ff35+x
28fd: f3 00 24  bbc7  $00,$2924
2900: 34 ff     and   a,$ff+x
2902: ff        stop
2903: ff        stop
2904: 08 29     or    a,#$29
2906: 17 29     or    a,($29)+y
2908: fc        inc   y
2909: d8 01     mov   $01,x
290b: e3 12 27  bbs7  $12,$2935
290e: 1f 7f 7f  jmp   ($7f7f+x)

2911: ea 0a 1e  not1  $03c1,2
2914: ff        stop
2915: ff        stop
2916: ff        stop
2917: fc        inc   y
2918: d8 01     mov   $01,x
291a: e3 16 2d  bbs7  $16,$294a
291d: 1f 7f 7f  jmp   ($7f7f+x)

2920: ea 0a 27  not1  $04e1,2
2923: ff        stop
2924: ff        stop
2925: ff        stop
2926: 2a 29 3e  or1   c,!($07c5,1)
2929: 29 fc e6  and   ($e6),($fc)
292c: 02 fa     set0  $fa
292e: 87 d1     adc   a,($d1+x)
2930: 00        nop
2931: e3 12 30  bbs7  $12,$2964
2934: 50 7f     bvc   $29b5
2936: 7f        reti
2937: f3 00 24  bbc7  $00,$295e
293a: 1c        asl   a
293b: dd        mov   a,y
293c: ff        stop
293d: ff        stop
293e: fc        inc   y
293f: e6        mov   a,(x)
2940: 02 fa     set0  $fa
2942: 87 d1     adc   a,($d1+x)
2944: 00        nop
2945: e3 16 33  bbs7  $16,$297b
2948: 50 7f     bvc   $29c9
294a: 7f        reti
294b: f3 00 20  bbc7  $00,$296e
294e: 21        tcall 2
294f: dc        dec   y
2950: ff        stop
2951: ff        stop
2952: 54 29     eor   a,$29+x
2954: ea 2a fc  not1  $1f85,2
2957: f8 04     mov   x,$04
2959: 18 ff 7f  or    $7f,#$ff
295c: 7f        reti
295d: f3 00 ff  bbc7  $00,$295f
2960: 38 05 00  and   $00,#$05
2963: fb cc     mov   y,$cc+x
2965: 37 91     and   a,($91)+y
2967: f8 ff     mov   x,$ff
2969: 6b 29     ror   $29
296b: fc        inc   y
296c: f6 00 e3  mov   a,$e300+y
296f: 04 f8     or    a,$f8
2971: 14 01     or    a,$01+x
2973: 11        tcall 1
2974: c3 7f 7f  bbs6  $7f,$29f6
2977: f3 00 c3  bbc7  $00,$293d
297a: 46        eor   a,(x)
297b: 11        tcall 1
297c: 00        nop
297d: f8 24     mov   x,$24
297f: 01        tcall 0
2980: fa 8f cb  mov   ($cb),($8f)
2983: 00        nop
2984: 46        eor   a,(x)
2985: f0 ff     beq   $2986
2987: f3 00 b4  bbc7  $00,$293e
298a: 1d        dec   x
298b: f2 ff     clr7  $ff
298d: ff        stop
298e: 90 29     bcc   $29b9
2990: fc        inc   y
2991: f6 01 e3  mov   a,$e301+y
2994: 24 f8     and   a,$f8
2996: 14 fe     or    a,$fe+x
2998: 11        tcall 1
2999: b4 7f     sbc   a,$7f+x
299b: 7f        reti
299c: f3 00 b4  bbc7  $00,$2953
299f: 46        eor   a,(x)
29a0: 12 00     clr0  $00
29a2: f8 04     mov   x,$04
29a4: fe fa     dbnz  y,$29a0
29a6: 8f cb 00  mov   $00,#$cb
29a9: 46        eor   a,(x)
29aa: f0 ff     beq   $29ab
29ac: f3 00 b4  bbc7  $00,$2963
29af: 1d        dec   x
29b0: f2 ff     clr7  $ff
29b2: ff        stop
29b3: b5 29 ea  sbc   a,$ea29+x
29b6: 0a fc e8  or1   c,$1d1f,4
29b9: 02 fa     set0  $fa
29bb: 80        setc
29bc: e2 00     set7  $00
29be: 2f b4     bra   $2974
29c0: 7f        reti
29c1: 7f        reti
29c2: ff        stop
29c3: c5 29 fc  mov   $fc29,a
29c6: fe 00     dbnz  y,$29c8
29c8: 3b 0f     rol   $0f+x
29ca: 7f        reti
29cb: 7f        reti
29cc: fb d3     mov   y,$d3+x
29ce: e1        tcall 14
29cf: 2d        push  a
29d0: 7f        reti
29d1: ff        stop
29d2: d4 29     mov   $29+x,a
29d4: fc        inc   y
29d5: dc        dec   y
29d6: 04 38     or    a,$38
29d8: 1e 7f 7f  cmp   x,$7f7f
29db: ff        stop
29dc: de 29 fc  cbne  $29+x,$29db
29df: e6        mov   a,(x)
29e0: 05 30 49  or    a,$4930
29e3: 7f        reti
29e4: 7f        reti
29e5: fb d1     mov   y,$d1+x
29e7: 30 46     bmi   $2a2f
29e9: ff        stop
29ea: f3 00 50  bbc7  $00,$2a3d
29ed: 2b fc     rol   $fc
29ef: ff        stop
29f0: ff        stop
29f1: f3 29 ea  bbc7  $29,$29de
29f4: be        das   a
29f5: fc        inc   y
29f6: be        das   a
29f7: 0a fa 8c  or1   c,$119f,2
29fa: cf        mul   ya
29fb: 00        nop
29fc: 0c 08 7d  asl   $7d08
29ff: 00        nop
2a00: 40        setp
2a01: 24 7f     and   a,$7f
2a03: 7f        reti
2a04: c0        di
2a05: e9 c0 d5  mov   x,$d5c0
2a08: c0        di
2a09: c1        tcall 12
2a0a: c0        di
2a0b: ad ff     cmp   y,#$ff
2a0d: 0f        brk
2a0e: 2a fc fe  or1   c,!($1fdf,4)
2a11: 09 32 13  or    ($13),($32)
2a14: 7f        reti
2a15: 7f        reti
2a16: ff        stop
2a17: 19        or    (x),(y)
2a18: 2a ea 93  or1   c,!($127d,2)
2a1b: fc        inc   y
2a1c: ec 06 e4  mov   y,$e406
2a1f: 00        nop
2a20: 55 83 33  eor   a,$3383+x
2a23: 22 7d     set1  $7d
2a25: 7f        reti
2a26: f3 00 1e  bbc7  $00,$2a47
2a29: 3c        rol   a
2a2a: 2c 00 36  rol   $3600
2a2d: 11        tcall 1
2a2e: ad f3     cmp   y,#$f3
2a30: 00        nop
2a31: 0f        brk
2a32: 3c        rol   a
2a33: 3b 00     rol   $00+x
2a35: ff        stop
2a36: 3a 2a     incw  $2a
2a38: 56 2a ea  eor   a,$ea2a+y
2a3b: a0        ei
2a3c: fc        inc   y
2a3d: d0 0a     bne   $2a49
2a3f: 0c 13 7d  asl   $7d13
2a42: 00        nop
2a43: 73 e3 10  bbc3  $e3,$2a56
2a46: ed        notc
2a47: 8d e6     mov   y,#$e6
2a49: 3c        rol   a
2a4a: 0c 7d 7f  asl   $7f7d
2a4d: c1        tcall 12
2a4e: ff        stop
2a4f: c5 ff e7  mov   $e7ff,a
2a52: 04 e6     or    a,$e6
2a54: 00        nop
2a55: ff        stop
2a56: ea a0 fc  not1  $1f94,0
2a59: fc        inc   y
2a5a: 0a e3 18  or1   c,$031c,3
2a5d: ed        notc
2a5e: 8d 0c     mov   y,#$0c
2a60: 0b 7d     asl   $7d
2a62: 00        nop
2a63: e6        mov   a,(x)
2a64: 3c        rol   a
2a65: 0c 7d 7f  asl   $7f7d
2a68: c1        tcall 12
2a69: ff        stop
2a6a: c5 ff e7  mov   $e7ff,a
2a6d: 04 e6     or    a,$e6
2a6f: 00        nop
2a70: ff        stop
2a71: 73 2a ea  bbc3  $2a,$2a5e
2a74: 1e fc e6  cmp   x,$e6fc
2a77: 0c fb cb  asl   $cbfb
2a7a: 36 82 7f  and   a,$7f82+y
2a7d: 7f        reti
2a7e: ff        stop
2a7f: 81        tcall 8
2a80: 2a ea ff  or1   c,!($1ffd,2)
2a83: fc        inc   y
2a84: e4 00     mov   a,$00
2a86: 36 ff 7f  and   a,$7fff+y
2a89: 7f        reti
2a8a: 36 1a ff  and   a,$ff1a+y
2a8d: fb d2     mov   y,$d2+x
2a8f: 36 46 bc  and   a,$bc46+y
2a92: ff        stop
2a93: 97 2a     adc   a,($2a)+y
2a95: a7 2a     sbc   a,($2a+x)
2a97: fc        inc   y
2a98: d2 03     clr6  $03
2a9a: 39        and   (x),(y)
2a9b: 2e 7f 7f  cbne  $7f,$2b1d
2a9e: fa 8f cd  mov   ($cd),($8f)
2aa1: 00        nop
2aa2: 39        and   (x),(y)
2aa3: af        mov   (x)+,a
2aa4: 7d        mov   a,x
2aa5: 7f        reti
2aa6: ff        stop
2aa7: fc        inc   y
2aa8: d2 03     clr6  $03
2aaa: e0        clrv
2aab: 50 39     bvc   $2ae6
2aad: 2e 7f 7f  cbne  $7f,$2b2f
2ab0: fa 8f d0  mov   ($d0),($8f)
2ab3: 00        nop
2ab4: 39        and   (x),(y)
2ab5: 78 ff ff  cmp   $ff,#$ff
2ab8: ba 2a     movw  ya,$2a
2aba: fc        inc   y
2abb: fa 04 30  mov   ($30),($04)
2abe: 18 7f 7f  or    $7f,#$7f
2ac1: 30 15     bmi   $2ad8
2ac3: d5 30 14  mov   $1430+x,a
2ac6: ad ff     cmp   y,#$ff
2ac8: ca 2a fc  mov1  $1f85,2,c
2acb: dc        dec   y
2acc: 08 29     or    a,#$29
2ace: 0a 7d 7f  or1   c,$0fef,5
2ad1: ed        notc
2ad2: 87 a9     adc   a,($a9+x)
2ad4: e9 a9 d5  mov   x,$d5a9
2ad7: a9 c1 ff  sbc   ($ff),($c1)
2ada: de 2a ef  cbne  $2a+x,$2acc
2add: 2a ea 1e  or1   c,!($03dd,2)
2ae0: fc        inc   y
2ae1: dc        dec   y
2ae2: 0c e3 12  asl   $12e3
2ae5: 30 2f     bmi   $2b16
2ae7: 7f        reti
2ae8: 7f        reti
2ae9: fb cf     mov   y,$cf+x
2aeb: 30 34     bmi   $2b21
2aed: ee        pop   y
2aee: ff        stop
2aef: ea 1e fc  not1  $1f83,6
2af2: dc        dec   y
2af3: 0c e3 16  asl   $16e3
2af6: 37 2a     and   a,($2a)+y
2af8: 7f        reti
2af9: 7f        reti
2afa: fb cf     mov   y,$cf+x
2afc: 37 34     and   a,($34)+y
2afe: ee        pop   y
2aff: ff        stop
2b00: 02 2b     set0  $2b
2b02: ea 1e fc  not1  $1f83,6
2b05: ea 0b fb  not1  $1f61,3
2b08: ca 37 88  mov1  $1106,7,c
2b0b: 7f        reti
2b0c: 7f        reti
2b0d: ff        stop
2b0e: 10 2b     bpl   $2b3b
2b10: fc        inc   y
2b11: e4 0d     mov   a,$0d
2b13: 34 1a     and   a,$1a+x
2b15: 7f        reti
2b16: 7f        reti
2b17: fb d3     mov   y,$d3+x
2b19: 34 1e     and   a,$1e+x
2b1b: ee        pop   y
2b1c: ff        stop
2b1d: 1f 2b ea  jmp   ($ea2b+x)

2b20: 78 fc e8  cmp   $e8,#$fc
2b23: 08 3a     or    a,#$3a
2b25: 28 7f     and   a,#$7f
2b27: 7f        reti
2b28: fb d3     mov   y,$d3+x
2b2a: 3a 50     incw  $50
2b2c: ff        stop
2b2d: f3 00 28  bbc7  $00,$2b58
2b30: 37 f7     and   a,($f7)+y
2b32: ff        stop
2b33: ff        stop
2b34: 36 2b ea  and   a,$ea2b+y
2b37: 78 fc e2  cmp   $e2,#$fc
2b3a: 03 3a 44  bbs0  $3a,$2b81
2b3d: 7f        reti
2b3e: 7f        reti
2b3f: fb d5     mov   y,$d5+x
2b41: e1        tcall 14
2b42: 32 7f     clr1  $7f
2b44: ff        stop
2b45: 49 2b 58  eor   ($58),($2b)
2b48: 2b fc     rol   $fc
2b4a: c4 07     mov   $07,a
2b4c: 2e 85 7f  cbne  $85,$2bce
2b4f: 7f        reti
2b50: fa 8f cc  mov   ($cc),($8f)
2b53: 00        nop
2b54: 2e c6 ff  cbne  $c6,$2b56
2b57: ff        stop
2b58: fc        inc   y
2b59: d2 07     clr6  $07
2b5b: 36 85 7f  and   a,$7f85+y
2b5e: 7f        reti
2b5f: f3 1e 67  bbc7  $1e,$2bc9
2b62: 3f 05 00  call  $0005
2b65: fa 8f cc  mov   ($cc),($8f)
2b68: 00        nop
2b69: 3e 12     cmp   x,$12
2b6b: ff        stop
2b6c: f3 00 12  bbc7  $00,$2b81
2b6f: 42 0e     set2  $0e
2b71: 00        nop
2b72: e6        mov   a,(x)
2b73: 3e 12     cmp   x,$12
2b75: 7f        reti
2b76: 69 f3 00  cmp   ($00),($f3)
2b79: 14 42     or    a,$42+x
2b7b: 0c 00 e7  asl   $e700
2b7e: 0a 00 00  or1   c,$0000,0
2b81: ff        stop
2b82: 84 2b     adc   a,$2b
2b84: fc        inc   y
2b85: da 11     movw  $11,ya
2b87: fa 8f e2  mov   ($e2),($8f)
2b8a: 00        nop
2b8b: 34 17     and   a,$17+x
2b8d: 7f        reti
2b8e: 7f        reti
2b8f: f3 00 17  bbc7  $00,$2ba9
2b92: 40        setp
2b93: 21        tcall 2
2b94: 00        nop
2b95: fa 8f d0  mov   ($d0),($8f)
2b98: 00        nop
2b99: 40        setp
2b9a: 56 fb ff  eor   a,$fffb+y
2b9d: 9f        xcn   a
2b9e: 2b fc     rol   $fc
2ba0: fa 0e fa  mov   ($fa),($0e)
2ba3: 86        adc   a,(x)
2ba4: d2 00     clr6  $00
2ba6: 33 18 7f  bbc1  $18,$2c28
2ba9: 7f        reti
2baa: 33 12 e9  bbc1  $12,$2b96
2bad: b3 d5 ff  bbc5  $d5,$2baf
2bb0: b2 2b     clr5  $2b
2bb2: fc        inc   y
2bb3: fe 0f     dbnz  y,$2bc4
2bb5: fb d0     mov   y,$d0+x
2bb7: 34 5a     and   a,$5a+x
2bb9: 7f        reti
2bba: 7f        reti
2bbb: ff        stop
2bbc: be        das   a
2bbd: 2b ea     rol   $ea
2bbf: ff        stop
2bc0: fc        inc   y
2bc1: f4 00     mov   a,$00+x
2bc3: 36 bf 7f  and   a,$7fbf+y
2bc6: 7f        reti
2bc7: fb d4     mov   y,$d4+x
2bc9: 36 3f b3  and   a,$b33f+y
2bcc: ff        stop
2bcd: cf        mul   ya
2bce: 2b ea     rol   $ea
2bd0: ff        stop
2bd1: fc        inc   y
2bd2: fa 01 36  mov   ($36),($01)
2bd5: 84 7f     adc   a,$7f
2bd7: 7f        reti
2bd8: fb d4     mov   y,$d4+x
2bda: 36 37 b2  and   a,$b237+y
2bdd: ff        stop
2bde: e0        clrv
2bdf: 2b ea     rol   $ea
2be1: ff        stop
2be2: fc        inc   y
2be3: f4 02     mov   a,$02+x
2be5: 36 37 7f  and   a,$7f37+y
2be8: 7f        reti
2be9: fb d4     mov   y,$d4+x
2beb: 36 2d b2  and   a,$b22d+y
2bee: ff        stop
2bef: f1        tcall 15
2bf0: 2b ea     rol   $ea
2bf2: ff        stop
2bf3: fc        inc   y
2bf4: e8 03     mov   a,#$03
2bf6: 36 45 7f  and   a,$7f45+y
2bf9: 7f        reti
2bfa: fb d4     mov   y,$d4+x
2bfc: 36 3c b4  and   a,$b43c+y
2bff: ff        stop
2c00: 02 2c     set0  $2c
2c02: ea ff fc  not1  $1f9f,7
2c05: f6 04 36  mov   a,$3604+y
2c08: 42 7f     set2  $7f
2c0a: 7f        reti
2c0b: fb d4     mov   y,$d4+x
2c0d: 36 28 b2  and   a,$b228+y
2c10: ff        stop
2c11: 13 2c ea  bbc0  $2c,$2bfe
2c14: ff        stop
2c15: fc        inc   y
2c16: fe 01     dbnz  y,$2c19
2c18: 36 a8 7f  and   a,$7fa8+y
2c1b: 7f        reti
2c1c: fb d4     mov   y,$d4+x
2c1e: 36 60 c6  and   a,$c660+y
2c21: ff        stop
2c22: 24 2c     and   a,$2c
2c24: ea ff fc  not1  $1f9f,7
2c27: ec 05 36  mov   y,$3605
2c2a: 64 7f     cmp   a,$7f
2c2c: 7f        reti
2c2d: fb d4     mov   y,$d4+x
2c2f: 36 62 b4  and   a,$b462+y
2c32: ff        stop
2c33: 35 2c ea  and   a,$ea2c+x
2c36: ff        stop
2c37: fc        inc   y
2c38: fe 02     dbnz  y,$2c3c
2c3a: 36 c1 7f  and   a,$7fc1+y
2c3d: 7f        reti
2c3e: fb d4     mov   y,$d4+x
2c40: 36 21 b2  and   a,$b221+y
2c43: ff        stop
2c44: 46        eor   a,(x)
2c45: 2c ea ff  rol   $ffea
2c48: fc        inc   y
2c49: ea 06 36  not1  $06c0,6
2c4c: 67 7f     cmp   a,($7f+x)
2c4e: 7f        reti
2c4f: fb d4     mov   y,$d4+x
2c51: 36 39 b3  and   a,$b339+y
2c54: ff        stop
2c55: 57 2c     eor   a,($2c)+y
2c57: ea ff fc  not1  $1f9f,7
2c5a: f0 07     beq   $2c63
2c5c: 36 65 7f  and   a,$7f65+y
2c5f: 7f        reti
2c60: fb d4     mov   y,$d4+x
2c62: 36 56 b2  and   a,$b256+y
2c65: ff        stop
2c66: 68 2c     cmp   a,#$2c
2c68: ea 64 fc  not1  $1f8c,4
2c6b: bc        inc   a
2c6c: 06        or    a,(x)
2c6d: 0c 06 7d  asl   $7d06
2c70: 00        nop
2c71: 41        tcall 4
2c72: 10 f8     bpl   $2c6c
2c74: c1        tcall 12
2c75: ff        stop
2c76: c1        tcall 12
2c77: 7f        reti
2c78: 6e fb d6  dbnz  $fb,$2c51
2c7b: 41        tcall 4
2c7c: 1e d5 ff  cmp   x,$ffd5
2c7f: 81        tcall 8
2c80: 2c fc fe  rol   $fefc
2c83: 01        tcall 0
2c84: 0c 07 7d  asl   $7d07
2c87: 00        nop
2c88: 46        eor   a,(x)
2c89: 0e ff ed  tset1 $edff
2c8c: 8c c6 ad  dec   $adc6
2c8f: ff        stop
2c90: 92 2c     clr4  $2c
2c92: ea 78 fc  not1  $1f8f,0
2c95: fe 00     dbnz  y,$2c97
2c97: e4 3c     mov   a,$3c
2c99: 8c 17 34  dec   $3417
2c9c: 91        tcall 9
2c9d: 7f        reti
2c9e: 7f        reti
2c9f: fa 8f d3  mov   ($d3),($8f)
2ca2: 00        nop
2ca3: 34 5a     and   a,$5a+x
2ca5: ff        stop
2ca6: f3 00 49  bbc7  $00,$2cf2
2ca9: 2f f8     bra   $2ca3
2cab: ff        stop
2cac: ff        stop
2cad: b1        tcall 11
2cae: 2c ca 2c  rol   $2cca
2cb1: ea ff fc  not1  $1f9f,7
2cb4: c0        di
2cb5: 01        tcall 0
2cb6: e3 1a f2  bbs7  $1a,$2cab
2cb9: 1a 0c     decw  $0c
2cbb: 14 7d     or    a,$7d+x
2cbd: 00        nop
2cbe: e0        clrv
2cbf: 0a 3b d2  or1   c,$1a47,3
2cc2: 7f        reti
2cc3: 7f        reti
2cc4: fb d4     mov   y,$d4+x
2cc6: 3b 78     rol   $78+x
2cc8: ad ff     cmp   y,#$ff
2cca: ea ff fc  not1  $1f9f,7
2ccd: d6 01 e3  mov   $e301+y,a
2cd0: 0e 0c 14  tset1 $140c
2cd3: 7d        mov   a,x
2cd4: 00        nop
2cd5: 3b d7     rol   $d7+x
2cd7: 7f        reti
2cd8: 7f        reti
2cd9: fb d4     mov   y,$d4+x
2cdb: 3b 7c     rol   $7c+x
2cdd: b0 ff     bcs   $2cde
2cdf: e1        tcall 14
2ce0: 2c ea ff  rol   $ffea
2ce3: fc        inc   y
2ce4: f0 09     beq   $2cef
2ce6: 3c        rol   a
2ce7: 08 7f     or    a,#$7f
2ce9: 7f        reti
2cea: f3 00 08  bbc7  $00,$2cf5
2ced: 24 00     and   a,$00
2cef: fd        mov   y,a
2cf0: e5 16 03  mov   a,$0316
2cf3: 00        nop
2cf4: 33 04 ff  bbc1  $04,$2cf6
2cf7: b0 ff     bcs   $2cf8
2cf9: 31        tcall 3
2cfa: 0a ff b0  or1   c,$161f,7
2cfd: ff        stop
2cfe: b1        tcall 11
2cff: ff        stop
2d00: fb d2     mov   y,$d2+x
2d02: e6        mov   a,(x)
2d03: 31        tcall 3
2d04: 14 7f     or    a,$7f+x
2d06: 7f        reti
2d07: b0 ff     bcs   $2d08
2d09: b2 ff     clr5  $ff
2d0b: e7 04     mov   a,($04+x)
2d0d: 00        nop
2d0e: 00        nop
2d0f: ff        stop
2d10: 12 2d     clr0  $2d
2d12: ea e6 fc  not1  $1f9c,6
2d15: e6        mov   a,(x)
2d16: 0f        brk
2d17: 0c 0a 7d  asl   $7d0a
2d1a: 00        nop
2d1b: 41        tcall 4
2d1c: 08 7f     or    a,#$7f
2d1e: 7f        reti
2d1f: 42 18     set2  $18
2d21: 7d        mov   a,x
2d22: 7f        reti
2d23: 41        tcall 4
2d24: 08 7f     or    a,#$7f
2d26: 5f 42 18  jmp   $1842

2d29: 7d        mov   a,x
2d2a: 5f 41 08  jmp   $0841

2d2d: 7f        reti
2d2e: 4b 42     lsr   $42
2d30: 18 7d 4b  or    $4b,#$7d
2d33: 41        tcall 4
2d34: 08 7f     or    a,#$7f
2d36: 37 42     and   a,($42)+y
2d38: 18 b7 ff  or    $ff,#$b7
2d3b: 3d        inc   x
2d3c: 2d        push  a
2d3d: ea ff fc  not1  $1f9f,7
2d40: fe 03     dbnz  y,$2d45
2d42: e5 ff 38  mov   a,$38ff
2d45: 00        nop
2d46: 3d        inc   x
2d47: 03 7f 7f  bbs0  $7f,$2dc9
2d4a: f3 00 03  bbc7  $00,$2d50
2d4d: 1e ab f5  cmp   x,$f5ab
2d50: e5 16 06  mov   a,$0616
2d53: 00        nop
2d54: 3d        inc   x
2d55: 04 ff     or    a,$ff
2d57: ba ff     movw  ya,$ff
2d59: 3b 08     rol   $08+x
2d5b: ff        stop
2d5c: 3a 09     incw  $09
2d5e: ff        stop
2d5f: bb ff     inc   $ff+x
2d61: fb d2     mov   y,$d2+x
2d63: e6        mov   a,(x)
2d64: 3b 0a     rol   $0a+x
2d66: 7f        reti
2d67: 7f        reti
2d68: 3a 08     incw  $08
2d6a: ff        stop
2d6b: 3c        rol   a
2d6c: 0d        push  psw
2d6d: ff        stop
2d6e: e7 07     mov   a,($07+x)
2d70: 00        nop
2d71: 00        nop
2d72: ff        stop
2d73: 75 2d fc  cmp   a,$fc2d+x
2d76: ce        pop   x
2d77: 03 39 37  bbs0  $39,$2db1
2d7a: 7d        mov   a,x
2d7b: 7f        reti
2d7c: b8 e9 b9  sbc   $b9,#$e9
2d7f: d5 39 3c  mov   $3c39+x,a
2d82: c1        tcall 12
2d83: ff        stop
2d84: 86        adc   a,(x)
2d85: 2d        push  a
2d86: ea ff fc  not1  $1f9f,7
2d89: ea 01 36  not1  $06c0,1
2d8c: 42 7f     set2  $7f
2d8e: 7f        reti
2d8f: fb d4     mov   y,$d4+x
2d91: 36 41 b7  and   a,$b741+y
2d94: ff        stop
2d95: 97 2d     adc   a,($2d)+y
2d97: ea ff fc  not1  $1f9f,7
2d9a: ea 02 36  not1  $06c0,2
2d9d: 8e        pop   psw
2d9e: 7f        reti
2d9f: 7f        reti
2da0: fb d4     mov   y,$d4+x
2da2: 36 66 b7  and   a,$b766+y
2da5: ff        stop
2da6: a8 2d     sbc   a,#$2d
2da8: ea ff fc  not1  $1f9f,7
2dab: f4 03     mov   a,$03+x
2dad: 36 8d 7f  and   a,$7f8d+y
2db0: 7f        reti
2db1: fb d4     mov   y,$d4+x
2db3: 36 6b b2  and   a,$b26b+y
2db6: ff        stop
2db7: b9        sbc   (x),(y)
2db8: 2d        push  a
2db9: ea ff fc  not1  $1f9f,7
2dbc: ec 04 36  mov   y,$3604
2dbf: 8e        pop   psw
2dc0: 7f        reti
2dc1: 7f        reti
2dc2: fb d2     mov   y,$d2+x
2dc4: 36 74 b2  and   a,$b274+y
2dc7: ff        stop
2dc8: ca 2d ea  mov1  $1d45,5,c
2dcb: ff        stop
2dcc: fc        inc   y
2dcd: ec 05 36  mov   y,$3605
2dd0: 72 7f     clr3  $7f
2dd2: 7f        reti
2dd3: fb d4     mov   y,$d4+x
2dd5: 36 66 b2  and   a,$b266+y
2dd8: ff        stop
2dd9: db 2d     mov   $2d+x,y
2ddb: ea ff fc  not1  $1f9f,7
2dde: fe 04     dbnz  y,$2de4
2de0: 36 d3 7f  and   a,$7fd3+y
2de3: 7f        reti
2de4: fb d4     mov   y,$d4+x
2de6: 36 75 bf  and   a,$bf75+y
2de9: ff        stop
2dea: ec 2d ea  mov   y,$ea2d
2ded: ff        stop
2dee: fc        inc   y
2def: ea 06 36  not1  $06c0,6
2df2: 4a 7f 7f  and1  c,$0fef,7
2df5: fb d4     mov   y,$d4+x
2df7: 36 48 b2  and   a,$b248+y
2dfa: ff        stop
2dfb: fd        mov   y,a
2dfc: 2d        push  a
2dfd: ea ff fc  not1  $1f9f,7
2e00: ec 00 39  mov   y,$3900
2e03: df        daa   a
2e04: 7f        reti
2e05: 7f        reti
2e06: fb d4     mov   y,$d4+x
2e08: 39        and   (x),(y)
2e09: 49 b7 ff  eor   ($ff),($b7)
2e0c: 0e 2e ea  tset1 $ea2e
2e0f: ff        stop
2e10: fc        inc   y
2e11: ec 07 36  mov   y,$3607
2e14: 8f 7f 7f  mov   $7f,#$7f
2e17: fb d4     mov   y,$d4+x
2e19: 36 55 b3  and   a,$b355+y
2e1c: ff        stop
2e1d: 1f 2e ea  jmp   ($ea2e+x)

2e20: 8c fc e4  dec   $e4fc
2e23: 05 34 1e  or    a,$1e34
2e26: 7d        mov   a,x
2e27: 7f        reti
2e28: f3 00 0a  bbc7  $00,$2e35
2e2b: 35 0e 00  and   a,$000e+x
2e2e: 36 23 e9  and   a,$e923+y
2e31: b6 d5 b7  sbc   a,$b7d5+y
2e34: c1        tcall 12
2e35: b7 ad     sbc   a,($ad)+y
2e37: ff        stop
2e38: 3a 2e     incw  $2e
2e3a: ea 7d fc  not1  $1f8f,5
2e3d: da 05     movw  $05,ya
2e3f: ed        notc
2e40: 8a 2c 23  eor1  c,$0465,4
2e43: 7d        mov   a,x
2e44: 7f        reti
2e45: ed        notc
2e46: 8f 2c 1b  mov   $1b,#$2c
2e49: e4 ac     mov   a,$ac
2e4b: d0 ac     bne   $2df9
2e4d: bc        inc   a
2e4e: ac ad ff  inc   $ffad
2e51: 53 2e ea  bbc2  $2e,$2e3e
2e54: 8c fc e6  dec   $e6fc
2e57: 06        or    a,(x)
2e58: 33 32 7d  bbc1  $32,$2ed8
2e5b: 7f        reti
2e5c: 33 2b df  bbc1  $2b,$2e3e
2e5f: b3 cb b3  bbc5  $cb,$2e15
2e62: b7 b3     sbc   a,($b3)+y
2e64: ad ff     cmp   y,#$ff
2e66: 68 2e     cmp   a,#$2e
2e68: ea aa fc  not1  $1f95,2
2e6b: b0 09     bcs   $2e76
2e6d: e6        mov   a,(x)
2e6e: 49 01 7d  eor   ($7d),($01)
2e71: 7f        reti
2e72: e7 2e     mov   a,($2e+x)
2e74: 00        nop
2e75: f9 e6     mov   x,$e6+y
2e77: 49 01 7d  eor   ($7d),($01)
2e7a: 55 e7 2e  eor   a,$2ee7+x
2e7d: 00        nop
2e7e: f9 e6     mov   x,$e6+y
2e80: 49 01 7d  eor   ($7d),($01)
2e83: 41        tcall 4
2e84: e7 2e     mov   a,($2e+x)
2e86: 00        nop
2e87: f9 ff     mov   x,$ff+y
2e89: 8b 2e     dec   $2e
2e8b: ea 78 fc  not1  $1f8f,0
2e8e: fe 19     dbnz  y,$2ea9
2e90: 45 3c 7d  eor   a,$7d3c
2e93: 7f        reti
2e94: f3 00 3c  bbc7  $00,$2ed3
2e97: 30 d6     bmi   $2e6f
2e99: ff        stop
2e9a: 32 1e     clr1  $1e
2e9c: ee        pop   y
2e9d: f3 00 1e  bbc7  $00,$2ebe
2ea0: 30 f8     bmi   $2e9a
2ea2: ff        stop
2ea3: b2 df     clr5  $df
2ea5: f3 00 1e  bbc7  $00,$2ec6
2ea8: 30 f8     bmi   $2ea2
2eaa: ff        stop
2eab: b2 cb     clr5  $cb
2ead: f3 00 1e  bbc7  $00,$2ece
2eb0: 30 f8     bmi   $2eaa
2eb2: ff        stop
2eb3: b2 c1     clr5  $c1
2eb5: f3 00 1e  bbc7  $00,$2ed6
2eb8: 30 f8     bmi   $2eb2
2eba: ff        stop
2ebb: b2 b7     clr5  $b7
2ebd: f3 00 1e  bbc7  $00,$2ede
2ec0: 30 f8     bmi   $2eba
2ec2: ff        stop
2ec3: b2 ad     clr5  $ad
2ec5: f3 00 1e  bbc7  $00,$2ee6
2ec8: 30 f8     bmi   $2ec2
2eca: ff        stop
2ecb: ff        stop
2ecc: ce        pop   x
2ecd: 2e ea 78  cbne  $ea,$2f48
2ed0: fc        inc   y
2ed1: f0 19     beq   $2eec
2ed3: e5 fd 4f  mov   a,$4ffd
2ed6: 00        nop
2ed7: 45 14 7d  eor   a,$7d14
2eda: 7f        reti
2edb: f3 00 19  bbc7  $00,$2ef7
2ede: 2d        push  a
2edf: 8d ff     mov   y,#$ff
2ee1: 33 19 ff  bbc1  $19,$2ee3
2ee4: b3 ee b3  bbc5  $ee,$2e9a
2ee7: df        daa   a
2ee8: b3 cb b3  bbc5  $cb,$2e9e
2eeb: c1        tcall 12
2eec: b3 b7 b3  bbc5  $b7,$2ea2
2eef: ad ff     cmp   y,#$ff
2ef1: f3 2e ea  bbc7  $2e,$2ede
2ef4: 78 fc fe  cmp   $fe,#$fc
2ef7: 19        or    (x),(y)
2ef8: 52 1e     clr2  $1e
2efa: 7f        reti
2efb: 7f        reti
2efc: f3 00 1e  bbc7  $00,$2f1d
2eff: 2e 70 ff  cbne  $70,$2f01
2f02: e5 ff 08  mov   a,$08ff
2f05: 00        nop
2f06: fa 8f d0  mov   ($d0),($8f)
2f09: 00        nop
2f0a: 2e 96 ff  cbne  $96,$2f0c
2f0d: ff        stop
2f0e: 12 2f     clr0  $2f
2f10: 47 2f     eor   a,($2f+x)
2f12: ea 82 fc  not1  $1f90,2
2f15: fe 09     dbnz  y,$2f20
2f17: 27 08     and   a,($08+x)
2f19: 7d        mov   a,x
2f1a: 7f        reti
2f1b: f3 00 08  bbc7  $00,$2f26
2f1e: 01        tcall 0
2f1f: 78 fd 24  cmp   $24,#$fd
2f22: 3c        rol   a
2f23: e9 f3 00  mov   x,$00f3
2f26: 3c        rol   a
2f27: 1f f6 ff  jmp   ($fff6+x)

2f2a: a4 d5     sbc   a,$d5
2f2c: f3 00 3c  bbc7  $00,$2f6b
2f2f: 1f f6 ff  jmp   ($fff6+x)

2f32: a4 b7     sbc   a,$b7
2f34: f3 00 3c  bbc7  $00,$2f73
2f37: 1f f6 ff  jmp   ($fff6+x)

2f3a: fa 8f d0  mov   ($d0),($8f)
2f3d: 00        nop
2f3e: a4 ad     sbc   a,$ad
2f40: f3 00 3c  bbc7  $00,$2f7f
2f43: 1f f6 ff  jmp   ($fff6+x)

2f46: ff        stop
2f47: ea 82 fc  not1  $1f90,2
2f4a: fe 19     dbnz  y,$2f65
2f4c: fa 8f cd  mov   ($cd),($8f)
2f4f: 00        nop
2f50: 4a ff 7d  and1  c,$0fbf,7
2f53: 7f        reti
2f54: f3 00 ff  bbc7  $00,$2f56
2f57: 18 e7 ff  or    $ff,#$e7
2f5a: ff        stop
2f5b: 5d        mov   x,a
2f5c: 2f fc     bra   $2f5a
2f5e: fe 0a     dbnz  y,$2f6a
2f60: fa ff ce  mov   ($ce),($ff)
2f63: 00        nop
2f64: e4 02     mov   a,$02
2f66: d4 01     mov   $01+x,a
2f68: 57 78     eor   a,($78)+y
2f6a: 7f        reti
2f6b: 7f        reti
2f6c: ff        stop
2f6d: 6f        ret

2f6e: 2f ea     bra   $2f5a
2f70: 82 fc     set4  $fc
2f72: d2 14     clr6  $14
2f74: 34 19     and   a,$19+x
2f76: 7d        mov   a,x
2f77: 7f        reti
2f78: ed        notc
2f79: 89 b4 d5  adc   ($d5),($b4)
2f7c: b4 c1     sbc   a,$c1+x
2f7e: b4 ad     sbc   a,$ad+x
2f80: ff        stop
2f81: 83 2f ea  bbs4  $2f,$2f6e
2f84: 78 fc c4  cmp   $c4,#$fc
2f87: 16 fb d5  or    a,$d5fb+y
2f8a: 4a 05 7f  and1  c,$0fe0,5
2f8d: 7f        reti
2f8e: f3 00 05  bbc7  $00,$2f96
2f91: 49 e7 ff  eor   ($ff),($e7)
2f94: e1        tcall 14
2f95: 35 7f ff  and   a,$ff7f+x
2f98: 9a 2f     subw  ya,$2f
2f9a: ea 8c fc  not1  $1f91,4
2f9d: e8 1a     mov   a,#$1a
2f9f: 3f 0f 7d  call  $7d0f
2fa2: 7f        reti
2fa3: fc        inc   y
2fa4: d8 0c     mov   $0c,x
2fa6: 54 0a     eor   a,$0a+x
2fa8: 7f        reti
2fa9: 7f        reti
2faa: f3 00 0a  bbc7  $00,$2fb7
2fad: 23 48 fd  bbs1  $48,$2fad
2fb0: 23 05 ff  bbs1  $05,$2fb2
2fb3: f3 00 05  bbc7  $00,$2fbb
2fb6: 41        tcall 4
2fb7: 55 03 41  eor   a,$4103+x
2fba: 19        or    (x),(y)
2fbb: 7d        mov   a,x
2fbc: 7f        reti
2fbd: f3 00 19  bbc7  $00,$2fd9
2fc0: 5b 93     lsr   $93+x
2fc2: 00        nop
2fc3: c1        tcall 12
2fc4: b2 f3     clr5  $f3
2fc6: 00        nop
2fc7: 19        or    (x),(y)
2fc8: 5b 93     lsr   $93+x
2fca: 00        nop
2fcb: ff        stop
2fcc: ce        pop   x
2fcd: 2f e2     bra   $2fb1
2fcf: 0a ea 8c  or1   c,$119d,2
2fd2: ee        pop   y
2fd3: dc        dec   y
2fd4: 5e 04 7d  cmp   y,$7d04
2fd7: 7f        reti
2fd8: 52 0a     clr2  $0a
2fda: ff        stop
2fdb: d2 d0     clr6  $d0
2fdd: ff        stop
2fde: e0        clrv
2fdf: 2f ea     bra   $2fcb
2fe1: 78 fc fe  cmp   $fe,#$fc
2fe4: 19        or    (x),(y)
2fe5: 3c        rol   a
2fe6: 23 7f 7f  bbs1  $7f,$3068
2fe9: f3 00 23  bbc7  $00,$300f
2fec: 2f d4     bra   $2fc2
2fee: ff        stop
2fef: fb d2     mov   y,$d2+x
2ff1: e1        tcall 14
2ff2: 5a 7f     cmpw  ya,$7f
2ff4: ff        stop
2ff5: f9 2f     mov   x,$2f+y
2ff7: fe 2f     dbnz  y,$3028
2ff9: ea ff e0  not1  $1c1f,7
2ffc: 01        tcall 0
2ffd: ff        stop
2ffe: ea ff e0  not1  $1c1f,7
3001: 01        tcall 0
3002: ff        stop
3003: 05 30 ea  or    a,$ea30
3006: 50 fc     bvc   $3004
3008: cc 18 3d  mov   $3d18,y
300b: 09 7d 7f  or    ($7f),($7d)
300e: bd        mov   sp,x
300f: c1        tcall 12
3010: bd        mov   sp,x
3011: 7f        reti
3012: 2d        push  a
3013: ff        stop
3014: 16 30 ea  or    a,$ea30+y
3017: 50 fc     bvc   $3015
3019: ce        pop   x
301a: 18 41 09  or    $09,#$41
301d: 7d        mov   a,x
301e: 7f        reti
301f: ed        notc
3020: 89 c1 c1  adc   ($c1),($c1)
3023: c1        tcall 12
3024: 7f        reti
3025: 2d        push  a
3026: ff        stop
3027: 29 30 ea  and   ($ea),($30)
302a: 50 fc     bvc   $3028
302c: ce        pop   x
302d: 18 34 0b  or    $0b,#$34
3030: 7d        mov   a,x
3031: 7f        reti
3032: b4 d5     sbc   a,$d5+x
3034: b4 c1     sbc   a,$c1+x
3036: b4 7f     sbc   a,$7f+x
3038: 2d        push  a
3039: ff        stop
303a: 3c        rol   a
303b: 30 ea     bmi   $3027
303d: 50 fc     bvc   $303b
303f: d4 18     mov   $18+x,a
3041: 3e 09     cmp   x,$09
3043: 7d        mov   a,x
3044: 7f        reti
3045: be        das   a
3046: c1        tcall 12
3047: be        das   a
3048: 7f        reti
3049: 28 ff     and   a,#$ff
304b: 4f 30     pcall $30
304d: 60        clrc
304e: 30 fc     bmi   $304c
3050: fe 2a     dbnz  y,$307c
3052: e3 12 2c  bbs7  $12,$3081
3055: 26        and   a,(x)
3056: 7f        reti
3057: 7f        reti
3058: fa 8f cf  mov   ($cf),($8f)
305b: 00        nop
305c: e1        tcall 14
305d: 2c 7f ff  rol   $ff7f
3060: fc        inc   y
3061: fe 2a     dbnz  y,$308d
3063: e3 16 30  bbs7  $16,$3096
3066: 26        and   a,(x)
3067: 7f        reti
3068: 7f        reti
3069: fa 8f cf  mov   ($cf),($8f)
306c: 00        nop
306d: e1        tcall 14
306e: 2c 7f ff  rol   $ff7f
3071: 73 30 ea  bbc3  $30,$305e
3074: 78 fc ae  cmp   $ae,#$fc
3077: 12 fa     clr0  $fa
3079: 8f e2 00  mov   $00,#$e2
307c: e5 ff 98  mov   a,$98ff
307f: 00        nop
3080: 59        eor   (x),(y)
3081: 18 7d 7f  or    $7f,#$7d
3084: ff        stop
3085: 87 30     adc   a,($30+x)
3087: ea 1e fc  not1  $1f83,6
308a: e6        mov   a,(x)
308b: 14 fa     or    a,$fa+x
308d: 8e        pop   psw
308e: d3 00 e4  bbc6  $00,$3075
3091: 00        nop
3092: 2d        push  a
3093: 86        adc   a,(x)
3094: 33 19 7f  bbc1  $19,$3116
3097: 7f        reti
3098: ff        stop
3099: 9b 30     dec   $30+x
309b: fc        inc   y
309c: c4 09     mov   $09,a
309e: e5 ff 22  mov   a,$22ff
30a1: 00        nop
30a2: 02 1e     set0  $1e
30a4: 7f        reti
30a5: 7f        reti
30a6: fa 8f d0  mov   ($d0),($8f)
30a9: 00        nop
30aa: e1        tcall 14
30ab: 50 7f     bvc   $312c
30ad: ff        stop
30ae: b0 30     bcs   $30e0
30b0: fc        inc   y
30b1: aa 09 e5  mov1  c,$1ca1,1
30b4: ff        stop
30b5: 05 00 1c  or    a,$1c00
30b8: 1e 7f 7f  cmp   x,$7f7f
30bb: fa 8f d0  mov   ($d0),($8f)
30be: 00        nop
30bf: e1        tcall 14
30c0: 50 7f     bvc   $3141
30c2: ff        stop
30c3: c5 30 ea  mov   $ea30,a
30c6: 2c fc fe  rol   $fefc
30c9: 1b fa     asl   $fa+x
30cb: 84 d2     adc   a,$d2
30cd: dc        dec   y
30ce: e4 dc     mov   a,$dc
30d0: 46        eor   a,(x)
30d1: 7f        reti
30d2: 3f 32 65  call  $6532
30d5: 3c        rol   a
30d6: f3 00 32  bbc7  $00,$310b
30d9: 39        and   (x),(y)
30da: fb ff     mov   y,$ff+x
30dc: e2 1a     set7  $1a
30de: fa 84 d3  mov   ($d3),($84)
30e1: 00        nop
30e2: 09 3c 7d  or    ($7d),($3c)
30e5: 6e ff e9  dbnz  $ff,$30d1
30e8: 30 ea     bmi   $30d4
30ea: 78 fc c8  cmp   $c8,#$fc
30ed: 1b fa     asl   $fa+x
30ef: 89 ce 00  adc   ($00),($ce)
30f2: 4d        push  x
30f3: 03 7f 7f  bbs0  $7f,$3175
30f6: f3 00 03  bbc7  $00,$30fc
30f9: 35 00 fc  and   a,$fc00+x
30fc: 35 19 7d  and   a,$7d19+x
30ff: 7f        reti
3100: f3 00 19  bbc7  $00,$311c
3103: 4d        push  x
3104: 73 00 ed  bbc3  $00,$30f4
3107: 86        adc   a,(x)
3108: 41        tcall 4
3109: 0f        brk
310a: c1        tcall 12
310b: f3 00 0f  bbc7  $00,$311d
310e: 4d        push  x
310f: 60        clrc
3110: 00        nop
3111: c1        tcall 12
3112: ad f3     cmp   y,#$f3
3114: 00        nop
3115: 0f        brk
3116: 4d        push  x
3117: 60        clrc
3118: 00        nop
3119: ff        stop
311a: 1c        asl   a
311b: 31        tcall 3
311c: ea 3c fc  not1  $1f87,4
311f: e2 1e     set7  $1e
3121: fa 8d e2  mov   ($e2),($8d)
3124: 00        nop
3125: 5b 03     lsr   $03+x
3127: 7d        mov   a,x
3128: 6e f3 00  dbnz  $f3,$312b
312b: 03 48 6b  bbs0  $48,$3199
312e: fe fa     dbnz  y,$312a
3130: 89 e2 00  adc   ($00),($e2)
3133: 48 0b     eor   a,#$0b
3135: ff        stop
3136: f3 00 0b  bbc7  $00,$3144
3139: 5b 69     lsr   $69+x
313b: 00        nop
313c: c8 e4     cmp   x,#$e4
313e: f3 00 0b  bbc7  $00,$314c
3141: 5b 69     lsr   $69+x
3143: 00        nop
3144: c8 c1     cmp   x,#$c1
3146: f3 00 0b  bbc7  $00,$3154
3149: 5b 69     lsr   $69+x
314b: 00        nop
314c: ff        stop
314d: 4f 31     pcall $31
314f: ea c8 fc  not1  $1f99,0
3152: d2 09     clr6  $09
3154: 3f 07 72  call  $7207
3157: 7f        reti
3158: f3 00 07  bbc7  $00,$3162
315b: 27 00     and   a,($00+x)
315d: fd        mov   y,a
315e: e6        mov   a,(x)
315f: 1b 27     asl   $27+x
3161: 72 7f     clr3  $7f
3163: f3 00 26  bbc7  $00,$318c
3166: 3f c0 00  call  $00c0
3169: e7 05     mov   a,($05+x)
316b: ec 00 ff  mov   y,$ff00
316e: 70 31     bvs   $31a1
3170: ea 50 fc  not1  $1f8a,0
3173: ca 15 ed  mov1  $1da2,5,c
3176: 8d e6     mov   y,#$e6
3178: 57 05     eor   a,($05)+y
317a: 7d        mov   a,x
317b: 7f        reti
317c: da ff     movw  $ff,ya
317e: e7 06     mov   a,($06+x)
3180: f1        tcall 15
3181: 00        nop
3182: ff        stop
3183: 85 31 ea  adc   a,$ea31
3186: 78 fc a8  cmp   $a8,#$fc
3189: 07 e4     or    a,($e4+x)
318b: 00        nop
318c: 3c        rol   a
318d: 14 e6     or    a,$e6+x
318f: f2 3c     clr7  $3c
3191: 55 0a 7d  eor   a,$7d0a+x
3194: 7f        reti
3195: 70 d6     bvs   $316d
3197: ff        stop
3198: e7 02     mov   a,($02+x)
319a: 00        nop
319b: 00        nop
319c: fb d0     mov   y,$d0+x
319e: f2 3c     clr7  $3c
31a0: 55 aa 7f  eor   a,$7faa+x
31a3: 7f        reti
31a4: ff        stop
31a5: a9 31 cc  sbc   ($cc),($31)
31a8: 31        tcall 3
31a9: ea 78 fc  not1  $1f8f,0
31ac: a8 07     sbc   a,#$07
31ae: fb d0     mov   y,$d0+x
31b0: e4 00     mov   a,$00
31b2: 3c        rol   a
31b3: 14 e3     or    a,$e3+x
31b5: 18 f2 3c  or    $3c,#$f2
31b8: e6        mov   a,(x)
31b9: 55 0a 7d  eor   a,$7d0a+x
31bc: 7f        reti
31bd: 55 82 ff  eor   a,$ff82+x
31c0: e7 04     mov   a,($04+x)
31c2: 00        nop
31c3: 04 55     or    a,$55
31c5: 0a ff 55  or1   c,$0abf,7
31c8: aa 7f 7f  mov1  c,$0fef,7
31cb: ff        stop
31cc: ea 78 fc  not1  $1f8f,0
31cf: a8 07     sbc   a,#$07
31d1: fb d0     mov   y,$d0+x
31d3: e4 00     mov   a,$00
31d5: 3c        rol   a
31d6: 16 e3 10  or    a,$10e3+y
31d9: e0        clrv
31da: 0e e6 56  tset1 $56e6
31dd: 0a 7d 7f  or1   c,$0fef,5
31e0: 56 82 ff  eor   a,$ff82+y
31e3: e7 04     mov   a,($04+x)
31e5: 00        nop
31e6: 04 56     or    a,$56
31e8: 0a ff 56  or1   c,$0adf,7
31eb: aa 7f 7f  mov1  c,$0fef,7
31ee: ff        stop
31ef: f1        tcall 15
31f0: 31        tcall 3
31f1: ea 64 fc  not1  $1f8c,4
31f4: f6 1c e3  mov   a,$e31c+y
31f7: 14 4b     or    a,$4b+x
31f9: 0a 7d 7f  or1   c,$0fef,5
31fc: d1        tcall 13
31fd: ff        stop
31fe: d1        tcall 13
31ff: c1        tcall 12
3200: d1        tcall 13
3201: ad ff     cmp   y,#$ff
3203: 05 32 fc  or    a,$fc32
3206: 9a 13     subw  ya,$13
3208: fa 8e e2  mov   ($e2),($8e)
320b: 00        nop
320c: e4 00     mov   a,$00
320e: ff        stop
320f: 3c        rol   a
3210: 54 0f     eor   a,$0f+x
3212: 7f        reti
3213: 7f        reti
3214: fa 8f d1  mov   ($d1),($8f)
3217: 00        nop
3218: 54 14     eor   a,$14+x
321a: b7 ff     sbc   a,($ff)+y
321c: 1e 32 ea  cmp   x,$ea32
321f: 6e fc c8  dbnz  $fc,$31ea
3222: 05 fa 8d  or    a,$8dfa
3225: e2 00     set7  $00
3227: 2b 0a     rol   $0a
3229: 7d        mov   a,x
322a: 7f        reti
322b: f3 00 0a  bbc7  $00,$3238
322e: 43 0b 01  bbs2  $0b,$3232
3231: ad ff     cmp   y,#$ff
3233: f3 00 0a  bbc7  $00,$3240
3236: 45 0b 01  eor   a,$010b
3239: af        mov   (x)+,a
323a: ff        stop
323b: f3 00 0a  bbc7  $00,$3248
323e: 47 0b     eor   a,($0b+x)
3240: 01        tcall 0
3241: b1        tcall 11
3242: ff        stop
3243: f3 00 0a  bbc7  $00,$3250
3246: 49 0b 01  eor   ($01),($0b)
3249: b3 ff f3  bbc5  $ff,$323f
324c: 00        nop
324d: 0a 4b 0b  or1   c,$0169,3
3250: 01        tcall 0
3251: b5 ff f3  sbc   a,$f3ff+x
3254: 00        nop
3255: 0a 4d 0b  or1   c,$0169,5
3258: 01        tcall 0
3259: b5 b2 f3  sbc   a,$f3b2+x
325c: 00        nop
325d: 0a 4d 0b  or1   c,$0169,5
3260: 01        tcall 0
3261: ff        stop
3262: 64 32     cmp   a,$32
3264: ea ff fc  not1  $1f9f,7
3267: ec 16 e5  mov   y,$e516
326a: ff        stop
326b: 14 ff     or    a,$ff+x
326d: 04 64     or    a,$64
326f: 7f        reti
3270: 7f        reti
3271: fb d8     mov   y,$d8+x
3273: e1        tcall 14
3274: 28 7f     and   a,#$7f
3276: ff        stop
3277: 79        cmp   (x),(y)
3278: 32 ea     clr1  $ea
327a: 50 fc     bvc   $3278
327c: ce        pop   x
327d: 18 42 09  or    $09,#$42
3280: 7d        mov   a,x
3281: 7f        reti
3282: ed        notc
3283: 89 c2 c1  adc   ($c1),($c2)
3286: c2 7f     set6  $7f
3288: 2d        push  a
3289: ff        stop
328a: 8c 32 ea  dec   $ea32
328d: b4 fc     sbc   a,$fc+x
328f: f6 0a ed  mov   a,$ed0a+y
3292: 8c 4d 05  dec   $054d
3295: 7f        reti
3296: 7f        reti
3297: f3 00 05  bbc7  $00,$329f
329a: 35 93 fc  and   a,$fc93+x
329d: 35 12 7d  and   a,$7d12+x
32a0: 64 f3     cmp   a,$f3
32a2: 00        nop
32a3: 12 4d     clr0  $4d
32a5: f5 00 b5  mov   a,$b500+x
32a8: d0 f3     bne   $329d
32aa: 00        nop
32ab: 12 4d     clr0  $4d
32ad: f5 00 b5  mov   a,$b500+x
32b0: bc        inc   a
32b1: f3 00 12  bbc7  $00,$32c6
32b4: 4d        push  x
32b5: f5 00 b5  mov   a,$b500+x
32b8: ad f3     cmp   y,#$f3
32ba: 00        nop
32bb: 12 4d     clr0  $4d
32bd: f5 00 ff  mov   a,$ff00+x
32c0: c2 32     set6  $32
32c2: ea dc fc  not1  $1f9b,4
32c5: ce        pop   x
32c6: 01        tcall 0
32c7: ed        notc
32c8: 8d 2b     mov   y,#$2b
32ca: 0a 7d 7f  or1   c,$0fef,5
32cd: f3 00 0a  bbc7  $00,$32da
32d0: 48 a2     eor   a,#$a2
32d2: 02 48     set0  $48
32d4: 1e ff f3  cmp   x,$f3ff
32d7: 00        nop
32d8: 1e 37 80  cmp   x,$8037
32db: ff        stop
32dc: c5 e4 f3  mov   $f3e4,a
32df: 00        nop
32e0: 1e 37 97  cmp   x,$9737
32e3: ff        stop
32e4: c5 c1 f3  mov   $f3c1,a
32e7: 00        nop
32e8: 1e 37 97  cmp   x,$9737
32eb: ff        stop
32ec: c5 ad f3  mov   $f3ad,a
32ef: 00        nop
32f0: 1e 37 97  cmp   x,$9737
32f3: ff        stop
32f4: ff        stop
32f5: f7 32     mov   a,($32)+y
32f7: ea 96 fc  not1  $1f92,6
32fa: d2 18     clr6  $18
32fc: 3d        inc   x
32fd: 08 7d     or    a,#$7d
32ff: 7f        reti
3300: fc        inc   y
3301: c4 1c     mov   $1c,a
3303: e6        mov   a,(x)
3304: 47 02     eor   a,($02+x)
3306: 7d        mov   a,x
3307: 7f        reti
3308: c2 ff     set6  $ff
330a: e7 0b     mov   a,($0b+x)
330c: fe 19     dbnz  y,$3327
330e: ef        sleep
330f: a0        ei
3310: e7 e6     mov   a,($e6+x)
3312: 4e 02 7d  tclr1 $7d02
3315: 2d        push  a
3316: ca ad e7  mov1  $1cf5,5,c
3319: 04 00     or    a,$00
331b: 00        nop
331c: ff        stop
331d: 1f 33 ea  jmp   ($ea33+x)

3320: 6e fc c0  dbnz  $fc,$32e3
3323: 08 ed     or    a,#$ed
3325: 8b 31     dec   $31
3327: 40        setp
3328: 7f        reti
3329: 7f        reti
332a: fb d5     mov   y,$d5+x
332c: e1        tcall 14
332d: 30 7f     bmi   $33ae
332f: ff        stop
3330: 32 33     clr1  $33
3332: ea c8 fc  not1  $1f99,0
3335: d4 07     mov   $07+x,a
3337: 48 0a     eor   a,#$0a
3339: 7d        mov   a,x
333a: 7f        reti
333b: f3 00 0a  bbc7  $00,$3348
333e: 10 56     bpl   $3396
3340: fb 3d     mov   y,$3d+x
3342: 19        or    (x),(y)
3343: ff        stop
3344: f3 00 19  bbc7  $00,$3360
3347: 0f        brk
3348: 90 fe     bcc   $3348
334a: bd        mov   sp,x
334b: e9 f3 00  mov   x,$00f3
334e: 19        or    (x),(y)
334f: 0f        brk
3350: 90 fe     bcc   $3350
3352: bd        mov   sp,x
3353: d5 f3 00  mov   $00f3+x,a
3356: 19        or    (x),(y)
3357: 0f        brk
3358: 90 fe     bcc   $3358
335a: bd        mov   sp,x
335b: c1        tcall 12
335c: f3 00 19  bbc7  $00,$3378
335f: 0f        brk
3360: 90 fe     bcc   $3360
3362: bd        mov   sp,x
3363: ad f3     cmp   y,#$f3
3365: 00        nop
3366: 19        or    (x),(y)
3367: 0f        brk
3368: 90 fe     bcc   $3368
336a: ff        stop
336b: 6d        push  y
336c: 33 ea a0  bbc1  $ea,$330f
336f: fc        inc   y
3370: f0 14     beq   $3386
3372: ed        notc
3373: 8d 0e     mov   y,#$0e
3375: 1e 72 7f  cmp   x,$7f72
3378: f3 00 1e  bbc7  $00,$3399
337b: 32 c0     clr1  $c0
337d: 00        nop
337e: 0e 18 e9  tset1 $e918
3381: f3 00 18  bbc7  $00,$339c
3384: 32 f2     clr1  $f2
3386: 00        nop
3387: 8e        pop   psw
3388: d5 f3 00  mov   $00f3+x,a
338b: 18 32 f2  or    $f2,#$32
338e: 00        nop
338f: fa 85 e2  mov   ($e2),($85)
3392: 00        nop
3393: 8e        pop   psw
3394: c1        tcall 12
3395: f3 00 18  bbc7  $00,$33b0
3398: 32 f2     clr1  $f2
339a: 00        nop
339b: 8e        pop   psw
339c: b7 f3     sbc   a,($f3)+y
339e: 00        nop
339f: 18 32 f2  or    $f2,#$32
33a2: 00        nop
33a3: ff        stop
33a4: a8 33     sbc   a,#$33
33a6: cd 33     mov   x,#$33
33a8: ea 9b fc  not1  $1f93,3
33ab: e0        clrv
33ac: 00        nop
33ad: f2 0e     clr7  $0e
33af: 36 0c 7f  and   a,$7f0c+y
33b2: 78 36 06  cmp   $06,#$36
33b5: 7d        mov   a,x
33b6: 6e bd 7f  dbnz  $bd,$3438
33b9: 78 bd 7d  cmp   $7d,#$bd
33bc: 6e ba 7f  dbnz  $ba,$343e
33bf: 78 ba 7d  cmp   $7d,#$ba
33c2: 6e e6 42  dbnz  $e6,$3407
33c5: 06        or    a,(x)
33c6: 7f        reti
33c7: 78 e7 07  cmp   $07,#$e7
33ca: f7 00     mov   a,($00)+y
33cc: ff        stop
33cd: ea 9b fc  not1  $1f93,3
33d0: f0 00     beq   $33d2
33d2: 36 06 7f  and   a,$7f06+y
33d5: 78 b6 7d  cmp   $7d,#$b6
33d8: 6e bd 7f  dbnz  $bd,$345a
33db: 78 bd 7d  cmp   $7d,#$bd
33de: 6e ba 7f  dbnz  $ba,$3460
33e1: 78 ba 7d  cmp   $7d,#$ba
33e4: 6e e6 42  dbnz  $e6,$3429
33e7: 06        or    a,(x)
33e8: 7f        reti
33e9: 78 e7 07  cmp   $07,#$e7
33ec: f7 00     mov   a,($00)+y
33ee: ff        stop
33ef: f1        tcall 15
33f0: 33 ea 6e  bbc1  $ea,$3461
33f3: fc        inc   y
33f4: e2 16     set7  $16
33f6: e6        mov   a,(x)
33f7: 45 0a 7d  eor   a,$7d0a
33fa: 7f        reti
33fb: c1        tcall 12
33fc: ff        stop
33fd: e7 06     mov   a,($06+x)
33ff: ef        sleep
3400: 00        nop
3401: ff        stop
3402: 04 34     or    a,$34
3404: ea 78 fc  not1  $1f8f,0
3407: d2 02     clr6  $02
3409: fa 8c e2  mov   ($e2),($8c)
340c: e5 e4 00  mov   a,$00e4
340f: ff        stop
3410: 81        tcall 8
3411: f2 3c     clr7  $3c
3413: 4b 0c     lsr   $0c
3415: 3f 7f f2  call  $f27f
3418: 4b 4b     lsr   $4b
341a: 3c        rol   a
341b: 7f        reti
341c: 7f        reti
341d: fb d9     mov   y,$d9+x
341f: e1        tcall 14
3420: 14 7d     or    a,$7d+x
3422: fb d6     mov   y,$d6+x
3424: 4b 41     lsr   $41
3426: 7f        reti
3427: 2d        push  a
3428: ff        stop
3429: 2b 34     rol   $34
342b: ea 82 fc  not1  $1f90,2
342e: ea 1d fa  not1  $1f43,5
3431: 8f d8 00  mov   $00,#$d8
3434: 55 09 7d  eor   a,$7d09+x
3437: 7f        reti
3438: 5c        lsr   a
3439: 10 ff     bpl   $343a
343b: f3 00 10  bbc7  $00,$344e
343e: 50 9d     bvc   $33dd
3440: ff        stop
3441: fa 89 d8  mov   ($d8),($89)
3444: 00        nop
3445: dc        dec   y
3446: cb f3     mov   $f3,y
3448: 00        nop
3449: 10 50     bpl   $349b
344b: 9d        mov   x,sp
344c: ff        stop
344d: dc        dec   y
344e: b2 f3     clr5  $f3
3450: 00        nop
3451: 10 50     bpl   $34a3
3453: 9d        mov   x,sp
3454: ff        stop
3455: ff        stop
3456: 58 34 ea  eor   $ea,#$34
3459: 8c fc e2  dec   $e2fc
345c: 1a 41     decw  $41
345e: 0a 7d 7f  or1   c,$0fef,5
3461: b9        sbc   (x),(y)
3462: ff        stop
3463: b9        sbc   (x),(y)
3464: c1        tcall 12
3465: b9        sbc   (x),(y)
3466: ad ff     cmp   y,#$ff
3468: 6c 34 be  ror   $be34
346b: 34 e2     and   a,$e2+x
346d: 1e ea 96  cmp   x,$96ea
3470: ee        pop   y
3471: fe fa     dbnz  y,$346d
3473: 88 e2     adc   a,#$e2
3475: 00        nop
3476: 56 14 7d  eor   a,$7d14+y
3479: 7f        reti
347a: f3 00 14  bbc7  $00,$3491
347d: 41        tcall 4
347e: 62 ff     set3  $ff
3480: df        daa   a
3481: ff        stop
3482: f3 00 14  bbc7  $00,$3499
3485: 41        tcall 4
3486: 1f ff df  jmp   ($dfff+x)

3489: da f3     movw  $f3,ya
348b: 00        nop
348c: 14 41     or    a,$41+x
348e: 1f ff 5f  jmp   ($5fff+x)

3491: 0a cb f3  or1   c,$1e79,3
3494: 00        nop
3495: 0a 41 3d  or1   c,$07a8,1
3498: fe ea     dbnz  y,$3484
349a: 78 fc 7c  cmp   $7c,#$fc
349d: 13 fa 85  bbc0  $fa,$3425
34a0: d3 00 e3  bbc6  $00,$3486
34a3: 12 58     clr0  $58
34a5: 3f f8 f3  call  $f3f8
34a8: 00        nop
34a9: 8c 40 ec  dec   $ec40
34ac: ff        stop
34ad: d8 d5     mov   $d5,x
34af: f3 00 8c  bbc7  $00,$343e
34b2: 40        setp
34b3: ec ff d8  mov   y,$d8ff
34b6: c1        tcall 12
34b7: f3 00 8c  bbc7  $00,$3446
34ba: 40        setp
34bb: ec ff ff  mov   y,$ffff
34be: e2 1c     set7  $1c
34c0: ea 96 ee  not1  $1dd2,6
34c3: fe 45     dbnz  y,$350a
34c5: 14 7d     or    a,$7d+x
34c7: 7f        reti
34c8: ce        pop   x
34c9: ff        stop
34ca: ce        pop   x
34cb: c1        tcall 12
34cc: 4e 0a ad  tclr1 $ad0a
34cf: ea 78 fc  not1  $1f8f,0
34d2: be        das   a
34d3: 05 fa 8a  or    a,$8afa
34d6: d4 00     mov   $00+x,a
34d8: e3 16 e6  bbs7  $16,$34c1
34db: 51        tcall 5
34dc: 0c 7f 7f  asl   $7f7f
34df: f3 04 08  bbc7  $04,$34ea
34e2: 4d        push  x
34e3: c4 ff     mov   $ff,a
34e5: 4d        push  x
34e6: 02 7d     set0  $7d
34e8: 7f        reti
34e9: f3 00 02  bbc7  $00,$34ee
34ec: 52 40     clr2  $40
34ee: 01        tcall 0
34ef: 51        tcall 5
34f0: 07 c1     or    a,($c1+x)
34f2: f3 04 08  bbc7  $04,$34fd
34f5: 4d        push  x
34f6: c4 ff     mov   $ff,a
34f8: e7 03     mov   a,($03+x)
34fa: 00        nop
34fb: 00        nop
34fc: e6        mov   a,(x)
34fd: 52 0c     clr2  $0c
34ff: 7f        reti
3500: 78 f3 00  cmp   $00,#$f3
3503: 0c 48 9a  asl   $9a48
3506: ff        stop
3507: 48 02     eor   a,#$02
3509: 72 78     clr3  $78
350b: f3 00 02  bbc7  $00,$3510
350e: 4d        push  x
350f: 40        setp
3510: 01        tcall 0
3511: 52 07     clr2  $07
3513: ad f3     cmp   y,#$f3
3515: 00        nop
3516: 0c 48 9a  asl   $9a48
3519: ff        stop
351a: e7 07     mov   a,($07+x)
351c: f4 00     mov   a,$00+x
351e: ff        stop
351f: 23 35 46  bbs1  $35,$3568
3522: 35 ea ff  and   a,$ffea+x
3525: fc        inc   y
3526: f2 02     clr7  $02
3528: 42 0a     set2  $0a
352a: 7f        reti
352b: 7f        reti
352c: f3 00 0a  bbc7  $00,$3539
352f: 11        tcall 1
3530: 1a fb     decw  $fb
3532: fa 8f ce  mov   ($ce),($8f)
3535: 00        nop
3536: e6        mov   a,(x)
3537: 33 32 7f  bbc1  $32,$35b9
353a: 7f        reti
353b: f3 00 32  bbc7  $00,$3570
353e: 11        tcall 1
353f: 52 ff     clr2  $ff
3541: e7 0a     mov   a,($0a+x)
3543: fd        mov   y,a
3544: 00        nop
3545: ff        stop
3546: ea ff fc  not1  $1f9f,7
3549: f2 09     clr7  $09
354b: 29 0a 7d  and   ($7d),($0a)
354e: 7f        reti
354f: f3 00 0a  bbc7  $00,$355c
3552: 13 cd fd  bbc0  $cd,$3552
3555: fa 8f ce  mov   ($ce),($8f)
3558: 00        nop
3559: 29 ff 7f  and   ($7f),($ff)
355c: 7f        reti
355d: f3 00 ff  bbc7  $00,$355f
3560: 13 ea ff  bbc0  $ea,$3562
3563: e1        tcall 14
3564: f5 7f ff  mov   a,$ff7f+x
3567: 69 35 ea  cmp   ($ea),($35)
356a: 60        clrc
356b: fc        inc   y
356c: e0        clrv
356d: 21        tcall 2
356e: fa 8f ef  mov   ($ef),($8f)
3571: 00        nop
3572: 21        tcall 2
3573: 02 7f     set0  $7f
3575: 7f        reti
3576: 99        adc   (x),(y)
3577: ff        stop
3578: 95 ff 91  adc   a,$91ff+x
357b: ff        stop
357c: e6        mov   a,(x)
357d: 0d        push  psw
357e: 01        tcall 0
357f: 7f        reti
3580: 7f        reti
3581: 8e        pop   psw
3582: ff        stop
3583: 8f ff 90  mov   $90,#$ff
3586: ff        stop
3587: 91        tcall 9
3588: ff        stop
3589: 92 ff     clr4  $ff
358b: 93 ff 94  bbc4  $ff,$3522
358e: ff        stop
358f: 95 ff 94  adc   a,$94ff+x
3592: ff        stop
3593: 93 ff 92  bbc4  $ff,$3528
3596: ff        stop
3597: 91        tcall 9
3598: ff        stop
3599: 90 ff     bcc   $359a
359b: 8f ff 8e  mov   $8e,#$ff
359e: ff        stop
359f: e7 0a     mov   a,($0a+x)
35a1: 00        nop
35a2: 08 ff     or    a,#$ff
35a4: a6        sbc   a,(x)
35a5: 35 ea 8c  and   a,$8cea+x
35a8: fc        inc   y
35a9: a0        ei
35aa: 1a 3c     decw  $3c
35ac: 06        or    a,(x)
35ad: 7d        mov   a,x
35ae: 7f        reti
35af: bc        inc   a
35b0: cb ff     mov   $ff,y
35b2: b4 35     sbc   a,$35+x
35b4: ea 94 fc  not1  $1f92,4
35b7: d4 09     mov   $09+x,a
35b9: ed        notc
35ba: 89 3b 78  adc   ($78),($3b)
35bd: 7d        mov   a,x
35be: 7f        reti
35bf: f3 00 5f  bbc7  $00,$3621
35c2: 25 de ff  and   a,$ffde
35c5: fa 8d d1  mov   ($d1),($8d)
35c8: 00        nop
35c9: 30 3c     bmi   $3607
35cb: 7f        reti
35cc: 50 f3     bvc   $35c1
35ce: 00        nop
35cf: 24 25     and   a,$25
35d1: d3 ff ff  bbc6  $ff,$35d3
35d4: d6 35 ea  mov   $ea35+y,a
35d7: c8 fc     cmp   x,#$fc
35d9: bc        inc   a
35da: 07 4f     or    a,($4f+x)
35dc: 0a 7d 7f  or1   c,$0fef,5
35df: f3 00 0a  bbc7  $00,$35ec
35e2: 23 56 fc  bbs1  $56,$35e1
35e5: 4f 14     pcall $14
35e7: ee        pop   y
35e8: f3 00 14  bbc7  $00,$35ff
35eb: 23 3e fe  bbs1  $3e,$35ec
35ee: cf        mul   ya
35ef: d5 f3 00  mov   $00f3+x,a
35f2: 14 23     or    a,$23+x
35f4: 3e fe     cmp   x,$fe
35f6: cf        mul   ya
35f7: c1        tcall 12
35f8: f3 00 14  bbc7  $00,$360f
35fb: 23 3e fe  bbs1  $3e,$35fc
35fe: cf        mul   ya
35ff: ad f3     cmp   y,#$f3
3601: 00        nop
3602: 14 23     or    a,$23+x
3604: 3e fe     cmp   x,$fe
3606: ff        stop
3607: 0b 36     asl   $36
3609: 1c        asl   a
360a: 36 ea 64  and   a,$64ea+y
360d: fc        inc   y
360e: c0        di
360f: 1f fa 8f  jmp   ($8ffa+x)

3612: d6 00 e3  mov   $e300+y,a
3615: 10 f2     bpl   $3609
3617: 08 fe     or    a,#$fe
3619: 2b 36     rol   $36
361b: ff        stop
361c: ea 64 fc  not1  $1f8c,4
361f: be        das   a
3620: 1f fa 8f  jmp   ($8ffa+x)

3623: d6 00 e3  mov   $e300+y,a
3626: 18 fe 2b  or    $2b,#$fe
3629: 36 ff 37  and   a,$37ff+y
362c: 02 7f     set0  $7f
362e: 7f        reti
362f: f3 00 02  bbc7  $00,$3634
3632: 43 66 02  bbs2  $66,$3637
3635: 43 0c 7d  bbs2  $0c,$36b5
3638: 78 43 1e  cmp   $1e,#$43
363b: 7f        reti
363c: 2d        push  a
363d: ff        stop
363e: 40        setp
363f: 36 ea ff  and   a,$ffea+y
3642: fc        inc   y
3643: 9a 14     subw  ya,$14
3645: ef        sleep
3646: f8 4b     mov   x,$4b
3648: f2 42     clr7  $42
364a: 0f        brk
364b: 1e 72 7f  cmp   x,$7f72
364e: f3 00 0f  bbc7  $00,$3660
3651: 32 55     clr1  $55
3653: 02 ed     set0  $ed
3655: 89 8f 7d  adc   ($7d),($8f)
3658: 55 f3 00  eor   a,$00f3+x
365b: 0f        brk
365c: 32 55     clr1  $55
365e: 02 8f     set0  $8f
3660: c1        tcall 12
3661: f3 00 0f  bbc7  $00,$3673
3664: 32 55     clr1  $55
3666: 02 8f     set0  $8f
3668: ad f3     cmp   y,#$f3
366a: 00        nop
366b: 0f        brk
366c: 32 55     clr1  $55
366e: 02 ff     set0  $ff
3670: 72 36     clr3  $36
3672: ea 78 fc  not1  $1f8f,0
3675: d2 14     clr6  $14
3677: fa 8c cd  mov   ($cd),($8c)
367a: 00        nop
367b: 51        tcall 5
367c: 04 7f     or    a,$7f
367e: 7f        reti
367f: f3 00 04  bbc7  $00,$3686
3682: 1f c0 f9  jmp   ($f9c0+x)

3685: 9f        xcn   a
3686: ff        stop
3687: f3 00 04  bbc7  $00,$368e
368a: 3d        inc   x
368b: c0        di
368c: 03 3f 1e  bbs0  $3f,$36ad
368f: 7d        mov   a,x
3690: 7f        reti
3691: f3 00 1e  bbc7  $00,$36b2
3694: 1b 70     asl   $70+x
3696: ff        stop
3697: bf        mov   a,(x)+
3698: b5 f3 00  sbc   a,$00f3+x
369b: 1e 1b 70  cmp   x,$701b
369e: ff        stop
369f: ff        stop
36a0: a4 36     sbc   a,$36
36a2: b3 36 ea  bbc5  $36,$368f
36a5: 5a fc     cmpw  ya,$fc
36a7: 96 07 75  adc   a,$7507+y
36aa: e3 12 42  bbs7  $12,$36ef
36ad: 07 7d     or    a,($7d+x)
36af: 7f        reti
36b0: c7 ff     mov   ($ff+x),a
36b2: ff        stop
36b3: ea 5a fc  not1  $1f8b,2
36b6: b4 07     sbc   a,$07+x
36b8: e3 16 42  bbs7  $16,$36fd
36bb: 05 7d 7f  or    a,$7f7d
36be: 47 07     eor   a,($07+x)
36c0: ff        stop
36c1: ed        notc
36c2: 86        adc   a,(x)
36c3: 42 05     set2  $05
36c5: b2 47     clr5  $47
36c7: 07 b2     or    a,($b2+x)
36c9: ff        stop
36ca: cc 36 ea  mov   $ea36,y
36cd: 82 fc     set4  $fc
36cf: d4 1b     mov   $1b+x,a
36d1: ed        notc
36d2: 8c 49 06  dec   $0649
36d5: 72 7f     clr3  $7f
36d7: f3 00 06  bbc7  $00,$36e0
36da: 31        tcall 3
36db: d2 fd     clr6  $fd
36dd: 31        tcall 3
36de: 0a 7d 5a  or1   c,$0b4f,5
36e1: f3 00 0a  bbc7  $00,$36ee
36e4: 49 43 01  eor   ($01),($43)
36e7: ff        stop
36e8: ea 36 ea  not1  $1d46,6
36eb: 5a fc     cmpw  ya,$fc
36ed: fe 24     dbnz  y,$3713
36ef: fa 8e d6  mov   ($d6),($8e)
36f2: 00        nop
36f3: 52 05     clr2  $05
36f5: 7d        mov   a,x
36f6: 7f        reti
36f7: 54 0c     eor   a,$0c+x
36f9: e6        mov   a,(x)
36fa: ff        stop
36fb: fd        mov   y,a
36fc: 36 ea 82  and   a,$82ea+y
36ff: fc        inc   y
3700: dc        dec   y
3701: 1b ed     asl   $ed+x
3703: 8c 45 06  dec   $0645
3706: 72 7f     clr3  $7f
3708: f3 00 06  bbc7  $00,$3711
370b: 2d        push  a
370c: d2 fd     clr6  $fd
370e: 2d        push  a
370f: 0a 7d 5a  or1   c,$0b4f,5
3712: f3 00 0a  bbc7  $00,$371f
3715: 45 43 01  eor   a,$0143
3718: ff        stop
3719: 1b 37     asl   $37+x
371b: ea 6e fc  not1  $1f8d,6
371e: f6 01 e6  mov   a,$e601+y
3721: 3f 04 7d  call  $7d04
3724: 7f        reti
3725: c3 ff c6  bbs6  $ff,$36ee
3728: ff        stop
3729: ca ff e7  mov1  $1cff,7,c
372c: 06        or    a,(x)
372d: f2 00     clr7  $00
372f: ff        stop
3730: 32 37     clr1  $37
3732: ea 78 fc  not1  $1f8f,0
3735: f0 01     beq   $3738
3737: 21        tcall 2
3738: 06        or    a,(x)
3739: 7d        mov   a,x
373a: 7f        reti
373b: f3 00 06  bbc7  $00,$3744
373e: 26        and   a,(x)
373f: 6a 00 a7  and1  c,!($14e0,0)
3742: ff        stop
3743: f3 00 06  bbc7  $00,$374c
3746: 2c 6a 00  rol   $006a
3749: ad ff     cmp   y,#$ff
374b: f3 00 06  bbc7  $00,$3754
374e: 32 6a     clr1  $6a
3750: 00        nop
3751: b3 ff f3  bbc5  $ff,$3747
3754: 00        nop
3755: 06        or    a,(x)
3756: 38 6a 00  and   $00,#$6a
3759: e6        mov   a,(x)
375a: 39        and   (x),(y)
375b: 06        or    a,(x)
375c: 7d        mov   a,x
375d: 7f        reti
375e: f3 00 06  bbc7  $00,$3767
3761: 3e 6a     cmp   x,$6a
3763: 00        nop
3764: bf        mov   a,(x)+
3765: ff        stop
3766: f3 00 06  bbc7  $00,$376f
3769: 44 6a     eor   a,$6a
376b: 00        nop
376c: e7 07     mov   a,($07+x)
376e: f4 00     mov   a,$00+x
3770: ff        stop
3771: 73 37 ea  bbc3  $37,$375e
3774: 8c fc cc  dec   $ccfc
3777: 02 e6     set0  $e6
3779: 3c        rol   a
377a: 0c 72 7f  asl   $7f72
377d: c8 ff     cmp   x,#$ff
377f: e7 06     mov   a,($06+x)
3781: f3 00 ff  bbc7  $00,$3783
3784: 86        adc   a,(x)
3785: 37 ea     and   a,($ea)+y
3787: 80        setc
3788: fc        inc   y
3789: a4 13     sbc   a,$13
378b: 44 0b     eor   a,$0b
378d: 7f        reti
378e: 7f        reti
378f: f3 00 06  bbc7  $00,$3798
3792: 20        clrp
3793: 00        nop
3794: fd        mov   y,a
3795: e6        mov   a,(x)
3796: 3b 02     rol   $02+x
3798: 7f        reti
3799: 7f        reti
379a: b6 ff af  sbc   a,$afff+y
379d: ff        stop
379e: 2a 03 7d  or1   c,!($0fa0,3)
37a1: 7f        reti
37a2: e7 05     mov   a,($05+x)
37a4: ef        sleep
37a5: 00        nop
37a6: ff        stop
37a7: a9 37 ea  sbc   ($ea),($37)
37aa: ff        stop
37ab: fc        inc   y
37ac: e0        clrv
37ad: 0c fa ff  asl   $fffa
37b0: e2 00     set7  $00
37b2: 1d        dec   x
37b3: 0a 7f 7f  or1   c,$0fef,7
37b6: f3 00 0a  bbc7  $00,$37c3
37b9: 0d        push  psw
37ba: 67 fe     cmp   a,($fe+x)
37bc: e6        mov   a,(x)
37bd: 14 14     or    a,$14+x
37bf: 7f        reti
37c0: 7f        reti
37c1: f3 00 0f  bbc7  $00,$37d3
37c4: 26        and   a,(x)
37c5: 33 01 e7  bbc1  $01,$37af
37c8: 05 eb 00  or    a,$00eb
37cb: ff        stop
37cc: d0 37     bne   $3805
37ce: f1        tcall 15
37cf: 37 ea     and   a,($ea)+y
37d1: 3c        rol   a
37d2: fc        inc   y
37d3: d2 14     clr6  $14
37d5: fa 88 e2  mov   ($e2),($88)
37d8: 00        nop
37d9: e4 00     mov   a,$00
37db: 1f 82 e3  jmp   ($e382+x)

37de: 10 33     bpl   $3813
37e0: 5a 7f     cmpw  ya,$7f
37e2: 7f        reti
37e3: f3 00 5a  bbc7  $00,$3840
37e6: 27 f8     and   a,($f8+x)
37e8: ff        stop
37e9: fa 8f ce  mov   ($ce),($8f)
37ec: 00        nop
37ed: e1        tcall 14
37ee: 82 7f     set4  $7f
37f0: ff        stop
37f1: ea 3c fc  not1  $1f87,4
37f4: d2 14     clr6  $14
37f6: fa 88 e2  mov   ($e2),($88)
37f9: 00        nop
37fa: e4 00     mov   a,$00
37fc: 1d        dec   x
37fd: 82 e3     set4  $e3
37ff: 18 30 5a  or    $5a,#$30
3802: 7f        reti
3803: 7f        reti
3804: f3 00 5a  bbc7  $00,$3861
3807: 24 f8     and   a,$f8
3809: ff        stop
380a: fa 8f ce  mov   ($ce),($8f)
380d: 00        nop
380e: e1        tcall 14
380f: 82 7f     set4  $7f
3811: ff        stop
3812: 14 38     or    a,$38+x
3814: ea 8c fc  not1  $1f91,4
3817: ac 07 fa  inc   $fa07
381a: 89 ca 00  adc   ($00),($ca)
381d: 34 14     and   a,$14+x
381f: 7f        reti
3820: 7f        reti
3821: f3 00 14  bbc7  $00,$3838
3824: 54 e3     eor   a,$e3+x
3826: 00        nop
3827: 54 7f     eor   a,$7f+x
3829: 7d        mov   a,x
382a: 7f        reti
382b: f3 00 7f  bbc7  $00,$38ad
382e: 40        setp
382f: ea ff 4c  not1  $099f,7
3832: 40        setp
3833: 7f        reti
3834: 2d        push  a
3835: f3 00 40  bbc7  $00,$3878
3838: 40        setp
3839: e6        mov   a,(x)
383a: ff        stop
383b: ff        stop
383c: 3e 38     cmp   x,$38
383e: ea 78 fc  not1  $1f8f,0
3841: fe 1a     dbnz  y,$385d
3843: 03 8c 7f  bbs0  $8c,$38c5
3846: 7f        reti
3847: fb d5     mov   y,$d5+x
3849: 03 3c ff  bbs0  $3c,$384b
384c: ff        stop
384d: 4f 38     pcall $38
384f: ea 82 fc  not1  $1f90,2
3852: f4 01     mov   a,$01+x
3854: 03 28 7d  bbs0  $28,$38d4
3857: 7f        reti
3858: 03 1e d5  bbs0  $1e,$3830
385b: 83 c1 83  bbs4  $c1,$37e1
385e: ad ff     cmp   y,#$ff
3860: 62 38     set3  $38
3862: fc        inc   y
3863: fe 09     dbnz  y,$386e
3865: e3 1e f8  bbs7  $1e,$3860
3868: 10 fe     bpl   $3868
386a: fa 87 cb  mov   ($cb),($87)
386d: 00        nop
386e: e5 ff 05  mov   a,$05ff
3871: 00        nop
3872: 1c        asl   a
3873: ff        stop
3874: 7d        mov   a,x
3875: 7f        reti
3876: ff        stop
3877: 79        cmp   (x),(y)
3878: 38 ea ff  and   $ff,#$ea
387b: fc        inc   y
387c: cc 19 fa  mov   $fa19,y
387f: 8a e2 00  eor1  c,$001c,2
3882: e5 ff b1  mov   a,$b1ff
3885: 00        nop
3886: 54 0c     eor   a,$0c+x
3888: 7f        reti
3889: 7f        reti
388a: fa 8f d3  mov   ($d3),($8f)
388d: 00        nop
388e: e1        tcall 14
388f: 46        eor   a,(x)
3890: 7f        reti
3891: ff        stop
