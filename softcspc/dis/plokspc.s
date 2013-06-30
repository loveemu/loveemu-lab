0400: 20        clrp
0401: 00        nop
0402: cd ff     mov   x,#$ff
0404: bd        mov   sp,x
0405: 8f 30 f1  mov   $f1,#$30
0408: 8f 6c f2  mov   $f2,#$6c
040b: 8f f3 f3  mov   $f3,#$f3
040e: 8f 5c f2  mov   $f2,#$5c
0411: 8f ff f3  mov   $f3,#$ff
0414: 8f 0c f2  mov   $f2,#$0c
0417: 8f 00 f3  mov   $f3,#$00
041a: 8f 1c f2  mov   $f2,#$1c
041d: 8f 00 f3  mov   $f3,#$00
0420: 3f 62 10  call  $1062
0423: 3f 08 07  call  $0708
0426: 8f 1b eb  mov   $eb,#$1b
0429: 8f 09 ec  mov   $ec,#$09
042c: 8f 13 ed  mov   $ed,#$13
042f: 8f 3a ee  mov   $ee,#$3a
0432: 8f 80 e8  mov   $e8,#$80
0435: 8f 80 e9  mov   $e9,#$80
0438: 8f fb d1  mov   $d1,#$fb
043b: e8 fa     mov   a,#$fa
043d: c4 f4     mov   $f4,a
043f: c4 f5     mov   $f5,a
0441: 64 f4     cmp   a,$f4
0443: d0 fc     bne   $0441
0445: 64 f4     cmp   a,$f4
0447: d0 f8     bne   $0441
0449: 3f e8 10  call  $10e8
044c: 8f 6c f2  mov   $f2,#$6c
044f: 8f 33 f3  mov   $f3,#$33
0452: cd 00     mov   x,#$00
0454: 3f 89 05  call  $0589
0457: 3f c5 04  call  $04c5
045a: 9d        mov   x,sp
045b: 3d        inc   x
045c: d0 2f     bne   $048d
045e: e4 f5     mov   a,$f5
0460: 64 f5     cmp   a,$f5
0462: d0 fa     bne   $045e
0464: 68 55     cmp   a,#$55
0466: f0 2c     beq   $0494
0468: 3f 63 11  call  $1163
046b: e5 44 03  mov   a,$0344
046e: 60        clrc
046f: 84 ff     adc   a,$ff
0471: c5 44 03  mov   $0344,a
0474: 68 04     cmp   a,#$04
0476: 90 e2     bcc   $045a
0478: 80        setc
0479: a8 04     sbc   a,#$04
047b: c5 44 03  mov   $0344,a
047e: 8f 0c f7  mov   $f7,#$0c
0481: 3f c5 04  call  $04c5
0484: 8f 08 f7  mov   $f7,#$08
0487: 8f 0f f7  mov   $f7,#$0f
048a: 5f 5a 04  jmp   $045a

048d: 7d        mov   a,x
048e: 9c        dec   a
048f: cd 80     mov   x,#$80
0491: 5f ac 12  jmp   $12ac

0494: e4 ff     mov   a,$ff
0496: e4 e1     mov   a,$e1
0498: eb ff     mov   y,$ff
049a: f0 fc     beq   $0498
049c: 80        setc
049d: a8 08     sbc   a,#$08
049f: 10 02     bpl   $04a3
04a1: e8 00     mov   a,#$00
04a3: 8f 0c f2  mov   $f2,#$0c
04a6: c4 f3     mov   $f3,a
04a8: 8f 1c f2  mov   $f2,#$1c
04ab: c4 f3     mov   $f3,a
04ad: 08 00     or    a,#$00
04af: d0 e7     bne   $0498
04b1: e8 ff     mov   a,#$ff
04b3: 8f 5c f2  mov   $f2,#$5c
04b6: c4 f3     mov   $f3,a
04b8: e8 b0     mov   a,#$b0
04ba: c4 f1     mov   $f1,a
04bc: 8f 6c f2  mov   $f2,#$6c
04bf: 8f e0 f3  mov   $f3,#$e0
04c2: 5f b6 12  jmp   $12b6

04c5: ab e6     inc   $e6
04c7: ba e2     movw  ya,$e2
04c9: f0 24     beq   $04ef
04cb: 30 12     bmi   $04df
04cd: ba e0     movw  ya,$e0
04cf: 7a e2     addw  ya,$e2
04d1: 10 08     bpl   $04db
04d3: 8f 00 e2  mov   $e2,#$00
04d6: 8f 00 e3  mov   $e3,#$00
04d9: 8d 7f     mov   y,#$7f
04db: da e0     movw  $e0,ya
04dd: 2f 10     bra   $04ef
04df: ba e0     movw  ya,$e0
04e1: 7a e2     addw  ya,$e2
04e3: 10 08     bpl   $04ed
04e5: 8f 00 e2  mov   $e2,#$00
04e8: 8f 00 e3  mov   $e3,#$00
04eb: 8d 00     mov   y,#$00
04ed: da e0     movw  $e0,ya
04ef: 8f 5c f2  mov   $f2,#$5c
04f2: 8f 00 f3  mov   $f3,#$00
04f5: 8d 00     mov   y,#$00
04f7: cd 00     mov   x,#$00
04f9: 3f f7 0f  call  $0ff7
04fc: 8d 02     mov   y,#$02
04fe: cd 10     mov   x,#$10
0500: 3f f7 0f  call  $0ff7
0503: 8d 04     mov   y,#$04
0505: cd 20     mov   x,#$20
0507: 3f f7 0f  call  $0ff7
050a: 8d 06     mov   y,#$06
050c: cd 30     mov   x,#$30
050e: 3f f7 0f  call  $0ff7
0511: 8d 08     mov   y,#$08
0513: cd 40     mov   x,#$40
0515: 3f f7 0f  call  $0ff7
0518: 8d 0a     mov   y,#$0a
051a: cd 50     mov   x,#$50
051c: 3f f7 0f  call  $0ff7
051f: 8d 0c     mov   y,#$0c
0521: cd 60     mov   x,#$60
0523: 3f f7 0f  call  $0ff7
0526: 8d 0e     mov   y,#$0e
0528: cd 70     mov   x,#$70
052a: 3f f7 0f  call  $0ff7
052d: 8f 4c f2  mov   $f2,#$4c
0530: fa d4 f3  mov   ($f3),($d4)
0533: 8f 00 d4  mov   $d4,#$00
0536: cd 00     mov   x,#$00
0538: 3f 53 07  call  $0753
053b: b0 2f     bcs   $056c
053d: cd 02     mov   x,#$02
053f: 3f 53 07  call  $0753
0542: b0 28     bcs   $056c
0544: cd 04     mov   x,#$04
0546: 3f 53 07  call  $0753
0549: b0 21     bcs   $056c
054b: cd 06     mov   x,#$06
054d: 3f 53 07  call  $0753
0550: b0 1a     bcs   $056c
0552: cd 08     mov   x,#$08
0554: 3f 53 07  call  $0753
0557: b0 13     bcs   $056c
0559: cd 0a     mov   x,#$0a
055b: 3f 53 07  call  $0753
055e: b0 0c     bcs   $056c
0560: cd 0c     mov   x,#$0c
0562: 3f 53 07  call  $0753
0565: b0 05     bcs   $056c
0567: cd 0e     mov   x,#$0e
0569: 3f 53 07  call  $0753
056c: 8f 5c f2  mov   $f2,#$5c
056f: fa d5 f3  mov   ($f3),($d5)
0572: 8f 00 d5  mov   $d5,#$00
0575: e4 e1     mov   a,$e1
0577: 8f 0c f2  mov   $f2,#$0c
057a: c4 f3     mov   $f3,a
057c: 8f 1c f2  mov   $f2,#$1c
057f: c4 f3     mov   $f3,a
0581: 8f 3d f2  mov   $f2,#$3d
0584: fa d6 f3  mov   ($f3),($d6)
0587: 6f        ret

0588: 6f        ret

; read voice ptr for song x
0589: 7d        mov   a,x
058a: 68 05     cmp   a,#$05
058c: b0 fa     bcs   $0588
058e: fd        mov   y,a               ; y = song select (0-4)
058f: cd 00     mov   x,#$00
0591: f6 85 13  mov   a,$1385+y
0594: f0 0a     beq   $05a0             ; $00xx = unused channel
0596: c4 31     mov   $31,a
0598: f6 80 13  mov   a,$1380+y
059b: c4 30     mov   $30,a             ; channel 1 voice ptr
059d: 3f 38 06  call  $0638
05a0: 3d        inc   x
05a1: 3d        inc   x
05a2: f6 8f 13  mov   a,$138f+y
05a5: f0 0a     beq   $05b1
05a7: d4 31     mov   $31+x,a
05a9: f6 8a 13  mov   a,$138a+y
05ac: d4 30     mov   $30+x,a           ; channel 2 voice ptr
05ae: 3f 38 06  call  $0638
05b1: 3d        inc   x
05b2: 3d        inc   x
05b3: f6 99 13  mov   a,$1399+y
05b6: f0 0a     beq   $05c2
05b8: d4 31     mov   $31+x,a
05ba: f6 94 13  mov   a,$1394+y
05bd: d4 30     mov   $30+x,a           ; channel 3 voice ptr
05bf: 3f 38 06  call  $0638
05c2: 3d        inc   x
05c3: 3d        inc   x
05c4: f6 a3 13  mov   a,$13a3+y
05c7: f0 0a     beq   $05d3
05c9: d4 31     mov   $31+x,a
05cb: f6 9e 13  mov   a,$139e+y
05ce: d4 30     mov   $30+x,a           ; channel 4 voice ptr
05d0: 3f 38 06  call  $0638
05d3: 3d        inc   x
05d4: 3d        inc   x
05d5: f6 ad 13  mov   a,$13ad+y
05d8: f0 0a     beq   $05e4
05da: d4 31     mov   $31+x,a
05dc: f6 a8 13  mov   a,$13a8+y
05df: d4 30     mov   $30+x,a           ; channel 5 voice ptr
05e1: 3f 38 06  call  $0638
05e4: 3d        inc   x
05e5: 3d        inc   x
05e6: f6 b7 13  mov   a,$13b7+y
05e9: f0 0a     beq   $05f5
05eb: d4 31     mov   $31+x,a
05ed: f6 b2 13  mov   a,$13b2+y
05f0: d4 30     mov   $30+x,a           ; channel 6 voice ptr
05f2: 3f 38 06  call  $0638
05f5: 3d        inc   x
05f6: 3d        inc   x
05f7: f6 c1 13  mov   a,$13c1+y
05fa: f0 0a     beq   $0606
05fc: d4 31     mov   $31+x,a
05fe: f6 bc 13  mov   a,$13bc+y
0601: d4 30     mov   $30+x,a           ; channel 7 voice ptr
0603: 3f 38 06  call  $0638
0606: 3d        inc   x
0607: 3d        inc   x
0608: f6 cb 13  mov   a,$13cb+y
060b: f0 0a     beq   $0617
060d: d4 31     mov   $31+x,a
060f: f6 c6 13  mov   a,$13c6+y
0612: d4 30     mov   $30+x,a           ; channel 8 voice ptr
0614: 3f 38 06  call  $0638
0617: 3d        inc   x
0618: 3d        inc   x
0619: dd        mov   a,y
061a: 5d        mov   x,a
061b: 1d        dec   x                 ; x = song select - 1
061c: f5 d0 13  mov   a,$13d0+x
061f: c4 d9     mov   $d9,a
0621: f5 d4 13  mov   a,$13d4+x
0624: c4 da     mov   $da,a
0626: f0 0f     beq   $0637             ; $00xx = not used
0628: 8d fb     mov   y,#$fb
062a: f7 d9     mov   a,($d9)+y
062c: d6 df 13  mov   $13df+y,a         ; copy from ($d9) to $13df in 252 bytes
062f: dc        dec   y
0630: d0 f8     bne   $062a
0632: f7 d9     mov   a,($d9)+y
0634: d6 df 13  mov   $13df+y,a
0637: 6f        ret

0638: e8 ff     mov   a,#$ff
063a: d5 c1 02  mov   $02c1+x,a
063d: d5 01 01  mov   $0101+x,a
0640: e4 e8     mov   a,$e8
0642: d5 66 03  mov   $0366+x,a
0645: e8 00     mov   a,#$00
0647: d5 00 01  mov   $0100+x,a
064a: d4 00     mov   $00+x,a
064c: d4 01     mov   $01+x,a
064e: d4 10     mov   $10+x,a
0650: d4 11     mov   $11+x,a
0652: d4 20     mov   $20+x,a
0654: d4 21     mov   $21+x,a
0656: d4 51     mov   $51+x,a
0658: d4 60     mov   $60+x,a
065a: d4 70     mov   $70+x,a
065c: d4 71     mov   $71+x,a
065e: d4 81     mov   $81+x,a
0660: d4 90     mov   $90+x,a
0662: d4 91     mov   $91+x,a
0664: d4 a0     mov   $a0+x,a
0666: d4 a1     mov   $a1+x,a
0668: d4 b0     mov   $b0+x,a
066a: d4 b1     mov   $b1+x,a
066c: d4 c0     mov   $c0+x,a
066e: d4 c1     mov   $c1+x,a
0670: d5 00 02  mov   $0200+x,a
0673: d5 01 02  mov   $0201+x,a
0676: d5 10 02  mov   $0210+x,a
0679: d5 11 02  mov   $0211+x,a
067c: d5 20 02  mov   $0220+x,a
067f: d5 21 02  mov   $0221+x,a
0682: d5 30 02  mov   $0230+x,a
0685: d5 31 02  mov   $0231+x,a
0688: d5 40 02  mov   $0240+x,a
068b: d5 41 02  mov   $0241+x,a
068e: d5 50 02  mov   $0250+x,a
0691: d5 51 02  mov   $0251+x,a
0694: d5 60 02  mov   $0260+x,a
0697: d5 61 02  mov   $0261+x,a
069a: d5 70 02  mov   $0270+x,a
069d: d5 71 02  mov   $0271+x,a
06a0: d5 80 02  mov   $0280+x,a
06a3: d5 81 02  mov   $0281+x,a
06a6: d5 90 02  mov   $0290+x,a
06a9: d5 91 02  mov   $0291+x,a
06ac: d5 a0 02  mov   $02a0+x,a
06af: d5 a1 02  mov   $02a1+x,a
06b2: d5 b0 02  mov   $02b0+x,a
06b5: d5 b1 02  mov   $02b1+x,a
06b8: d5 c0 02  mov   $02c0+x,a
06bb: d5 d0 02  mov   $02d0+x,a
06be: d5 d1 02  mov   $02d1+x,a
06c1: d5 e0 02  mov   $02e0+x,a
06c4: d5 e1 02  mov   $02e1+x,a
06c7: d5 f0 02  mov   $02f0+x,a
06ca: d5 45 03  mov   $0345+x,a
06cd: d5 65 03  mov   $0365+x,a
06d0: e8 ff     mov   a,#$ff
06d2: d5 f1 02  mov   $02f1+x,a
06d5: 7d        mov   a,x
06d6: 1c        asl   a
06d7: 1c        asl   a
06d8: 1c        asl   a
06d9: d4 50     mov   $50+x,a
06db: e8 01     mov   a,#$01
06dd: d4 80     mov   $80+x,a
06df: f5 aa 10  mov   a,$10aa+x
06e2: 24 d7     and   a,$d7
06e4: c4 d7     mov   $d7,a
06e6: f5 aa 10  mov   a,$10aa+x
06e9: 24 d6     and   a,$d6
06eb: c4 d6     mov   $d6,a
06ed: f5 aa 10  mov   a,$10aa+x
06f0: 24 d8     and   a,$d8
06f2: c4 d8     mov   $d8,a
06f4: f5 a9 10  mov   a,$10a9+x
06f7: 04 d5     or    a,$d5
06f9: c4 d5     mov   $d5,a
06fb: 7d        mov   a,x
06fc: 1c        asl   a
06fd: 1c        asl   a
06fe: 1c        asl   a
06ff: 60        clrc
0700: 88 07     adc   a,#$07
0702: c4 f2     mov   $f2,a
0704: 8f 9f f3  mov   $f3,#$9f
0707: 6f        ret

0708: e8 00     mov   a,#$00
070a: 8d 00     mov   y,#$00
070c: d6 00 00  mov   $0000+y,a
070f: d6 00 01  mov   $0100+y,a
0712: fc        inc   y
0713: ad f0     cmp   y,#$f0
0715: d0 f5     bne   $070c
0717: bc        inc   a
0718: 8d 0e     mov   y,#$0e
071a: d6 80 00  mov   $0080+y,a
071d: dc        dec   y
071e: dc        dec   y
071f: 10 f9     bpl   $071a
0721: 9c        dec   a
0722: 8d 00     mov   y,#$00
0724: d6 00 02  mov   $0200+y,a
0727: d6 00 03  mov   $0300+y,a
072a: dc        dec   y
072b: d0 f7     bne   $0724
072d: 8f 00 db  mov   $db,#$00
0730: 8f 03 dc  mov   $dc,#$03
0733: 8f 04 d9  mov   $d9,#$04
0736: 8f 00 da  mov   $da,#$00
0739: 8d 00     mov   y,#$00
073b: e8 00     mov   a,#$00
073d: d7 db     mov   ($db)+y,a
073f: fc        inc   y
0740: d0 02     bne   $0744
0742: ab dc     inc   $dc
0744: 8b d9     dec   $d9
0746: f8 d9     mov   x,$d9
0748: 3d        inc   x
0749: d0 f0     bne   $073b
074b: 8b da     dec   $da
074d: f8 da     mov   x,$da
074f: 3d        inc   x
0750: d0 e9     bne   $073b
0752: 6f        ret

0753: f5 f1 02  mov   a,$02f1+x
0756: 30 03     bmi   $075b
0758: 5f 63 11  jmp   $1163

075b: e8 01     mov   a,#$01
075d: d5 65 03  mov   $0365+x,a
0760: f5 a9 10  mov   a,$10a9+x
0763: c4 d2     mov   $d2,a
0765: f5 aa 10  mov   a,$10aa+x
0768: c4 d3     mov   $d3,a
076a: 9b 80     dec   $80+x             ; decrease tick counter
076c: d0 25     bne   $0793
; tick
076e: 3f 85 07  call  $0785
0771: 10 1d     bpl   $0790             ; 00-7f - note/rest
0773: 68 ba     cmp   a,#$ba
0775: b0 0b     bcs   $0782
0777: 1c        asl   a
0778: fd        mov   y,a
0779: f6 70 0b  mov   a,$0b70+y
077c: 2d        push  a
077d: f6 6f 0b  mov   a,$0b6f+y
0780: 2d        push  a
0781: 6f        ret

; vcmd db-ff - end of track (duplicated)
0782: 5f 68 0f  jmp   $0f68

; read next voice byte
0785: e7 30     mov   a,($30+x)
0787: bb 30     inc   $30+x
0789: d0 02     bne   $078d
078b: bb 31     inc   $31+x
078d: 08 00     or    a,#$00
078f: 6f        ret

; vcmd 00-7f - note/rest
0790: 3f 95 0a  call  $0a95
0793: 3f e3 07  call  $07e3
0796: 3f 6e 08  call  $086e
0799: 3f 94 08  call  $0894
079c: 3f d8 08  call  $08d8
079f: 3f 26 09  call  $0926
07a2: 3f 71 09  call  $0971
07a5: 3f b9 09  call  $09b9
07a8: f5 60 02  mov   a,$0260+x
07ab: 60        clrc
07ac: 95 20 02  adc   a,$0220+x
07af: 0d        push  psw
07b0: 60        clrc
07b1: 95 c0 02  adc   a,$02c0+x
07b4: d4 10     mov   $10+x,a
07b6: f5 61 02  mov   a,$0261+x
07b9: 95 21 02  adc   a,$0221+x
07bc: 8e        pop   psw
07bd: 88 00     adc   a,#$00
07bf: d4 11     mov   $11+x,a
07c1: 5f 63 11  jmp   $1163

07c4: f5 60 02  mov   a,$0260+x
07c7: 80        setc
07c8: b5 d1 02  sbc   a,$02d1+x
07cb: c4 d9     mov   $d9,a
07cd: f5 61 02  mov   a,$0261+x
07d0: b5 e0 02  sbc   a,$02e0+x
07d3: 04 d9     or    a,$d9
07d5: 6f        ret

07d6: f5 d1 02  mov   a,$02d1+x
07d9: d5 60 02  mov   $0260+x,a
07dc: f5 e0 02  mov   a,$02e0+x
07df: d5 61 02  mov   $0261+x,a
07e2: 6f        ret

07e3: f5 00 01  mov   a,$0100+x
07e6: f0 0a     beq   $07f2
07e8: e4 ea     mov   a,$ea
07ea: d5 45 03  mov   $0345+x,a
07ed: e4 d0     mov   a,$d0
07ef: d5 46 03  mov   $0346+x,a
07f2: f5 45 03  mov   a,$0345+x
07f5: f0 6b     beq   $0862
07f7: fd        mov   y,a
07f8: f5 46 03  mov   a,$0346+x
07fb: cf        mul   ya
07fc: dd        mov   a,y
07fd: c4 e5     mov   $e5,a
07ff: f5 45 03  mov   a,$0345+x
0802: 80        setc
0803: a4 e5     sbc   a,$e5
0805: 1c        asl   a
0806: 9c        dec   a
0807: d4 01     mov   $01+x,a
0809: e4 e5     mov   a,$e5
080b: 1c        asl   a
080c: d4 00     mov   $00+x,a
080e: f5 46 03  mov   a,$0346+x
0811: 10 07     bpl   $081a
0813: f5 45 03  mov   a,$0345+x
0816: d4 00     mov   $00+x,a
0818: 2f 05     bra   $081f
081a: f5 45 03  mov   a,$0345+x
081d: d4 01     mov   $01+x,a
081f: e4 e6     mov   a,$e6
0821: 28 0f     and   a,#$0f
0823: d0 17     bne   $083c
0825: f5 56 03  mov   a,$0356+x
0828: f0 12     beq   $083c
082a: fd        mov   y,a
082b: f5 45 03  mov   a,$0345+x
082e: cf        mul   ya
082f: dd        mov   a,y
0830: d5 45 03  mov   $0345+x,a
0833: d0 07     bne   $083c
0835: d4 00     mov   $00+x,a
0837: d4 01     mov   $01+x,a
0839: d5 56 03  mov   $0356+x,a
083c: f5 55 03  mov   a,$0355+x
083f: 30 0a     bmi   $084b
0841: 60        clrc
0842: 95 46 03  adc   a,$0346+x
0845: 90 18     bcc   $085f
0847: 8d ff     mov   y,#$ff
0849: 2f 08     bra   $0853
084b: 60        clrc
084c: 95 46 03  adc   a,$0346+x
084f: b0 0e     bcs   $085f
0851: 8d 00     mov   y,#$00
0853: f5 55 03  mov   a,$0355+x
0856: 48 ff     eor   a,#$ff
0858: 60        clrc
0859: 88 01     adc   a,#$01
085b: d5 55 03  mov   $0355+x,a
085e: dd        mov   a,y
085f: d5 46 03  mov   $0346+x,a
0862: 6f        ret

0863: 80        setc
0864: a8 40     sbc   a,#$40
0866: 30 02     bmi   $086a
0868: 1c        asl   a
0869: 6f        ret

086a: 1c        asl   a
086b: 08 80     or    a,#$80
086d: 6f        ret

086e: f5 91 02  mov   a,$0291+x
0871: f0 20     beq   $0893
0873: 9b 91     dec   $91+x
0875: d0 1c     bne   $0893
0877: f5 b0 02  mov   a,$02b0+x
087a: 75 90 02  cmp   a,$0290+x
087d: f0 08     beq   $0887
087f: d5 90 02  mov   $0290+x,a
0882: f5 a1 02  mov   a,$02a1+x
0885: 2f 0a     bra   $0891
0887: 60        clrc
0888: 95 91 02  adc   a,$0291+x
088b: d5 90 02  mov   $0290+x,a
088e: f5 a0 02  mov   a,$02a0+x
0891: d4 91     mov   $91+x,a
0893: 6f        ret

0894: f4 81     mov   a,$81+x
0896: d0 07     bne   $089f
0898: f5 90 02  mov   a,$0290+x
089b: d4 90     mov   $90+x,a
089d: 2f 33     bra   $08d2
089f: 9b 81     dec   $81+x
08a1: d0 34     bne   $08d7
08a3: f5 80 02  mov   a,$0280+x
08a6: d4 81     mov   $81+x,a
08a8: f4 90     mov   a,$90+x
08aa: 75 90 02  cmp   a,$0290+x
08ad: f0 28     beq   $08d7
08af: 90 0f     bcc   $08c0
08b1: f4 90     mov   a,$90+x
08b3: 80        setc
08b4: b5 81 02  sbc   a,$0281+x
08b7: d4 90     mov   $90+x,a
08b9: 75 90 02  cmp   a,$0290+x
08bc: b0 14     bcs   $08d2
08be: 2f 0d     bra   $08cd
08c0: f5 81 02  mov   a,$0281+x
08c3: 60        clrc
08c4: 94 90     adc   a,$90+x
08c6: d4 90     mov   $90+x,a
08c8: 75 90 02  cmp   a,$0290+x
08cb: 90 05     bcc   $08d2
08cd: f5 90 02  mov   a,$0290+x
08d0: d4 90     mov   $90+x,a
08d2: fb 90     mov   y,$90+x
08d4: 3f ee 0a  call  $0aee
08d7: 6f        ret

08d8: f4 70     mov   a,$70+x
08da: f0 04     beq   $08e0
08dc: 9b 70     dec   $70+x
08de: 2f 45     bra   $0925
08e0: f5 10 02  mov   a,$0210+x
08e3: f0 40     beq   $0925
08e5: 30 15     bmi   $08fc
08e7: f5 00 02  mov   a,$0200+x
08ea: 60        clrc
08eb: 95 20 02  adc   a,$0220+x
08ee: d5 20 02  mov   $0220+x,a
08f1: e8 00     mov   a,#$00
08f3: 95 21 02  adc   a,$0221+x
08f6: d5 21 02  mov   $0221+x,a
08f9: 5f 11 09  jmp   $0911

08fc: f5 00 02  mov   a,$0200+x
08ff: 48 ff     eor   a,#$ff
0901: bc        inc   a
0902: 60        clrc
0903: 95 20 02  adc   a,$0220+x
0906: d5 20 02  mov   $0220+x,a
0909: e8 ff     mov   a,#$ff
090b: 95 21 02  adc   a,$0221+x
090e: d5 21 02  mov   $0221+x,a
0911: 9b 71     dec   $71+x
0913: d0 10     bne   $0925
0915: f5 01 02  mov   a,$0201+x
0918: d4 71     mov   $71+x,a
091a: f0 09     beq   $0925
091c: f5 10 02  mov   a,$0210+x
091f: 48 ff     eor   a,#$ff
0921: bc        inc   a
0922: d5 10 02  mov   $0210+x,a
0925: 6f        ret

0926: f5 51 02  mov   a,$0251+x
0929: d0 05     bne   $0930
092b: 3f d6 07  call  $07d6
092e: 2f 40     bra   $0970
0930: 3f c4 07  call  $07c4
0933: f0 3b     beq   $0970
0935: 90 1f     bcc   $0956
0937: f5 51 02  mov   a,$0251+x
093a: 48 ff     eor   a,#$ff
093c: bc        inc   a
093d: 60        clrc
093e: 95 60 02  adc   a,$0260+x
0941: d5 60 02  mov   $0260+x,a
0944: f5 61 02  mov   a,$0261+x
0947: 88 ff     adc   a,#$ff
0949: d5 61 02  mov   $0261+x,a
094c: 3f c4 07  call  $07c4
094f: b0 1f     bcs   $0970
0951: 3f d6 07  call  $07d6
0954: 2f 1a     bra   $0970
0956: f5 60 02  mov   a,$0260+x
0959: 60        clrc
095a: 95 51 02  adc   a,$0251+x
095d: d5 60 02  mov   $0260+x,a
0960: f5 61 02  mov   a,$0261+x
0963: 88 00     adc   a,#$00
0965: d5 61 02  mov   $0261+x,a
0968: 3f c4 07  call  $07c4
096b: 90 03     bcc   $0970
096d: 3f d6 07  call  $07d6
0970: 6f        ret

0971: f5 d0 02  mov   a,$02d0+x
0974: f0 42     beq   $09b8
0976: f4 51     mov   a,$51+x
0978: f0 1e     beq   $0998
097a: 9b 51     dec   $51+x
097c: d0 1a     bne   $0998
097e: 3f be 0f  call  $0fbe
0981: 10 0e     bpl   $0991
0983: 68 80     cmp   a,#$80
0985: d0 06     bne   $098d
0987: f4 61     mov   a,$61+x
0989: d4 60     mov   $60+x,a
098b: 2f f1     bra   $097e
098d: 9b 60     dec   $60+x
098f: 2f 02     bra   $0993
0991: d4 21     mov   $21+x,a
0993: f5 41 02  mov   a,$0241+x
0996: d4 51     mov   $51+x,a
0998: f4 c1     mov   a,$c1+x
099a: f0 1c     beq   $09b8
099c: 74 80     cmp   a,$80+x
099e: d0 18     bne   $09b8
09a0: f4 61     mov   a,$61+x
09a2: d0 14     bne   $09b8
09a4: f4 40     mov   a,$40+x
09a6: c4 d9     mov   $d9,a
09a8: f4 41     mov   a,$41+x
09aa: c4 da     mov   $da,a
09ac: 8d ff     mov   y,#$ff
09ae: fc        inc   y
09af: f7 d9     mov   a,($d9)+y
09b1: 10 fb     bpl   $09ae
09b3: fc        inc   y
09b4: db 60     mov   $60+x,y
09b6: db 61     mov   $61+x,y
09b8: 6f        ret

09b9: f5 d0 02  mov   a,$02d0+x
09bc: d0 32     bne   $09f0
09be: f4 80     mov   a,$80+x
09c0: f0 10     beq   $09d2
09c2: 74 c1     cmp   a,$c1+x
09c4: d0 0c     bne   $09d2
09c6: e8 03     mov   a,#$03
09c8: d4 a1     mov   $a1+x,a
09ca: f4 21     mov   a,$21+x
09cc: d4 b1     mov   $b1+x,a
09ce: e8 00     mov   a,#$00
09d0: d4 b0     mov   $b0+x,a
09d2: 9b a0     dec   $a0+x
09d4: d0 1a     bne   $09f0
09d6: fb a1     mov   y,$a1+x
09d8: f6 f6 09  mov   a,$09f6+y
09db: 2d        push  a
09dc: f6 f1 09  mov   a,$09f1+y
09df: 2d        push  a
09e0: 7d        mov   a,x
09e1: c4 e5     mov   $e5,a
09e3: 1c        asl   a
09e4: 1c        asl   a
09e5: 1c        asl   a
09e6: 80        setc
09e7: a4 e5     sbc   a,$e5
09e9: 5c        lsr   a
09ea: fd        mov   y,a
09eb: f6 04 03  mov   a,$0304+y
09ee: d4 a0     mov   $a0+x,a
09f0: 6f        ret

09f1: fb 2d     mov   y,$2d+x
09f3: 68 69     cmp   a,#$69
09f5: 94 09     adc   a,$09+x
09f7: 0a 0a 0a  or1   c,$0a0a,0
09fa: 0a f6 07  or1   c,$07f6,0
09fd: 03 80 b6  bbs0  $80,$09b6
0a00: 05 03 6d  or    a,$6d03
0a03: 4d        push  x
0a04: 2d        push  a
0a05: f6 06 03  mov   a,$0306+y
0a08: f0 1f     beq   $0a29
0a0a: fb b0     mov   y,$b0+x
0a0c: 5d        mov   x,a
0a0d: 1d        dec   x
0a0e: ae        pop   a
0a0f: cf        mul   ya
0a10: 9e        div   ya,x
0a11: ce        pop   x
0a12: ee        pop   y
0a13: 60        clrc
0a14: 96 05 03  adc   a,$0305+y
0a17: d4 21     mov   $21+x,a
0a19: bb b0     inc   $b0+x
0a1b: f6 06 03  mov   a,$0306+y
0a1e: 74 b0     cmp   a,$b0+x
0a20: d0 06     bne   $0a28
0a22: bb a1     inc   $a1+x
0a24: e8 00     mov   a,#$00
0a26: d4 b0     mov   $b0+x,a
0a28: 6f        ret

0a29: ae        pop   a
0a2a: ce        pop   x
0a2b: ee        pop   y
0a2c: 6f        ret

0a2d: f6 07 03  mov   a,$0307+y
0a30: 80        setc
0a31: b6 09 03  sbc   a,$0309+y
0a34: 6d        push  y
0a35: 4d        push  x
0a36: 2d        push  a
0a37: f6 08 03  mov   a,$0308+y
0a3a: 2d        push  a
0a3b: 60        clrc
0a3c: b4 b0     sbc   a,$b0+x
0a3e: f0 13     beq   $0a53
0a40: fd        mov   y,a
0a41: ae        pop   a
0a42: f0 20     beq   $0a64
0a44: 5d        mov   x,a
0a45: ae        pop   a
0a46: cf        mul   ya
0a47: 9e        div   ya,x
0a48: ce        pop   x
0a49: ee        pop   y
0a4a: 60        clrc
0a4b: 96 09 03  adc   a,$0309+y
0a4e: d4 21     mov   $21+x,a
0a50: bb b0     inc   $b0+x
0a52: 6f        ret

0a53: ae        pop   a
0a54: ae        pop   a
0a55: ce        pop   x
0a56: ee        pop   y
0a57: f6 09 03  mov   a,$0309+y
0a5a: d4 21     mov   $21+x,a
0a5c: bb a1     inc   $a1+x
0a5e: e8 00     mov   a,#$00
0a60: d4 b0     mov   $b0+x,a
0a62: 6f        ret

0a63: 6f        ret

0a64: ae        pop   a
0a65: ce        pop   x
0a66: ee        pop   y
0a67: 6f        ret

0a68: 6f        ret

0a69: f4 b1     mov   a,$b1+x
0a6b: 6d        push  y
0a6c: 4d        push  x
0a6d: 2d        push  a
0a6e: f6 0a 03  mov   a,$030a+y
0a71: 2d        push  a
0a72: 60        clrc
0a73: b4 b0     sbc   a,$b0+x
0a75: f0 0f     beq   $0a86
0a77: fd        mov   y,a
0a78: ae        pop   a
0a79: f0 16     beq   $0a91
0a7b: 5d        mov   x,a
0a7c: ae        pop   a
0a7d: cf        mul   ya
0a7e: 9e        div   ya,x
0a7f: ce        pop   x
0a80: ee        pop   y
0a81: d4 21     mov   $21+x,a
0a83: bb b0     inc   $b0+x
0a85: 6f        ret

0a86: ae        pop   a
0a87: ae        pop   a
0a88: ce        pop   x
0a89: ee        pop   y
0a8a: e8 00     mov   a,#$00
0a8c: d4 21     mov   $21+x,a
0a8e: bb a1     inc   $a1+x
0a90: 6f        ret

0a91: ae        pop   a
0a92: ce        pop   x
0a93: ee        pop   y
0a94: 6f        ret

0a95: f0 50     beq   $0ae7
0a97: fb 20     mov   y,$20+x
0a99: 60        clrc
0a9a: 96 b8 38  adc   a,$38b8+y
0a9d: 5b c0     lsr   $c0+x
0a9f: b0 04     bcs   $0aa5
0aa1: 60        clrc
0aa2: 95 40 02  adc   a,$0240+x
0aa5: d5 b0 02  mov   $02b0+x,a
0aa8: d5 90 02  mov   $0290+x,a
0aab: fd        mov   y,a
0aac: f5 80 02  mov   a,$0280+x
0aaf: d4 81     mov   $81+x,a
0ab1: d0 05     bne   $0ab8
0ab3: db 90     mov   $90+x,y
0ab5: 3f ee 0a  call  $0aee
0ab8: 3f 17 0b  call  $0b17
0abb: f5 d0 02  mov   a,$02d0+x
0abe: f0 0e     beq   $0ace
0ac0: f5 e1 02  mov   a,$02e1+x
0ac3: d0 09     bne   $0ace
0ac5: d4 60     mov   $60+x,a
0ac7: d4 61     mov   $61+x,a
0ac9: f5 41 02  mov   a,$0241+x
0acc: d4 51     mov   $51+x,a
0ace: f5 a1 02  mov   a,$02a1+x
0ad1: d4 91     mov   $91+x,a
0ad3: 3f 46 0b  call  $0b46
0ad6: f5 f0 02  mov   a,$02f0+x
0ad9: d0 0c     bne   $0ae7
0adb: e4 d2     mov   a,$d2
0add: 04 d4     or    a,$d4
0adf: c4 d4     mov   $d4,a
0ae1: e4 d2     mov   a,$d2
0ae3: 04 d5     or    a,$d5
0ae5: c4 d5     mov   $d5,a
0ae7: 3f 5a 0b  call  $0b5a
0aea: 3f 31 0b  call  $0b31
0aed: 6f        ret

0aee: f6 00 12  mov   a,$1200+y
0af1: c4 d9     mov   $d9,a
0af3: f6 55 12  mov   a,$1255+y
0af6: c4 da     mov   $da,a
0af8: fb 20     mov   y,$20+x
0afa: f6 e5 38  mov   a,$38e5+y
0afd: fd        mov   y,a
0afe: 6d        push  y
0aff: e4 d9     mov   a,$d9
0b01: cf        mul   ya
0b02: cb dd     mov   $dd,y
0b04: ee        pop   y
0b05: e4 da     mov   a,$da
0b07: cf        mul   ya
0b08: 8f 00 de  mov   $de,#$00
0b0b: 7a dd     addw  ya,$dd
0b0d: 7a d9     addw  ya,$d9
0b0f: d5 d1 02  mov   $02d1+x,a
0b12: dd        mov   a,y
0b13: d5 e0 02  mov   $02e0+x,a
0b16: 6f        ret

0b17: f5 11 02  mov   a,$0211+x
0b1a: d4 70     mov   $70+x,a
0b1c: f5 01 02  mov   a,$0201+x
0b1f: 5c        lsr   a
0b20: d4 71     mov   $71+x,a
0b22: f5 50 02  mov   a,$0250+x
0b25: d5 10 02  mov   $0210+x,a
0b28: e8 00     mov   a,#$00
0b2a: d5 21 02  mov   $0221+x,a
0b2d: d5 20 02  mov   $0220+x,a
0b30: 6f        ret

0b31: f5 70 02  mov   a,$0270+x
0b34: f0 0a     beq   $0b40
; duration (direct)
0b36: f4 80     mov   a,$80+x
0b38: 80        setc
0b39: b5 70 02  sbc   a,$0270+x
0b3c: d4 c1     mov   $c1+x,a
0b3e: 2f 05     bra   $0b45
; duration (subtract)
0b40: f5 71 02  mov   a,$0271+x
0b43: d4 c1     mov   $c1+x,a
0b45: 6f        ret

0b46: f5 b1 02  mov   a,$02b1+x
0b49: f0 0e     beq   $0b59
0b4b: f5 e1 02  mov   a,$02e1+x
0b4e: d0 09     bne   $0b59
0b50: e8 00     mov   a,#$00
0b52: d4 a1     mov   $a1+x,a
0b54: d4 b0     mov   $b0+x,a
0b56: bc        inc   a
0b57: d4 a0     mov   $a0+x,a
0b59: 6f        ret

0b5a: f5 31 02  mov   a,$0231+x
0b5d: d0 05     bne   $0b64
0b5f: f5 30 02  mov   a,$0230+x
0b62: d0 08     bne   $0b6c
0b64: e8 00     mov   a,#$00
0b66: d5 31 02  mov   $0231+x,a
0b69: 3f 85 07  call  $0785
0b6c: d4 80     mov   $80+x,a
0b6e: 6f        ret

0b6f: dw $0f68  ; 80 - end of track
0b71: dw $0be3  ; 81 - goto
0b73: dw $0bf2  ; 82 - call subroutine
0b75: dw $0c06  ; 83 - end subroutine
0b77: dw $0c0c  ; 84 - repeat start
0b79: dw $0c1a  ; 85 - repeat end
0b7b: dw $0c3b  ; 86 - set default note length
0b7d: dw $0c44  ; 87
0b7f: dw $0c4c  ; 88 - transpose (absolute)
0b81: dw $0c55  ; 89 - set instrument
0b83: dw $0c5d  ; 8a
0b85: dw $0c6a  ; 8b
0b87: dw $0c72  ; 8c
0b89: dw $0c9c  ; 8d
0b8b: dw $0ca5  ; 8e
0b8d: dw $0ca9  ; 8f
0b8f: dw $0ccd  ; 90
0b91: dw $0cc5  ; 91
0b93: dw $0cd6  ; 92 - set duration (direct)
0b95: dw $0ce4  ; 93 - set duration (subtract)
0b97: dw $0cf2  ; 94
0b99: dw $0d01  ; 95
0b9b: dw $0d09  ; 96
0b9d: dw $0d20  ; 97
0b9f: dw $0d5c  ; 98
0ba1: dw $0d65  ; 99
0ba3: dw $0d6e  ; 9a
0ba5: dw $0d88  ; 9b
0ba7: dw $0d80  ; 9c
0ba9: dw $0d97  ; 9d
0bab: dw $0da6  ; 9e
0bad: dw $0dae  ; 9f
0baf: dw $0db6  ; a0
0bb1: dw $0dbd  ; a1
0bb3: dw $0dd9  ; a2
0bb5: dw $0df6  ; a3
0bb7: dw $0e0f  ; a4
0bb9: dw $0e4e  ; a5
0bbb: dw $0e74  ; a6
0bbd: dw $0e85  ; a7
0bbf: dw $0ea3  ; a8
0bc1: dw $0eb1  ; a9
0bc3: dw $0ed3  ; aa
0bc5: dw $0ee2  ; ab
0bc7: dw $0ef1  ; ac
0bc9: dw $0efc  ; ad
0bcb: dw $0f07  ; ae
0bcd: dw $0f12  ; af
0bcf: dw $0f2a  ; b0
0bd1: dw $0f33  ; b1
0bd3: dw $0f3c  ; b2
0bd5: dw $0f45  ; b3
0bd7: dw $0f54  ; b4
0bd9: dw $0f5d  ; b5
0bdb: dw $0f7e  ; b6 - set tempo
0bdd: dw $0f86  ; b7
0bdf: dw $0f96  ; b8
0be1: dw $0fb0  ; b9

; vcmd 81 - goto
0be3: 3f 85 07  call  $0785
0be6: 2d        push  a
0be7: 3f 85 07  call  $0785
0bea: d4 31     mov   $31+x,a
0bec: ae        pop   a
0bed: d4 30     mov   $30+x,a
0bef: 5f 6e 07  jmp   $076e

; vcmd 82 - call subroutine
0bf2: 3f 85 07  call  $0785
0bf5: 2d        push  a
0bf6: 3f 85 07  call  $0785
0bf9: 2d        push  a
0bfa: 3f e6 0f  call  $0fe6             ; push return address
0bfd: ae        pop   a
0bfe: d4 31     mov   $31+x,a
0c00: ae        pop   a
0c01: d4 30     mov   $30+x,a           ; goto arg1/2
0c03: 5f 6e 07  jmp   $076e

; vcmd 83 - end subroutine
0c06: 3f cd 0f  call  $0fcd             ; pop return address
0c09: 5f 6e 07  jmp   $076e

; vcmd 84 - repeat start
0c0c: 3f 85 07  call  $0785             ; arg1 - repeat count
0c0f: 2d        push  a
0c10: 3f e6 0f  call  $0fe6             ; push voice ptr
0c13: ae        pop   a
0c14: 3f de 0f  call  $0fde             ; push repeat count
0c17: 5f 6e 07  jmp   $076e

; vcmd 85 - repeat end
0c1a: fb 50     mov   y,$50+x
0c1c: f6 7f 03  mov   a,$037f+y
0c1f: 9c        dec   a                 ; decrement repeat count
0c20: f0 10     beq   $0c32
; repeat again
0c22: d6 7f 03  mov   $037f+y,a         ; update repeat count
0c25: f6 7e 03  mov   a,$037e+y
0c28: d4 31     mov   $31+x,a
0c2a: f6 7d 03  mov   a,$037d+y
0c2d: d4 30     mov   $30+x,a           ; goto repeat start
0c2f: 5f 6e 07  jmp   $076e
; repeat end
0c32: dd        mov   a,y
0c33: 80        setc
0c34: a8 03     sbc   a,#$03
0c36: d4 50     mov   $50+x,a           ; pop repeat params
0c38: 5f 6e 07  jmp   $076e

; vcmd 86 - set default note length
0c3b: 3f 85 07  call  $0785
0c3e: d5 30 02  mov   $0230+x,a
0c41: 5f 6e 07  jmp   $076e

; vcmd 87
0c44: e8 ff     mov   a,#$ff
0c46: d5 31 02  mov   $0231+x,a
0c49: 5f 6e 07  jmp   $076e

; vcmd 88 - transpose (absolute)
0c4c: 3f 85 07  call  $0785
0c4f: d5 40 02  mov   $0240+x,a
0c52: 5f 6e 07  jmp   $076e

; vcmd 89 - set instrument
0c55: 3f 85 07  call  $0785
0c58: d4 20     mov   $20+x,a
0c5a: 5f 6e 07  jmp   $076e

; vcmd 8a
0c5d: 3f 85 07  call  $0785
0c60: d4 00     mov   $00+x,a
0c62: e8 00     mov   a,#$00
0c64: d5 45 03  mov   $0345+x,a
0c67: 5f 6e 07  jmp   $076e

; vcmd 8b
0c6a: 3f 85 07  call  $0785
0c6d: d4 01     mov   $01+x,a
0c6f: 5f 62 0c  jmp   $0c62

; vcmd 8c
0c72: 3f 85 07  call  $0785
0c75: d5 41 02  mov   $0241+x,a
0c78: 3f 85 07  call  $0785
0c7b: d4 40     mov   $40+x,a
0c7d: 3f 85 07  call  $0785
0c80: d4 41     mov   $41+x,a
0c82: e8 00     mov   a,#$00
0c84: d4 60     mov   $60+x,a
0c86: d5 b1 02  mov   $02b1+x,a
0c89: d5 e1 02  mov   $02e1+x,a
0c8c: 9c        dec   a
0c8d: d5 d0 02  mov   $02d0+x,a
0c90: e8 01     mov   a,#$01
0c92: d4 51     mov   $51+x,a
0c94: 3f be 0f  call  $0fbe
0c97: d4 21     mov   $21+x,a
0c99: 5f 6e 07  jmp   $076e

; vcmd 8d
0c9c: 3f 85 07  call  $0785
0c9f: d5 c0 02  mov   $02c0+x,a
0ca2: 5f 6e 07  jmp   $076e

; vcmd 8e
0ca5: e8 01     mov   a,#$01
0ca7: 2f 02     bra   $0cab

; vmcd 8f
0ca9: e8 ff     mov   a,#$ff
0cab: d5 50 02  mov   $0250+x,a
0cae: 3f 85 07  call  $0785
0cb1: d5 11 02  mov   $0211+x,a
0cb4: 3f 85 07  call  $0785
0cb7: d5 00 02  mov   $0200+x,a
0cba: 3f 85 07  call  $0785
0cbd: d5 01 02  mov   $0201+x,a
0cc0: d4 71     mov   $71+x,a
0cc2: 5f 6e 07  jmp   $076e

; vcmd 91
0cc5: e8 00     mov   a,#$00
0cc7: d5 50 02  mov   $0250+x,a
0cca: 5f 6e 07  jmp   $076e

; vcmd 90
0ccd: 3f 85 07  call  $0785
0cd0: d5 51 02  mov   $0251+x,a
0cd3: 5f 6e 07  jmp   $076e

; vcmd 92 - set duration (direct)
0cd6: 3f 85 07  call  $0785
0cd9: d5 70 02  mov   $0270+x,a
0cdc: e8 00     mov   a,#$00
0cde: d5 71 02  mov   $0271+x,a
0ce1: 5f 6e 07  jmp   $076e

; vcmd 93 - set duration (subtract)
0ce4: 3f 85 07  call  $0785
0ce7: d5 71 02  mov   $0271+x,a
0cea: e8 00     mov   a,#$00
0cec: d5 70 02  mov   $0270+x,a
0cef: 5f 6e 07  jmp   $076e

; vcmd 94
0cf2: 3f 85 07  call  $0785
0cf5: d5 81 02  mov   $0281+x,a
0cf8: 3f 85 07  call  $0785
0cfb: d5 80 02  mov   $0280+x,a
0cfe: 5f 6e 07  jmp   $076e

; vcmd 95
0d01: e8 00     mov   a,#$00
0d03: d5 80 02  mov   $0280+x,a
0d06: 5f 6e 07  jmp   $076e

; vcmd 96
0d09: 3f 85 07  call  $0785
0d0c: d5 91 02  mov   $0291+x,a
0d0f: 3f 85 07  call  $0785
0d12: d5 a0 02  mov   $02a0+x,a
0d15: 3f 85 07  call  $0785
0d18: d5 a1 02  mov   $02a1+x,a
0d1b: d4 91     mov   $91+x,a
0d1d: 5f 6e 07  jmp   $076e

; vcmd 97
0d20: e8 d8     mov   a,#$d8
0d22: c4 d9     mov   $d9,a
0d24: e8 13     mov   a,#$13
0d26: c4 da     mov   $da,a
0d28: 3f 85 07  call  $0785
0d2b: 8d 07     mov   y,#$07
0d2d: cf        mul   ya
0d2e: 4d        push  x
0d2f: 60        clrc
0d30: 7a d9     addw  ya,$d9
0d32: da d9     movw  $d9,ya
0d34: 8d 00     mov   y,#$00
0d36: 7d        mov   a,x
0d37: c4 e5     mov   $e5,a
0d39: 1c        asl   a
0d3a: 1c        asl   a
0d3b: 1c        asl   a
0d3c: 80        setc
0d3d: a4 e5     sbc   a,$e5
0d3f: 5c        lsr   a
0d40: 5d        mov   x,a
0d41: f7 d9     mov   a,($d9)+y
0d43: d5 04 03  mov   $0304+x,a
0d46: 3d        inc   x
0d47: fc        inc   y
0d48: ad 07     cmp   y,#$07
0d4a: d0 f5     bne   $0d41
0d4c: ce        pop   x
0d4d: e8 00     mov   a,#$00
0d4f: d5 d0 02  mov   $02d0+x,a
0d52: d5 e1 02  mov   $02e1+x,a
0d55: 9c        dec   a
0d56: d5 b1 02  mov   $02b1+x,a
0d59: 5f 6e 07  jmp   $076e

; vcmd 98
0d5c: e4 d2     mov   a,$d2
0d5e: 04 d6     or    a,$d6
0d60: c4 d6     mov   $d6,a
0d62: 5f 6e 07  jmp   $076e

; vcmd 99
0d65: e4 d3     mov   a,$d3
0d67: 24 d6     and   a,$d6
0d69: c4 d6     mov   $d6,a
0d6b: 5f 6e 07  jmp   $076e

; vcmd 9a
0d6e: 3f 85 07  call  $0785
0d71: 28 1f     and   a,#$1f
0d73: 08 20     or    a,#$20
0d75: 8f 6c f2  mov   $f2,#$6c
0d78: 00        nop
0d79: 00        nop
0d7a: 00        nop
0d7b: c4 f3     mov   $f3,a
0d7d: 5f 6e 07  jmp   $076e

; vcmd 9c
0d80: e8 ff     mov   a,#$ff
0d82: d5 e1 02  mov   $02e1+x,a
0d85: 5f 6e 07  jmp   $076e

; vcmd 9b
0d88: e8 00     mov   a,#$00
0d8a: d5 e1 02  mov   $02e1+x,a
0d8d: d5 d0 02  mov   $02d0+x,a
0d90: 9c        dec   a
0d91: d5 b1 02  mov   $02b1+x,a
0d94: 5f 6e 07  jmp   $076e

; vcmd 9d
0d97: e8 00     mov   a,#$00
0d99: d5 e1 02  mov   $02e1+x,a
0d9c: d5 b1 02  mov   $02b1+x,a
0d9f: 9c        dec   a
0da0: d5 d0 02  mov   $02d0+x,a
0da3: 5f 6e 07  jmp   $076e

; vcmd 9e
0da6: e8 00     mov   a,#$00
0da8: d5 f0 02  mov   $02f0+x,a
0dab: 5f 6e 07  jmp   $076e

; vcmd 9f
0dae: e8 ff     mov   a,#$ff
0db0: d5 f0 02  mov   $02f0+x,a
0db3: 5f 6e 07  jmp   $076e

; vcmd a0
0db6: e8 01     mov   a,#$01
0db8: d4 c0     mov   $c0+x,a
0dba: 5f 6e 07  jmp   $076e

; vcmd a1
0dbd: 4d        push  x
0dbe: f8 e4     mov   x,$e4
0dc0: cd 01     mov   x,#$01
0dc2: 3f 89 05  call  $0589
0dc5: ce        pop   x
0dc6: f5 a9 10  mov   a,$10a9+x
0dc9: c4 d2     mov   $d2,a
0dcb: f5 aa 10  mov   a,$10aa+x
0dce: c4 d3     mov   $d3,a
0dd0: 8f 00 e0  mov   $e0,#$00
0dd3: 8f 7f e1  mov   $e1,#$7f
0dd6: 5f 6e 07  jmp   $076e

; vcmd a2
0dd9: 7d        mov   a,x
0dda: c4 e5     mov   $e5,a
0ddc: 1c        asl   a
0ddd: 1c        asl   a
0dde: 1c        asl   a
0ddf: 80        setc
0de0: a4 e5     sbc   a,$e5
0de2: 5c        lsr   a
0de3: fd        mov   y,a
0de4: e8 07     mov   a,#$07
0de6: c4 e5     mov   $e5,a
0de8: 3f 85 07  call  $0785
0deb: d6 04 03  mov   $0304+y,a
0dee: fc        inc   y
0def: 8b e5     dec   $e5
0df1: d0 f5     bne   $0de8
0df3: 5f 88 0d  jmp   $0d88

; vcmd a3
0df6: 3f 85 07  call  $0785
0df9: 3f b9 10  call  $10b9
0dfc: 1c        asl   a
0dfd: c4 e5     mov   $e5,a
0dff: 60        clrc
0e00: f4 30     mov   a,$30+x
0e02: 84 e5     adc   a,$e5
0e04: d4 30     mov   $30+x,a
0e06: f4 31     mov   a,$31+x
0e08: 88 00     adc   a,#$00
0e0a: d4 31     mov   $31+x,a
0e0c: 5f e3 0b  jmp   $0be3

; vcmd a4
0e0f: 3f 85 07  call  $0785
0e12: c4 e5     mov   $e5,a
0e14: 3f b9 10  call  $10b9
0e17: c4 d9     mov   $d9,a
0e19: 1c        asl   a
0e1a: 60        clrc
0e1b: 94 30     adc   a,$30+x
0e1d: d4 30     mov   $30+x,a
0e1f: f4 31     mov   a,$31+x
0e21: 88 00     adc   a,#$00
0e23: d4 31     mov   $31+x,a
0e25: 3f 85 07  call  $0785
0e28: 2d        push  a
0e29: 3f 85 07  call  $0785
0e2c: 2d        push  a
0e2d: e4 e5     mov   a,$e5
0e2f: 9c        dec   a
0e30: 80        setc
0e31: a4 d9     sbc   a,$d9
0e33: 1c        asl   a
0e34: c4 d9     mov   $d9,a
0e36: f4 30     mov   a,$30+x
0e38: 84 d9     adc   a,$d9
0e3a: d4 30     mov   $30+x,a
0e3c: f4 31     mov   a,$31+x
0e3e: 88 00     adc   a,#$00
0e40: d4 31     mov   $31+x,a
0e42: 3f e6 0f  call  $0fe6
0e45: ae        pop   a
0e46: d4 31     mov   $31+x,a
0e48: ae        pop   a
0e49: d4 30     mov   $30+x,a
0e4b: 5f 6e 07  jmp   $076e

; vcmd a5
0e4e: 3f 85 07  call  $0785
0e51: 3f 5d 0e  call  $0e5d
0e54: 16 3c 03  or    a,$033c+y
0e57: d6 3c 03  mov   $033c+y,a
0e5a: 5f 6e 07  jmp   $076e

0e5d: 4d        push  x
0e5e: fd        mov   y,a
0e5f: 28 07     and   a,#$07
0e61: 5d        mov   x,a
0e62: dd        mov   a,y
0e63: 5c        lsr   a
0e64: 5c        lsr   a
0e65: 5c        lsr   a
0e66: fd        mov   y,a
0e67: f5 6c 0e  mov   a,$0e6c+x
0e6a: ce        pop   x
0e6b: 6f        ret

0e6c: db $01,$02,$04,$08,$10,$20,$40,$80

; vcmd a6
0e74: 3f 85 07  call  $0785
0e77: 3f 5d 0e  call  $0e5d
0e7a: 48 ff     eor   a,#$ff
0e7c: 36 3c 03  and   a,$033c+y
0e7f: d6 3c 03  mov   $033c+y,a
0e82: 5f 6e 07  jmp   $076e

; vcmd a7
0e85: 3f 85 07  call  $0785
0e88: 3f 5d 0e  call  $0e5d
0e8b: 36 3c 03  and   a,$033c+y
0e8e: f0 03     beq   $0e93
0e90: 5f e3 0b  jmp   $0be3

0e93: f4 30     mov   a,$30+x
0e95: 60        clrc
0e96: 88 02     adc   a,#$02
0e98: d4 30     mov   $30+x,a
0e9a: f4 31     mov   a,$31+x
0e9c: 88 00     adc   a,#$00
0e9e: d4 31     mov   $31+x,a
0ea0: 5f 6e 07  jmp   $076e

; vcmd a8
0ea3: 3f 85 07  call  $0785
0ea6: 3f 5d 0e  call  $0e5d
0ea9: 36 3c 03  and   a,$033c+y
0eac: d0 e5     bne   $0e93
0eae: 5f e3 0b  jmp   $0be3

; vcmd a9
0eb1: 3f 85 07  call  $0785
0eb4: 3f 5d 0e  call  $0e5d
0eb7: 36 3c 03  and   a,$033c+y
0eba: f0 03     beq   $0ebf
0ebc: 5f 6e 07  jmp   $076e

0ebf: e8 01     mov   a,#$01
0ec1: d4 80     mov   $80+x,a
0ec3: f4 30     mov   a,$30+x
0ec5: 80        setc
0ec6: a8 02     sbc   a,#$02
0ec8: d4 30     mov   $30+x,a
0eca: f4 31     mov   a,$31+x
0ecc: a8 00     sbc   a,#$00
0ece: d4 31     mov   $31+x,a
0ed0: 5f 93 07  jmp   $0793

; vcmd aa
0ed3: e4 d2     mov   a,$d2
0ed5: 04 d7     or    a,$d7
0ed7: c4 d7     mov   $d7,a
0ed9: 8f 4d f2  mov   $f2,#$4d
0edc: fa d7 f3  mov   ($f3),($d7)
0edf: 5f 6e 07  jmp   $076e

; vcmd ab
0ee2: e4 d3     mov   a,$d3
0ee4: 24 d7     and   a,$d7
0ee6: c4 d7     mov   $d7,a
0ee8: 8f 4d f2  mov   $f2,#$4d
0eeb: fa d7 f3  mov   ($f3),($d7)
0eee: 5f 6e 07  jmp   $076e

; vcmd ac
0ef1: 3f 85 07  call  $0785
0ef4: 8f 2c f2  mov   $f2,#$2c
0ef7: c4 f3     mov   $f3,a
0ef9: 5f 6e 07  jmp   $076e

; vcmd ad
0efc: 3f 85 07  call  $0785
0eff: 8f 3c f2  mov   $f2,#$3c
0f02: c4 f3     mov   $f3,a
0f04: 5f 6e 07  jmp   $076e

; vcmd ae
0f07: 3f 85 07  call  $0785
0f0a: 8f 0d f2  mov   $f2,#$0d
0f0d: c4 f3     mov   $f3,a
0f0f: 5f 6e 07  jmp   $076e

; vcmd af
0f12: 8d 00     mov   y,#$00
0f14: dd        mov   a,y
0f15: 1c        asl   a
0f16: 1c        asl   a
0f17: 1c        asl   a
0f18: 1c        asl   a
0f19: 08 0f     or    a,#$0f
0f1b: c4 f2     mov   $f2,a
0f1d: 3f 85 07  call  $0785
0f20: c4 f3     mov   $f3,a
0f22: fc        inc   y
0f23: ad 08     cmp   y,#$08
0f25: d0 ed     bne   $0f14
0f27: 5f 6e 07  jmp   $076e

; vcmd b0
0f2a: 3f 85 07  call  $0785
0f2d: d5 56 03  mov   $0356+x,a
0f30: 5f 6e 07  jmp   $076e

; vcmd b1
0f33: e4 d2     mov   a,$d2
0f35: 04 d8     or    a,$d8
0f37: c4 d8     mov   $d8,a
0f39: 5f 6e 07  jmp   $076e

; vcmd b2
0f3c: e4 d3     mov   a,$d3
0f3e: 24 d8     and   a,$d8
0f40: c4 d8     mov   $d8,a
0f42: 5f 6e 07  jmp   $076e

; vcmd b3
0f45: 3f 85 07  call  $0785
0f48: d5 45 03  mov   $0345+x,a
0f4b: 3f 85 07  call  $0785
0f4e: d5 46 03  mov   $0346+x,a
0f51: 5f 6e 07  jmp   $076e

; vcmd b4
0f54: 3f 85 07  call  $0785
0f57: d5 55 03  mov   $0355+x,a
0f5a: 5f 6e 07  jmp   $076e

; vcmd b5
0f5d: 3f 85 07  call  $0785
0f60: 4d        push  x
0f61: 3f f7 10  call  $10f7
0f64: ce        pop   x
0f65: 5f 6e 07  jmp   $076e

; vcmd 80 - end of track
0f68: e8 00     mov   a,#$00
0f6a: d5 65 03  mov   $0365+x,a
0f6d: d5 f1 02  mov   $02f1+x,a
0f70: d5 c1 02  mov   $02c1+x,a
0f73: 9c        dec   a
0f74: d5 01 01  mov   $0101+x,a
0f77: e4 d2     mov   a,$d2
0f79: 04 d5     or    a,$d5
0f7b: c4 d5     mov   $d5,a
0f7d: 6f        ret

; vcmd b6 - set tempo
0f7e: 3f 85 07  call  $0785
0f81: c4 fc     mov   $fc,a
0f83: 5f 6e 07  jmp   $076e

; vcmd b7
0f86: e8 ff     mov   a,#$ff
0f88: d5 00 01  mov   $0100+x,a
0f8b: e8 01     mov   a,#$01
0f8d: d5 46 03  mov   $0346+x,a
0f90: d5 45 03  mov   $0345+x,a
0f93: 5f 6e 07  jmp   $076e

; vcmd b8
0f96: 3f 85 07  call  $0785
0f99: fd        mov   y,a
0f9a: f6 a0 0f  mov   a,$0fa0+y
0f9d: 5f 71 07  jmp   $0771

0fa0: db $2d,$2d,$2d,$2d,$2d,$2d,$2d,$2d
0fa8: db $2d,$2d,$2d,$2d,$2d,$2d,$2d,$2d

; vcmd b9
0fb0: 7d        mov   a,x
0fb1: 1c        asl   a
0fb2: 1c        asl   a
0fb3: 1c        asl   a
0fb4: 74 50     cmp   a,$50+x
0fb6: f0 03     beq   $0fbb
0fb8: 5f ac 12  jmp   $12ac

0fbb: 5f 6e 07  jmp   $076e

0fbe: f4 40     mov   a,$40+x
0fc0: c4 d9     mov   $d9,a
0fc2: f4 41     mov   a,$41+x
0fc4: c4 da     mov   $da,a
0fc6: fb 60     mov   y,$60+x
0fc8: bb 60     inc   $60+x
0fca: f7 d9     mov   a,($d9)+y
0fcc: 6f        ret

0fcd: fb 50     mov   y,$50+x
0fcf: dc        dec   y
0fd0: f6 80 03  mov   a,$0380+y
0fd3: d4 31     mov   $31+x,a
0fd5: dc        dec   y
0fd6: f6 80 03  mov   a,$0380+y
0fd9: d4 30     mov   $30+x,a
0fdb: db 50     mov   $50+x,y
0fdd: 6f        ret

; push repeat count
0fde: fb 50     mov   y,$50+x
0fe0: d6 80 03  mov   $0380+y,a
0fe3: bb 50     inc   $50+x
0fe5: 6f        ret

; push voice ptr
0fe6: fb 50     mov   y,$50+x           ; load stack ptr
0fe8: f4 30     mov   a,$30+x
0fea: d6 80 03  mov   $0380+y,a
0fed: f4 31     mov   a,$31+x
0fef: d6 81 03  mov   $0381+y,a         ; push voice ptr
0ff2: fc        inc   y
0ff3: fc        inc   y
0ff4: db 50     mov   $50+x,y           ; save stack ptr
0ff6: 6f        ret

0ff7: f6 65 03  mov   a,$0365+y
0ffa: f0 37     beq   $1033
0ffc: d8 f2     mov   $f2,x
0ffe: f6 00 00  mov   a,$0000+y
1001: 3f 36 10  call  $1036
1004: c4 f3     mov   $f3,a
1006: 3d        inc   x
1007: d8 f2     mov   $f2,x
1009: f6 01 00  mov   a,$0001+y
100c: 3f 36 10  call  $1036
100f: c4 f3     mov   $f3,a
1011: 3d        inc   x
1012: d8 f2     mov   $f2,x
1014: f6 10 00  mov   a,$0010+y
1017: c4 f3     mov   $f3,a
1019: 3d        inc   x
101a: d8 f2     mov   $f2,x
101c: f6 11 00  mov   a,$0011+y
101f: c4 f3     mov   $f3,a
1021: 3d        inc   x
1022: d8 f2     mov   $f2,x
1024: f6 20 00  mov   a,$0020+y
1027: c4 f3     mov   $f3,a
1029: 3d        inc   x
102a: 3d        inc   x
102b: 3d        inc   x
102c: d8 f2     mov   $f2,x
102e: f6 21 00  mov   a,$0021+y
1031: c4 f3     mov   $f3,a
1033: 5f 63 11  jmp   $1163

1036: 30 17     bmi   $104f
1038: 6d        push  y
1039: 4d        push  x
103a: 6d        push  y
103b: ce        pop   x
103c: fd        mov   y,a
103d: f5 c1 02  mov   a,$02c1+x
1040: bc        inc   a
1041: d0 04     bne   $1047
1043: e4 e8     mov   a,$e8
1045: 2f 03     bra   $104a
1047: f5 66 03  mov   a,$0366+x
104a: cf        mul   ya
104b: dd        mov   a,y
104c: ce        pop   x
104d: ee        pop   y
104e: 6f        ret

104f: 48 ff     eor   a,#$ff
1051: bc        inc   a
1052: 6d        push  y
1053: 4d        push  x
1054: 6d        push  y
1055: ce        pop   x
1056: fd        mov   y,a
1057: f5 66 03  mov   a,$0366+x
105a: cf        mul   ya
105b: dd        mov   a,y
105c: ce        pop   x
105d: ee        pop   y
105e: 48 ff     eor   a,#$ff
1060: bc        inc   a
1061: 6f        ret

1062: cd 00     mov   x,#$00
1064: f5 74 10  mov   a,$1074+x
1067: 30 0a     bmi   $1073
1069: c4 f2     mov   $f2,a
106b: f5 8f 10  mov   a,$108f+x
106e: c4 f3     mov   $f3,a
1070: 3d        inc   x
1071: d0 f1     bne   $1064
1073: 6f        ret

1074: db $2c,$3c,$5c,$2d,$3d,$4d,$7d,$6d,$0d,$5d,$0f,$1f,$2f,$3f,$4f,$5f,$6f,$7f,$05,$15,$25,$35,$45,$55,$65,$75,$ff
108f: db $00,$00,$ff,$00,$00,$00,$00,$03,$00,$38,$7f,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00

10a9: db $01,$fe
10ab: db $02,$fd
10ad: db $04,$fb
10af: db $08,$f7
10b1: db $10,$ef
10b3: db $20,$df
10b5: db $40,$bf
10b7: db $80,$7f

10b9: fd        mov   y,a
10ba: 3f d7 10  call  $10d7
10bd: 3f d7 10  call  $10d7
10c0: 3f d7 10  call  $10d7
10c3: 3f d7 10  call  $10d7
10c6: 3f d7 10  call  $10d7
10c9: 3f d7 10  call  $10d7
10cc: 3f d7 10  call  $10d7
10cf: 3f d7 10  call  $10d7
10d2: e4 eb     mov   a,$eb
10d4: cf        mul   ya
10d5: dd        mov   a,y
10d6: 6f        ret

10d7: e4 eb     mov   a,$eb
10d9: 28 48     and   a,#$48
10db: 88 38     adc   a,#$38
10dd: 1c        asl   a
10de: 1c        asl   a
10df: 2b ee     rol   $ee
10e1: 2b ed     rol   $ed
10e3: 2b ec     rol   $ec
10e5: 2b eb     rol   $eb
10e7: 6f        ret

10e8: 8f 00 f1  mov   $f1,#$00
10eb: 00        nop
10ec: 00        nop
10ed: 00        nop
10ee: 00        nop
10ef: 8f 85 fc  mov   $fc,#$85
10f2: 8f 04 f1  mov   $f1,#$04
10f5: 6f        ret

10f6: 6f        ret

10f7: fd        mov   y,a
10f8: 68 4f     cmp   a,#$4f
10fa: b0 fa     bcs   $10f6
10fc: cd 0e     mov   x,#$0e
10fe: f6 68 17  mov   a,$1768+y
1101: 30 0a     bmi   $110d
1103: dd        mov   a,y
1104: 75 01 01  cmp   a,$0101+x
1107: f0 32     beq   $113b
1109: 1d        dec   x
110a: 1d        dec   x
110b: 10 f7     bpl   $1104
110d: cd 0e     mov   x,#$0e
110f: f5 f1 02  mov   a,$02f1+x
1112: f0 27     beq   $113b
1114: 1d        dec   x
1115: 1d        dec   x
1116: 10 f7     bpl   $110f
1118: cd 0e     mov   x,#$0e
111a: e8 ff     mov   a,#$ff
111c: d8 e5     mov   $e5,x
111e: 75 c1 02  cmp   a,$02c1+x
1121: 90 05     bcc   $1128
1123: f5 c1 02  mov   a,$02c1+x
1126: d8 e5     mov   $e5,x
1128: 1d        dec   x
1129: 1d        dec   x
112a: 10 f2     bpl   $111e
112c: f8 e5     mov   x,$e5
112e: c4 e5     mov   $e5,a
1130: f6 68 17  mov   a,$1768+y
1133: 28 7f     and   a,#$7f
1135: 64 e5     cmp   a,$e5
1137: b0 02     bcs   $113b
1139: 60        clrc
113a: 6f        ret

113b: dd        mov   a,y
113c: d5 01 01  mov   $0101+x,a
113f: f6 68 17  mov   a,$1768+y
1142: 28 7f     and   a,#$7f
1144: d5 c1 02  mov   $02c1+x,a
1147: 3f 45 06  call  $0645
114a: f6 ca 16  mov   a,$16ca+y
114d: d4 30     mov   $30+x,a
114f: f6 19 17  mov   a,$1719+y
1152: d4 31     mov   $31+x,a
1154: e4 e9     mov   a,$e9
1156: d5 66 03  mov   $0366+x,a
1159: 80        setc
115a: 6f        ret

115b: e4 f4     mov   a,$f4
115d: c4 f4     mov   $f4,a
115f: bc        inc   a
1160: c4 d1     mov   $d1,a
1162: 6f        ret

1163: e4 f4     mov   a,$f4
1165: 64 f4     cmp   a,$f4
1167: d0 fa     bne   $1163
1169: 64 d1     cmp   a,$d1
116b: d0 21     bne   $118e
116d: fd        mov   y,a
116e: e4 f6     mov   a,$f6
1170: 64 f6     cmp   a,$f6
1172: d0 fa     bne   $116e
1174: 68 0c     cmp   a,#$0c
1176: b0 e3     bcs   $115b
1178: 5d        mov   x,a
1179: f0 e0     beq   $115b
117b: f5 9a 11  mov   a,$119a+x
117e: 2d        push  a
117f: f5 8f 11  mov   a,$118f+x
1182: 2d        push  a
1183: e4 f7     mov   a,$f7
1185: 64 f7     cmp   a,$f7
1187: d0 fa     bne   $1183
1189: cb f4     mov   $f4,y
118b: fc        inc   y
118c: cb d1     mov   $d1,y
118e: 60        clrc
118f: 6f        ret

1190: db $aa,$c2,$cd,$e3,$e4,$e9,$f6,$f9,$fc,$ff,$a6
119b: db $11,$11,$11,$11,$11,$11,$11,$11,$11,$11,$11

11a6: c5 a0 0f  mov   $0fa0,a
11a9: 6f        ret

; start song (a - 1), save song index to $e4
11aa: 5d        mov   x,a
11ab: 1d        dec   x
11ac: d8 e4     mov   $e4,x
11ae: 3f 89 05  call  $0589
11b1: 8f 00 e2  mov   $e2,#$00
11b4: 8f 00 e3  mov   $e3,#$00
11b7: 8f 00 e0  mov   $e0,#$00
11ba: 8f 7f e1  mov   $e1,#$7f
11bd: 3f e8 10  call  $10e8
11c0: 80        setc
11c1: 6f        ret

11c2: 3f f7 10  call  $10f7
11c5: 8f 00 e0  mov   $e0,#$00
11c8: 8f 7f e1  mov   $e1,#$7f
11cb: 60        clrc
11cc: 6f        ret

11cd: 8d 00     mov   y,#$00
11cf: bc        inc   a
11d0: 9c        dec   a
11d1: 10 01     bpl   $11d4
11d3: dc        dec   y
11d4: cb e3     mov   $e3,y
11d6: 1c        asl   a
11d7: 2b e3     rol   $e3
11d9: 1c        asl   a
11da: 2b e3     rol   $e3
11dc: 1c        asl   a
11dd: 2b e3     rol   $e3
11df: c4 e2     mov   $e2,a
11e1: 60        clrc
11e2: 6f        ret

11e3: 6f        ret

11e4: 3f aa 11  call  $11aa
11e7: 60        clrc
11e8: 6f        ret

11e9: 3f 5d 0e  call  $0e5d
11ec: 48 ff     eor   a,#$ff
11ee: 36 3c 03  and   a,$033c+y
11f1: d6 3c 03  mov   $033c+y,a
11f4: 60        clrc
11f5: 6f        ret

11f6: c4 e8     mov   $e8,a
11f8: 6f        ret

11f9: c4 e9     mov   $e9,a
11fb: 6f        ret

11fc: c4 ea     mov   $ea,a
11fe: 6f        ret

11ff: c4 d0     mov   $d0,a
1201: 6f        ret

1202: 80
1203: 87 8f
1205: 98 a1 aa
1208: b5 bf cb
120b: d7 e4
120d: f1
120e: 00
120f: 0f
1210: 1f 30 42
1213: 55 6a 7f
1216: 96 ae c8
1219: e3 00 1e
121c: 3e 60
121e: 85 ab d4
1221: ff
1222: 2c 5d 90
1225: c6
1226: 00
1227: 3c
1228: 7d
1229: c1
122a: 0a 56 a8
122d: fe 59
122f: ba 20
1231: 8d 00
1233: 79
1234: fa 83 14
1237: ad 50
1239: fc
123a: b2 74
123c: 41
123d: 1a 00
123f: f3 f5 07
1242: 28 5b
1244: a0
1245: f9 65
1247: e8 82
1249: 34 00
124b: e7 eb
124d: 0e 51 b7
1250: 41
1251: f2 cb
1253: d1
1254: 04

1255: 68
1256: 00
1257: 00
1258: 00
1259: 00
125a: 00
125b: 00
125c: 00
125d: 00
125e: 00
125f: 00
1260: 00
1261: 00
1262: 00
1263: 01
1264: 01
1265: 01
1266: 01
1267: 01
1268: 01
1269: 01
126a: 01
126b: 01
126c: 01
126d: 01
126e: 01
126f: 02 02
1271: 02 02
1273: 02 02
1275: 02 02
1277: 03 03 03
127a: 03 04 04
127d: 04 04
127f: 05 05 05
1282: 05 06 06
1285: 07 07
1287: 08 08
1289: 08 09
128b: 0a 0a 0b
128e: 0b 0c
1290: 0d
1291: 0e 0f 10
1294: 10 11
1296: 13 14 15
1299: 16 17 19
129c: 1a 1c
129e: 1e 20 21
12a1: 23 26 28
12a4: 2a 2d 2f
12a7: 32 35
12a9: 39
12aa: 3c
12ab: 40

12ac: c4 f4     mov   $f4,a
12ae: d8 f5     mov   $f5,x
12b0: bc        inc   a
12b1: c4 f6     mov   $f6,a
12b3: 5f b0 12  jmp   $12b0

12b6: cd ef     mov   x,#$ef
12b8: bd        mov   sp,x
12b9: e8 00     mov   a,#$00
12bb: c6        mov   (x),a
12bc: 1d        dec   x
12bd: d0 fc     bne   $12bb
12bf: 8f 30 f1  mov   $f1,#$30
12c2: 8f aa f4  mov   $f4,#$aa
12c5: 8f bb f5  mov   $f5,#$bb
12c8: 78 cc f4  cmp   $f4,#$cc
12cb: d0 fb     bne   $12c8
12cd: 2f 19     bra   $12e8
12cf: eb f4     mov   y,$f4
12d1: d0 fc     bne   $12cf
12d3: 7e f4     cmp   y,$f4
12d5: d0 0b     bne   $12e2
12d7: e4 f5     mov   a,$f5
12d9: cb f4     mov   $f4,y
12db: d7 00     mov   ($00)+y,a
12dd: fc        inc   y
12de: d0 f3     bne   $12d3
12e0: ab 01     inc   $01
12e2: 10 ef     bpl   $12d3
12e4: 7e f4     cmp   y,$f4
12e6: 10 eb     bpl   $12d3
12e8: ba f6     movw  ya,$f6
12ea: da 00     movw  $00,ya
12ec: da f6     movw  $f6,ya
12ee: ba f4     movw  ya,$f4
12f0: da f4     movw  $f4,ya
12f2: dd        mov   a,y
12f3: 5d        mov   x,a
12f4: d0 d9     bne   $12cf
12f6: 5f 00 04  jmp   $0400





































12f9: 57 57     eor   a,($57)+y
12fb: 57 57     eor   a,($57)+y
12fd: 77 57     cmp   a,($57)+y
12ff: 57 5d     eor   a,($5d)+y
1301: 5d        mov   x,a
1302: 5d        mov   x,a
1303: 5d        mov   x,a
1304: 5d        mov   x,a
1305: 5d        mov   x,a
1306: 5d        mov   x,a
1307: 5d        mov   x,a
1308: 5d        mov   x,a
1309: 5d        mov   x,a
130a: 5d        mov   x,a
130b: 5d        mov   x,a
130c: 5d        mov   x,a
130d: 5d        mov   x,a
130e: 5d        mov   x,a
130f: 5d        mov   x,a
1310: 5d        mov   x,a
1311: 5d        mov   x,a
1312: 5d        mov   x,a
1313: 5d        mov   x,a
1314: 5d        mov   x,a
1315: 5d        mov   x,a
1316: 5d        mov   x,a
1317: 5d        mov   x,a
1318: 5d        mov   x,a
1319: 5d        mov   x,a
131a: 5d        mov   x,a
131b: 5d        mov   x,a
131c: 5d        mov   x,a
131d: 5d        mov   x,a
131e: 5d        mov   x,a
131f: 5d        mov   x,a
1320: 5d        mov   x,a
1321: 5d        mov   x,a
1322: 5d        mov   x,a
1323: 5d        mov   x,a
1324: 5d        mov   x,a
1325: 5d        mov   x,a
1326: 5d        mov   x,a
1327: 5d        mov   x,a
1328: 5d        mov   x,a
1329: 5d        mov   x,a
132a: 5d        mov   x,a
132b: 5d        mov   x,a
132c: 5d        mov   x,a
132d: 5d        mov   x,a
132e: 5d        mov   x,a
132f: 5d        mov   x,a
1330: 5d        mov   x,a
1331: 5d        mov   x,a
1332: 5d        mov   x,a
1333: 5d        mov   x,a
1334: 5d        mov   x,a
1335: 5d        mov   x,a
1336: 5d        mov   x,a
1337: 5d        mov   x,a
1338: 5d        mov   x,a
1339: 5d        mov   x,a
133a: 5d        mov   x,a
133b: 5d        mov   x,a
133c: 5d        mov   x,a
133d: 5d        mov   x,a
133e: 5d        mov   x,a
133f: 5d        mov   x,a
1340: 5d        mov   x,a
1341: 5d        mov   x,a
1342: 5d        mov   x,a
1343: 5d        mov   x,a
1344: 5d        mov   x,a
1345: 5d        mov   x,a
1346: 5d        mov   x,a
1347: 5d        mov   x,a
1348: 5d        mov   x,a
1349: 5d        mov   x,a
134a: 5d        mov   x,a
134b: 5d        mov   x,a
134c: 5d        mov   x,a
134d: 5d        mov   x,a
134e: 5d        mov   x,a
134f: 5d        mov   x,a
1350: 5d        mov   x,a
1351: 5d        mov   x,a
1352: 5d        mov   x,a
1353: 5d        mov   x,a
1354: 5d        mov   x,a
1355: 5d        mov   x,a
1356: 5d        mov   x,a
1357: 5d        mov   x,a
1358: 5d        mov   x,a
1359: 5d        mov   x,a
135a: 5d        mov   x,a
135b: 5d        mov   x,a
135c: 5d        mov   x,a
135d: 5d        mov   x,a
135e: 5d        mov   x,a
135f: 5d        mov   x,a
1360: 5d        mov   x,a
1361: 5d        mov   x,a
1362: 5d        mov   x,a
1363: 5d        mov   x,a
1364: 5d        mov   x,a
1365: 5d        mov   x,a
1366: 5d        mov   x,a
1367: 5d        mov   x,a
1368: 5d        mov   x,a
1369: 5d        mov   x,a
136a: 5d        mov   x,a
136b: 5d        mov   x,a
136c: 5d        mov   x,a
136d: 5d        mov   x,a
136e: 5d        mov   x,a
136f: 5d        mov   x,a
1370: 5d        mov   x,a
1371: 5d        mov   x,a
1372: 5d        mov   x,a
1373: 5d        mov   x,a
1374: 5d        mov   x,a
1375: 5d        mov   x,a
1376: 5d        mov   x,a
1377: 5d        mov   x,a
1378: 5d        mov   x,a
1379: 5d        mov   x,a
137a: 5d        mov   x,a
137b: 5d        mov   x,a
137c: 5d        mov   x,a
137d: 5d        mov   x,a
137e: 5d        mov   x,a
137f: 5d        mov   x,a
1380: fe 00     dbnz  y,$1382
1382: b8 93 0d  sbc   $0d,#$93
1385: 14 23     or    a,$23+x
1387: 15 15 15  or    a,$1515+x
138a: fe 03     dbnz  y,$138f
138c: f2 b7     clr7  $b7
138e: 26        and   a,(x)
138f: 14 23     or    a,$23+x
1391: 15 15 15  or    a,$1515+x
1394: fe 06     dbnz  y,$139c
1396: 1a b7     decw  $b7
1398: 3d        inc   x
1399: 14 23     or    a,$23+x
139b: 16 15 15  or    a,$1515+y
139e: fe 09     dbnz  y,$13a9
13a0: 5e b7 52  cmp   y,$52b7
13a3: 14 23     or    a,$23+x
13a5: 16 15 15  or    a,$1515+y
13a8: fe 0c     dbnz  y,$13b6
13aa: 80        setc
13ab: b7 68     sbc   a,($68)+y
13ad: 14 23     or    a,$23+x
13af: 16 15 15  or    a,$1515+y
13b2: fe 0f     dbnz  y,$13c3
13b4: c7 b7     mov   ($b7+x),a
13b6: 7d        mov   a,x
13b7: 14 23     or    a,$23+x
13b9: 16 15 15  or    a,$1515+y
13bc: fe 12     dbnz  y,$13d0
13be: c8 b7     cmp   x,#$b7
13c0: 92 14     clr4  $14
13c2: 23 16 15  bbs1  $16,$13da
13c5: 15 fe 15  or    a,$15fe+x
13c8: c9 b7 92  mov   $92b7,x
13cb: 14 23     or    a,$23+x
13cd: 16 15 15  or    a,$1515+y
13d0: 18 db f7  or    $f7,#$db
13d3: ff        stop
13d4: 23 14 14  bbs1  $14,$13eb
13d7: 14 01     or    a,$01+x
13d9: 00        nop
13da: ff        stop
13db: 00        nop
13dc: 01        tcall 0
13dd: 00        nop
13de: 01        tcall 0
13df: 01        tcall 0
13e0: 78 01 78  cmp   $78,#$01
13e3: 01        tcall 0
13e4: 78 10 01  cmp   $01,#$10
13e7: 3c        rol   a
13e8: 01        tcall 0
13e9: 3c        rol   a
13ea: 01        tcall 0
13eb: 3c        rol   a
13ec: 10 01     bpl   $13ef
13ee: 00        nop
13ef: 08 1e     or    a,#$1e
13f1: 08 00     or    a,#$00
13f3: 10 80     bpl   $1375
13f5: b6 c8 b3  sbc   a,$b3c8+y
13f8: 70 4d     bvs   $1447
13fa: 89 0a 97  adc   ($97),($0a)
13fd: 01        tcall 0
13fe: 8e        pop   psw
13ff: 0c 05 00  asl   $0005
1402: 93 00 18  bbc4  $00,$141d
1405: 24 9c     and   a,$9c
1407: 9f        xcn   a
1408: 8e        pop   psw
1409: 0a 03 0d  or1   c,$0d03,0
140c: 93 09 82  bbc4  $09,$1391
140f: 36 25 92  and   a,$9225+y
1412: 0a 84 02  or1   c,$0284,0
1415: 1d        dec   x
1416: 24 18     and   a,$18
1418: 24 1f     and   a,$1f
141a: 18 1a 0c  or    $0c,#$1a
141d: 18 18 1a  or    $1a,#$18
1420: 0c 1d 18  asl   $181d
1423: 1f 0c 20  jmp   ($200c+x)

1426: 0c 1f 0c  asl   $0c1f
1429: 1d        dec   x
142a: 0c 1f 18  asl   $181f
142d: 1a 0c     decw  $0c
142f: 1f 18 24  jmp   ($2418+x)

1432: 0c 85 88  asl   $8885
1435: e8 92     mov   a,#$92
1437: 06        or    a,(x)
1438: 82 17     set4  $17
143a: 25 38 0c  and   a,$0c38
143d: 37 0c     and   a,($0c)+y
143f: 30 0c     bmi   $144d
1441: 82 17     set4  $17
1443: 25 88 00  and   a,$0088
1446: 81        tcall 8
1447: 33 23 80  bbc1  $23,$13ca
144a: b3 62 b1  bbc5  $62,$13fe
144d: 8c 01 ea  dec   $ea01
1450: 25 89 09  and   a,$0989
1453: 9d        mov   x,sp
1454: 37 0c     and   a,($0c)+y
1456: 37 0c     and   a,($0c)+y
1458: 97 01     adc   a,($01)+y
145a: 93 06 37  bbc4  $06,$1494
145d: 0c 93 00  asl   $0093
1460: 84 10     adc   a,$10
1462: 82 9b     set4  $9b
1464: 25 85 8c  and   a,$8c85
1467: 01        tcall 0
1468: c3 25 89  bbs6  $25,$13f4
146b: 01        tcall 0
146c: 84 02     adc   a,$02
146e: 84 02     adc   a,$02
1470: 97 01     adc   a,($01)+y
1472: 9b 37     dec   $37+x
1474: 0c 37 0c  asl   $0c37
1477: 37 0c     and   a,($0c)+y
1479: 9d        mov   x,sp
147a: 84 06     adc   a,$06
147c: 37 03     and   a,($03)+y
147e: 9c        dec   a
147f: 37 03     and   a,($03)+y
1481: 85 85 97  adc   a,$9785
1484: 01        tcall 0
1485: 9b 37     dec   $37+x
1487: 0c 37 0c  asl   $0c37
148a: 37 0c     and   a,($0c)+y
148c: 37 18     and   a,($18)+y
148e: 37 0c     and   a,($0c)+y
1490: 37 18     and   a,($18)+y
1492: 37 0c     and   a,($0c)+y
1494: 37 18     and   a,($18)+y
1496: 37 06     and   a,($06)+y
1498: 37 06     and   a,($06)+y
149a: 85 8c 01  adc   a,$018c
149d: ea 25 81  not1  $0125,4
14a0: 99        adc   (x),(y)
14a1: 23 80 00  bbs1  $80,$14a4
14a4: 24 97     and   a,$97
14a6: 03 86 0c  bbs0  $86,$14b5
14a9: b3 54 59  bbc5  $54,$1505
14ac: 84 04     adc   a,$04
14ae: 89 1a 84  adc   ($84),($1a)
14b1: 06        or    a,(x)
14b2: 35 85 89  and   a,$8985+x
14b5: 1b 84     asl   $84+x
14b7: 06        or    a,(x)
14b8: 37 85     and   a,($85)+y
14ba: 85 b3 38  adc   a,$38b3
14bd: 59        eor   (x),(y)
14be: 97 01     adc   a,($01)+y
14c0: 86        adc   a,(x)
14c1: 00        nop
14c2: 84 04     adc   a,$04
14c4: 84 02     adc   a,$02
14c6: 89 06 04  adc   ($04),($06)
14c9: 18 89 1e  or    $1e,#$89
14cc: 29 0c 85  and   ($85),($0c)
14cf: 84 02     adc   a,$02
14d1: 89 06 04  adc   ($04),($06)
14d4: 18 89 05  or    $05,#$89
14d7: 2b 0c     rol   $0c
14d9: 85 85 01  adc   a,$0185
14dc: 78 01 78  cmp   $78,#$01
14df: 01        tcall 0
14e0: 78 06 01  cmp   $01,#$06
14e3: 3c        rol   a
14e4: 01        tcall 0
14e5: 3c        rol   a
14e6: 01        tcall 0
14e7: 3c        rol   a
14e8: 0a 01 00  or1   c,$0001,0
14eb: 05 3c 01  or    a,$013c
14ee: 3c        rol   a
14ef: 0a 01 0a  or1   c,$0a01,0
14f2: 28 78     and   a,#$78
14f4: 01        tcall 0
14f5: 78 0a 01  cmp   $01,#$0a
14f8: 78 01 78  cmp   $78,#$01
14fb: 01        tcall 0
14fc: 78 0a 80  cmp   $80,#$0a
14ff: 01        tcall 0
1500: 78 01 78  cmp   $78,#$01
1503: 01        tcall 0
1504: 78 5a 01  cmp   $01,#$5a
1507: 78 01 78  cmp   $78,#$01
150a: 01        tcall 0
150b: 78 64 b6  cmp   $b6,#$64
150e: c5 b3 46  mov   $46b3,a
1511: c8 89     cmp   x,#$89
1513: 18 97 01  or    $01,#$97
1516: 2a 01 94  or1   c,!($1401,4)
1519: 01        tcall 0
151a: 02 9f     set0  $9f
151c: 1e 40 92  cmp   x,$9240
151f: 01        tcall 0
1520: 84 20     adc   a,$20
1522: 00        nop
1523: 10 85     bpl   $14aa
1525: 80        setc
1526: b3 46 32  bbc5  $46,$155b
1529: 89 26 97  adc   ($97),($26)
152c: 01        tcall 0
152d: 1e 01 94  cmp   x,$9401
1530: 01        tcall 0
1531: 01        tcall 0
1532: 9f        xcn   a
1533: 36 40 92  and   a,$9240+y
1536: 01        tcall 0
1537: 84 20     adc   a,$20
1539: 00        nop
153a: 10 85     bpl   $14c1
153c: 80        setc
153d: b3 5a 7f  bbc5  $5a,$15bf
1540: 89 09 97  adc   ($97),($09)
1543: 01        tcall 0
1544: 1e 01 90  cmp   x,$9001
1547: 0a 3c 40  or1   c,$003c,2
154a: 92 01     clr4  $01
154c: 84 20     adc   a,$20
154e: 00        nop
154f: 10 85     bpl   $14d6
1551: 80        setc
1552: b3 78 dc  bbc5  $78,$1531
1555: 89 14 b4  adc   ($b4),($14)
1558: 05 97 02  or    a,$0297
155b: 14 01     or    a,$01+x
155d: 92 05     clr4  $05
155f: 90 0a     bcc   $156b
1561: 84 3c     adc   a,$3c
1563: 46        eor   a,(x)
1564: 07 9c     or    a,($9c+x)
1566: 85 80 b3  adc   a,$b380
1569: 50 1e     bvc   $1589
156b: 89 26 97  adc   ($97),($26)
156e: 01        tcall 0
156f: 36 08 42  and   a,$4208+y
1572: 08 3d     or    a,#$3d
1574: 36 92 01  and   a,$0192+y
1577: 84 20     adc   a,$20
1579: 00        nop
157a: 10 85     bpl   $1501
157c: 80        setc
157d: b3 3c d8  bbc5  $3c,$1558
1580: 89 26 97  adc   ($97),($26)
1583: 01        tcall 0
1584: 00        nop
1585: 1c        asl   a
1586: 92 01     clr4  $01
1588: 84 20     adc   a,$20
158a: 36 08 9c  and   a,$9c08+y
158d: 42 08     set2  $08
158f: 3d        inc   x
1590: 0c 85 80  asl   $8085
1593: b6 be b3  sbc   a,$b3be+y
1596: 78 7f 97  cmp   $97,#$7f
1599: 01        tcall 0
159a: 89 24 8e  adc   ($8e),($24)
159d: 01        tcall 0
159e: 14 02     or    a,$02+x
15a0: 1b 01     asl   $01+x
15a2: 90 28     bcc   $15cc
15a4: 50 0d     bvc   $15b3
15a6: 8f 01 32  mov   $32,#$01
15a9: 04 9f     or    a,$9f
15ab: 50 0d     bvc   $15ba
15ad: 91        tcall 9
15ae: 90 fa     bcc   $15aa
15b0: 50 05     bvc   $15b7
15b2: 97 00     adc   a,($00)+y
15b4: 46        eor   a,(x)
15b5: 04 80     or    a,$80
15b7: 80        setc
15b8: b6 c8 b3  sbc   a,$b3c8+y
15bb: 32 4d     clr1  $4d
15bd: 89 07 93  adc   ($93),($07)
15c0: 03 97 03  bbs0  $97,$15c6
15c3: 8e        pop   psw
15c4: 0d        push  psw
15c5: 04 0a     or    a,$0a
15c7: 32 0a     clr1  $0a
15c9: 32 0a     clr1  $0a
15cb: 34 0a     and   a,$0a+x
15cd: 36 0a 38  and   a,$380a+y
15d0: 01        tcall 0
15d1: 9c        dec   a
15d2: 90 18     bcc   $15ec
15d4: 39        and   (x),(y)
15d5: 1d        dec   x
15d6: 9b 90     dec   $90+x
15d8: 00        nop
15d9: 36 1e 37  and   a,$371e+y
15dc: 1e 34 1e  cmp   x,$1e34
15df: 35 01 9c  and   a,$9c01+x
15e2: 90 1e     bcc   $1602
15e4: 36 13 9b  and   a,$9b13+y
15e7: 90 00     bcc   $15e9
15e9: 37 0a     and   a,($0a)+y
15eb: 39        and   (x),(y)
15ec: 14 3e     or    a,$3e+x
15ee: 28 32     and   a,#$32
15f0: 14 80     or    a,$80+x
15f2: b3 3c b4  bbc5  $3c,$15a9
15f5: 89 1c 93  adc   ($93),($1c)
15f8: 04 97     or    a,$97
15fa: 02 8e     set0  $8e
15fc: 0f        brk
15fd: 02 0b     set0  $0b
15ff: 21        tcall 2
1600: 0a 1f 0a  or1   c,$0a1f,0
1603: 1e 0a 1c  cmp   x,$1c0a
1606: 0a 1a 1e  or1   c,$1e1a,0
1609: 1e 1e 1c  cmp   x,$1c1e
160c: 1e 15 1e  cmp   x,$1e15
160f: 1a 14     decw  $14
1611: 1c        asl   a
1612: 0a 1e 1e  or1   c,$1e1e,0
1615: 1a 1e     decw  $1e
1617: 0e 14 80  tset1 $8014
161a: b3 3c 5a  bbc5  $3c,$1677
161d: 97 02     adc   a,($02)+y
161f: 00        nop
1620: 28 89     and   a,#$89
1622: 04 26     or    a,$26
1624: 14 89     or    a,$89+x
1626: 05 26 0a  or    a,$0a26
1629: 92 02     clr4  $02
162b: 26        and   a,(x)
162c: 14 92     or    a,$92+x
162e: 00        nop
162f: 26        and   a,(x)
1630: 0a 89 04  or1   c,$0489,0
1633: 2d        push  a
1634: 14 89     or    a,$89+x
1636: 05 2d 0a  or    a,$0a2d
1639: 92 02     clr4  $02
163b: 28 14     and   a,#$14
163d: 92 00     clr4  $00
163f: 2d        push  a
1640: 0a 89 04  or1   c,$0489,0
1643: 26        and   a,(x)
1644: 14 89     or    a,$89+x
1646: 05 26 0a  or    a,$0a26
1649: 89 04 26  adc   ($26),($04)
164c: 0a 89 05  or1   c,$0589,0
164f: 26        and   a,(x)
1650: 0a 89 04  or1   c,$0489,0
1653: 26        and   a,(x)
1654: 0a 89 05  or1   c,$0589,0
1657: 26        and   a,(x)
1658: 1e 89 04  cmp   x,$0489
165b: 26        and   a,(x)
165c: 14 80     or    a,$80+x
165e: b3 1e b4  bbc5  $1e,$1615
1661: 89 0d 93  adc   ($93),($0d)
1664: 04 97     or    a,$97
1666: 02 8e     set0  $8e
1668: 0f        brk
1669: 02 0b     set0  $0b
166b: 00        nop
166c: 28 2a     and   a,#$2a
166e: 1e 26 1e  cmp   x,$1e26
1671: 28 1e     and   a,#$1e
1673: 2d        push  a
1674: 1e 26 14  cmp   x,$1426
1677: 28 0a     and   a,#$0a
1679: 2a 14 26  or1   c,!($0614,1)
167c: 28 26     and   a,#$26
167e: 14 80     or    a,$80+x
1680: b3 32 78  bbc5  $32,$16fb
1683: 89 20 97  adc   ($97),($20)
1686: 04 84     or    a,$84
1688: 08 37     or    a,#$37
168a: 05 9c 85  or    a,$859c
168d: 9b 93     dec   $93+x
168f: 05 84 02  or    a,$0284
1692: 89 02 97  adc   ($97),($02)
1695: 02 2d     set0  $2d
1697: 14 2d     or    a,$2d+x
1699: 0a 97 01  or1   c,$0197,0
169c: 89 20 37  adc   ($37),($20)
169f: 14 37     or    a,$37+x
16a1: 0a 85 89  or1   c,$0985,4
16a4: 02 97     set0  $97
16a6: 02 2d     set0  $2d
16a8: 14 2d     or    a,$2d+x
16aa: 0a 97 01  or1   c,$0197,0
16ad: 89 20 37  adc   ($37),($20)
16b0: 14 37     or    a,$37+x
16b2: 0a 97 04  or1   c,$0497,0
16b5: 93 00 84  bbc4  $00,$163c
16b8: 06        or    a,(x)
16b9: 37 05     and   a,($05)+y
16bb: 9c        dec   a
16bc: 85 9b 93  adc   a,$939b
16bf: 05 89 02  or    a,$0289
16c2: 97 02     adc   a,($02)+y
16c4: 2d        push  a
16c5: 14 80     or    a,$80+x
16c7: 80        setc
16c8: 80        setc
16c9: 80        setc
16ca: b7 d3     sbc   a,($d3)+y
16cc: e3 f3 03  bbs7  $f3,$16d2
16cf: 19        or    (x),(y)
16d0: 29 3d 75  and   ($75),($3d)
16d3: 92 b9     clr4  $b9
16d5: d1        tcall 13
16d6: fb 17     mov   y,$17+x
16d8: 32 4b     clr1  $4b
16da: 72 b9     clr3  $b9
16dc: e7 11     mov   a,($11+x)
16de: 39        and   (x),(y)
16df: 6b 87     ror   $87
16e1: b5 dd 00  sbc   a,$00dd+x
16e4: 23 46 8b  bbs1  $46,$1672
16e7: e1        tcall 14
16e8: 0f        brk
16e9: 3c        rol   a
16ea: 43 8a 9e  bbs2  $8a,$168b
16ed: ae        pop   a
16ee: c6        mov   (x),a
16ef: e2 0a     set7  $0a
16f1: 0d        push  psw
16f2: 25 58 5b  and   a,$5b58
16f5: 91        tcall 9
16f6: af        mov   (x)+,a
16f7: cd ef     mov   x,#$ef
16f9: 0f        brk
16fa: 37 3a     and   a,($3a)+y
16fc: 5e 61 c6  cmp   y,$c661
16ff: e4 06     mov   a,$06
1701: 2a 45 5b  or1   c,!($1b45,2)
1704: 76 8a a4  cmp   a,$a48a+y
1707: d7 02     mov   ($02)+y,a
1709: 20        clrp
170a: 46        eor   a,(x)
170b: 68 9a     cmp   a,#$9a
170d: b4 ce     sbc   a,$ce+x
170f: ff        stop
1710: 26        and   a,(x)
1711: 7d        mov   a,x
1712: 9f        xcn   a
1713: c6        mov   (x),a
1714: e3 fa 20  bbs7  $fa,$1737
1717: 23 5e 17  bbs1  $5e,$1731
171a: 17 17     or    a,($17)+y
171c: 17 18     or    a,($18)+y
171e: 18 18 18  or    $18,#$18
1721: 18 18 18  or    $18,#$18
1724: 18 18 19  or    $19,#$18
1727: 19        or    (x),(y)
1728: 19        or    (x),(y)
1729: 19        or    (x),(y)
172a: 19        or    (x),(y)
172b: 19        or    (x),(y)
172c: 1a 1a     decw  $1a
172e: 1a 1a     decw  $1a
1730: 1a 1a     decw  $1a
1732: 1b 1b     asl   $1b+x
1734: 1b 1b     asl   $1b+x
1736: 1b 1c     asl   $1c+x
1738: 1c        asl   a
1739: 1c        asl   a
173a: 1c        asl   a
173b: 1c        asl   a
173c: 1c        asl   a
173d: 1c        asl   a
173e: 1c        asl   a
173f: 1d        dec   x
1740: 1d        dec   x
1741: 1d        dec   x
1742: 1d        dec   x
1743: 1d        dec   x
1744: 1d        dec   x
1745: 1d        dec   x
1746: 1d        dec   x
1747: 1d        dec   x
1748: 1e 1e 1e  cmp   x,$1e1e
174b: 1e 1e 1e  cmp   x,$1e1e
174e: 1e 1f 1f  cmp   x,$1f1f
1751: 1f 1f 1f  jmp   ($1f1f+x)

1754: 1f 1f 1f  jmp   ($1f1f+x)

1757: 20        clrp
1758: 20        clrp
1759: 20        clrp
175a: 20        clrp
175b: 20        clrp
175c: 20        clrp
175d: 20        clrp
175e: 20        clrp
175f: 21        tcall 2
1760: 21        tcall 2
1761: 21        tcall 2
1762: 21        tcall 2
1763: 21        tcall 2
1764: 21        tcall 2
1765: 22 22     set1  $22
1767: 22 05     set1  $05
1769: 06        or    a,(x)
176a: 05 03 05  or    a,$0503
176d: 06        or    a,(x)
176e: 04 06     or    a,$06
1770: 05 05 07  or    a,$0705
1773: 05 05 05  or    a,$0505
1776: 05 05 03  or    a,$0305
1779: 09 09 09  or    ($09),($09)
177c: 05 06 05  or    a,$0506
177f: 05 05 05  or    a,$0505
1782: 05 07 0a  or    a,$0a07
1785: 05 05 09  or    a,$0905
1788: 05 05 05  or    a,$0505
178b: 05 05 0a  or    a,$0a05
178e: 0a 05 05  or1   c,$0505,0
1791: 09 08 08  or    ($08),($08)
1794: 08 08     or    a,#$08
1796: 08 08     or    a,#$08
1798: 09 08 09  or    ($09),($08)
179b: 08 09     or    a,#$09
179d: 09 07 07  or    ($07),($07)
17a0: 07 07     or    a,($07+x)
17a2: 0b 07     asl   $07
17a4: 05 04 05  or    a,$0504
17a7: 05 05 07  or    a,$0705
17aa: 05 05 06  or    a,$0605
17ad: 05 07 06  or    a,$0607
17b0: 06        or    a,(x)
17b1: 09 08 06  or    ($06),($08)
17b4: 06        or    a,(x)
17b5: 06        or    a,(x)
17b6: 05 b3 3c  or    a,$3cb3
17b9: 7f        reti
17ba: a2 01     set5  $01
17bc: 78 01 78  cmp   $78,#$01
17bf: 14 1e     or    a,$1e+x
17c1: 0f        brk
17c2: 89 18 8e  adc   ($8e),($18)
17c5: 01        tcall 0
17c6: 8c 05 35  dec   $3505
17c9: 01        tcall 0
17ca: 90 3c     bcc   $1808
17cc: 93 10 9f  bbc4  $10,$176e
17cf: 9c        dec   a
17d0: 50 24     bvc   $17f6
17d2: 80        setc
17d3: b3 7d 7f  bbc5  $7d,$1855
17d6: a2 01     set5  $01
17d8: 7e 01     cmp   y,$01
17da: 7e 01     cmp   y,$01
17dc: 7e 05     cmp   y,$05
17de: 89 17 2b  adc   ($2b),($17)
17e1: 1e 80 b3  cmp   x,$b380
17e4: 7d        mov   a,x
17e5: 7f        reti
17e6: a2 01     set5  $01
17e8: 7e 01     cmp   y,$01
17ea: 7e 01     cmp   y,$01
17ec: 7e 05     cmp   y,$05
17ee: 89 12 33  adc   ($33),($12)
17f1: 19        or    (x),(y)
17f2: 80        setc
17f3: b3 78 7f  bbc5  $78,$1875
17f6: a2 01     set5  $01
17f8: 7e 01     cmp   y,$01
17fa: 7e 01     cmp   y,$01
17fc: 7e 05     cmp   y,$05
17fe: 89 13 2f  adc   ($2f),($13)
1801: 1e 80 b3  cmp   x,$b380
1804: 64 7f     cmp   a,$7f
1806: a2 01     set5  $01
1808: 7e 01     cmp   y,$01
180a: 7e 28     cmp   y,$28
180c: 00        nop
180d: 05 89 14  or    a,$1489
1810: 86        adc   a,(x)
1811: 08 26     or    a,#$26
1813: 9c        dec   a
1814: 24 22     and   a,$22
1816: 20        clrp
1817: 1e 80 b3  cmp   x,$b380
181a: 50 7f     bvc   $189b
181c: a2 01     set5  $01
181e: 7e 01     cmp   y,$01
1820: 7e 01     cmp   y,$01
1822: 7e 05     cmp   y,$05
1824: 89 14 32  adc   ($32),($14)
1827: 10 80     bpl   $17a9
1829: b3 78 7f  bbc5  $78,$18ab
182c: a2 01     set5  $01
182e: 7e 01     cmp   y,$01
1830: 7e 01     cmp   y,$01
1832: 7e 05     cmp   y,$05
1834: 89 15 1a  adc   ($1a),($15)
1837: 0c 1d 09  asl   $091d
183a: 21        tcall 2
183b: 07 80     or    a,($80+x)
183d: b3 46 7f  bbc5  $46,$18bf
1840: 98 a2 01  adc   $01,#$a2
1843: 14 28     or    a,$28+x
1845: 32 3c     clr1  $3c
1847: 00        nop
1848: 05 89 0b  or    a,$0b89
184b: 86        adc   a,(x)
184c: 02 9a     set0  $9a
184e: 1f 03 9f  jmp   ($9f03+x)

1851: 9c        dec   a
1852: 9a 1e     subw  ya,$1e
1854: 03 9a 1d  bbs0  $9a,$1874
1857: 03 9a 1c  bbs0  $9a,$1876
185a: 03 9a 1b  bbs0  $9a,$1878
185d: 03 86 05  bbs0  $86,$1865
1860: 9a 1a     subw  ya,$1a
1862: 03 9a 1b  bbs0  $9a,$1880
1865: 03 9a 1c  bbs0  $9a,$1884
1868: 03 9a 1d  bbs0  $9a,$1888
186b: 03 9a 1e  bbs0  $9a,$188c
186e: 03 86 00  bbs0  $86,$1871
1871: 03 3d 99  bbs0  $3d,$180d
1874: 80        setc
1875: b3 78 7f  bbc5  $78,$18f7
1878: a2 01     set5  $01
187a: 50 01     bvc   $187d
187c: 50 50     bvc   $18ce
187e: 00        nop
187f: 05 89 19  or    a,$1989
1882: 88 00     adc   a,#$00
1884: 0c 1e 9c  asl   $9c1e
1887: 0b 14     asl   $14
1889: 0d        push  psw
188a: 28 0a     and   a,#$0a
188c: 32 0c     clr1  $0c
188e: 0a 0d 1e  or1   c,$1e0d,0
1891: 80        setc
1892: b3 78 7f  bbc5  $78,$1914
1895: 99        adc   (x),(y)
1896: 89 01 a2  adc   ($a2),($01)
1899: 01        tcall 0
189a: 0a 0a 78  or1   c,$180a,3
189d: 05 00 01  or    a,$0100
18a0: 36 02 9c  and   a,$9c02+y
18a3: 89 25 a3  adc   ($a3),($25)
18a6: 02 ab     set0  $ab
18a8: 18 b2 18  or    $18,#$b2
18ab: 8f 08 c8  mov   $c8,#$08
18ae: 00        nop
18af: 30 0d     bmi   $18be
18b1: 80        setc
18b2: 8f 01 3c  mov   $3c,#$01
18b5: 00        nop
18b6: 31        tcall 3
18b7: 0d        push  psw
18b8: 80        setc
18b9: b3 7d 7f  bbc5  $7d,$193b
18bc: a2 01     set5  $01
18be: 7e 01     cmp   y,$01
18c0: 7e 64     cmp   y,$64
18c2: 00        nop
18c3: 05 89 17  or    a,$1789
18c6: 2b 0d     rol   $0d
18c8: 9c        dec   a
18c9: 90 02     bcc   $18cd
18cb: 84 07     adc   a,$07
18cd: 09 0d 85  or    ($85),($0d)
18d0: 80        setc
18d1: b3 14 7f  bbc5  $14,$1953
18d4: 98 a2 01  adc   $01,#$a2
18d7: 7e 01     cmp   y,$01
18d9: 7e 82     cmp   y,$82
18db: 00        nop
18dc: 05 89 0b  or    a,$0b89
18df: 86        adc   a,(x)
18e0: 02 84     set0  $84
18e2: 0d        push  psw
18e3: 9a 1f     subw  ya,$1f
18e5: 05 9f 9c  or    a,$9c9f
18e8: 9a 1d     subw  ya,$1d
18ea: 05 9a 1b  or    a,$1b9a
18ed: 05 9a 19  or    a,$199a
18f0: 05 9a 17  or    a,$179a
18f3: 05 85 97  or    a,$9785
18f6: 00        nop
18f7: 99        adc   (x),(y)
18f8: 01        tcall 0
18f9: 01        tcall 0
18fa: 80        setc
18fb: b3 78 7f  bbc5  $78,$197d
18fe: a2 01     set5  $01
1900: 00        nop
1901: 07 78     or    a,($78+x)
1903: 01        tcall 0
1904: 78 07 89  cmp   $89,#$07
1907: 25 1a 01  and   a,$011a
190a: 9c        dec   a
190b: 9f        xcn   a
190c: 90 3c     bcc   $194a
190e: 0a 05 90  or1   c,$1005,4
1911: 1e 93 08  cmp   x,$0893
1914: 28 14     and   a,#$14
1916: 80        setc
1917: b3 78 7f  bbc5  $78,$1999
191a: 99        adc   (x),(y)
191b: 89 02 a2  adc   ($a2),($02)
191e: 01        tcall 0
191f: 14 0a     or    a,$0a+x
1921: 78 01 78  cmp   $78,#$01
1924: 05 31 02  or    a,$0231
1927: 89 25 93  adc   ($93),($25)
192a: 06        or    a,(x)
192b: 8f 01 0f  mov   $0f,#$01
192e: 00        nop
192f: 1f 10 80  jmp   ($8010+x)

1932: b3 78 7f  bbc5  $78,$19b4
1935: 99        adc   (x),(y)
1936: 89 01 a2  adc   ($a2),($01)
1939: 01        tcall 0
193a: 14 0a     or    a,$0a+x
193c: 78 14 00  cmp   $00,#$14
193f: 01        tcall 0
1940: 36 02 89  and   a,$8902+y
1943: 25 8f 09  and   a,$098f
1946: 29 00 30  and   ($30),($00)
1949: 28 80     and   a,#$80
194b: b3 78 7f  bbc5  $78,$19cd
194e: 99        adc   (x),(y)
194f: a2 01     set5  $01
1951: 14 0a     or    a,$0a+x
1953: 78 01 78  cmp   $78,#$01
1956: 19        or    (x),(y)
1957: 89 02 31  adc   ($31),($02)
195a: 02 89     set0  $89
195c: 25 8f 01  and   a,$018f
195f: 03 00 1f  bbs0  $00,$1981
1962: 01        tcall 0
1963: 9c        dec   a
1964: 90 0f     bcc   $1975
1966: 92 19     clr4  $19
1968: 9f        xcn   a
1969: 21        tcall 2
196a: 1e 8f 01  cmp   x,$018f
196d: 0c 00 00  asl   $0000
1970: 10 80     bpl   $18f2
1972: b3 37 7f  bbc5  $37,$19f4
1975: a2 01     set5  $01
1977: 78 01 78  cmp   $78,#$01
197a: 0a 28 0f  or1   c,$0f28,0
197d: 89 23 a3  adc   ($a3),($23)
1980: 03 87 19  bbs0  $87,$199c
1983: 9b 19     dec   $19+x
1985: aa 19 3b  mov1  c,$1b19,1
1988: 01        tcall 0
1989: 90 1e     bcc   $19a9
198b: 8e        pop   psw
198c: 01        tcall 0
198d: 0a 00 9f  or1   c,$1f00,4
1990: 3a 0a     incw  $0a
1992: 92 01     clr4  $01
1994: 8e        pop   psw
1995: 01        tcall 0
1996: 28 00     and   a,#$00
1998: 00        nop
1999: 0f        brk
199a: 80        setc
199b: 8e        pop   psw
199c: 01        tcall 0
199d: 0a 00 3a  or1   c,$1a00,1
19a0: 0a 92 01  or1   c,$0192,0
19a3: 8e        pop   psw
19a4: 01        tcall 0
19a5: 3c        rol   a
19a6: 00        nop
19a7: 00        nop
19a8: 0f        brk
19a9: 80        setc
19aa: 8e        pop   psw
19ab: 01        tcall 0
19ac: 1e 00 39  cmp   x,$3900
19af: 0a 92 01  or1   c,$0192,0
19b2: 8e        pop   psw
19b3: 01        tcall 0
19b4: 14 00     or    a,$00+x
19b6: 00        nop
19b7: 0f        brk
19b8: 80        setc
19b9: b5 12 b5  sbc   a,$b512+x
19bc: 13 b3 50  bbc0  $b3,$1a0f
19bf: 7f        reti
19c0: 89 01 a2  adc   ($a2),($01)
19c3: 01        tcall 0
19c4: 64 01     cmp   a,$01
19c6: 64 01     cmp   a,$01
19c8: 64 28     cmp   a,$28
19ca: 89 25 86  adc   ($86),($25)
19cd: 08 49     or    a,#$49
19cf: 46        eor   a,(x)
19d0: 43 40 3d  bbs2  $40,$1a10
19d3: 3a 37     incw  $37
19d5: 34 31     and   a,$31+x
19d7: 2e 2b 28  cbne  $2b,$1a02
19da: 89 03 86  adc   ($86),($03)
19dd: 00        nop
19de: 90 0d     bcc   $19ed
19e0: 29 01 93  and   ($93),($01)
19e3: 27 1a     and   a,($1a+x)
19e5: 50 80     bvc   $1967
19e7: b3 50 7f  bbc5  $50,$1a69
19ea: 89 01 a2  adc   ($a2),($01)
19ed: 01        tcall 0
19ee: 64 01     cmp   a,$01
19f0: 64 01     cmp   a,$01
19f2: 64 28     cmp   a,$28
19f4: 89 09 86  adc   ($86),($09)
19f7: 08 49     or    a,#$49
19f9: 46        eor   a,(x)
19fa: 43 40 3d  bbs2  $40,$1a3a
19fd: 3a 37     incw  $37
19ff: 34 31     and   a,$31+x
1a01: 2e 2b 28  cbne  $2b,$1a2c
1a04: 89 1b 86  adc   ($86),($1b)
1a07: 00        nop
1a08: 90 37     bcc   $1a41
1a0a: 29 04 93  and   ($93),($04)
1a0d: 27 4a     and   a,($4a+x)
1a0f: 50 80     bvc   $1991
1a11: b3 78 7f  bbc5  $78,$1a93
1a14: 89 01 a2  adc   ($a2),($01)
1a17: 01        tcall 0
1a18: 6e 01 6e  dbnz  $01,$1a89
1a1b: 01        tcall 0
1a1c: 6e 28 89  dbnz  $28,$19a8
1a1f: 00        nop
1a20: 86        adc   a,(x)
1a21: 08 49     or    a,#$49
1a23: 46        eor   a,(x)
1a24: 43 40 3d  bbs2  $40,$1a64
1a27: 3a 37     incw  $37
1a29: 34 31     and   a,$31+x
1a2b: 2e 2b 28  cbne  $2b,$1a56
1a2e: 86        adc   a,(x)
1a2f: 00        nop
1a30: 90 22     bcc   $1a54
1a32: 29 04 93  and   ($93),($04)
1a35: 27 4a     and   a,($4a+x)
1a37: 50 80     bvc   $19b9
1a39: b3 37 7f  bbc5  $37,$1abb
1a3c: a2 01     set5  $01
1a3e: 50 01     bvc   $1a41
1a40: 50 32     bvc   $1a74
1a42: 00        nop
1a43: 0a 89 18  or1   c,$1889,0
1a46: 90 00     bcc   $1a48
1a48: 32 01     clr1  $01
1a4a: 90 ff     bcc   $1a4b
1a4c: 9f        xcn   a
1a4d: 9c        dec   a
1a4e: 48 0c     eor   a,#$0c
1a50: 90 00     bcc   $1a52
1a52: 3c        rol   a
1a53: 01        tcall 0
1a54: 90 ff     bcc   $1a55
1a56: 4d        push  x
1a57: 0c 90 00  asl   $0090
1a5a: 46        eor   a,(x)
1a5b: 01        tcall 0
1a5c: 90 ff     bcc   $1a5d
1a5e: 52 0c     clr2  $0c
1a60: 90 00     bcc   $1a62
1a62: 50 01     bvc   $1a65
1a64: 90 64     bcc   $1aca
1a66: 92 01     clr4  $01
1a68: 2f 0c     bra   $1a76
1a6a: 80        setc
1a6b: b3 5a 7f  bbc5  $5a,$1aed
1a6e: a2 01     set5  $01
1a70: 3c        rol   a
1a71: 01        tcall 0
1a72: 3c        rol   a
1a73: 14 00     or    a,$00+x
1a75: 05 89 0f  or    a,$0f89
1a78: 96 13 01  adc   a,$0113+y
1a7b: 01        tcall 0
1a7c: 86        adc   a,(x)
1a7d: 02 32     set0  $32
1a7f: 9c        dec   a
1a80: 37 3c     and   a,($3c)+y
1a82: 41        tcall 4
1a83: 46        eor   a,(x)
1a84: 50 55     bvc   $1adb
1a86: 80        setc
1a87: b3 4b 7f  bbc5  $4b,$1b09
1a8a: a2 01     set5  $01
1a8c: 78 01 78  cmp   $78,#$01
1a8f: 0a 28 0a  or1   c,$0a28,0
1a92: 89 23 a3  adc   ($a3),($23)
1a95: 02 9a     set0  $9a
1a97: 1a ac     decw  $ac
1a99: 1a 24     decw  $24
1a9b: 01        tcall 0
1a9c: 90 05     bcc   $1aa3
1a9e: 9f        xcn   a
1a9f: 22 07     set1  $07
1aa1: 92 01     clr4  $01
1aa3: 00        nop
1aa4: 0a 8f 01  or1   c,$018f,0
1aa7: 0a 00 00  or1   c,$0000,0
1aaa: 0a 80 92  or1   c,$1280,4
1aad: 01        tcall 0
1aae: 8f 01 08  mov   $08,#$01
1ab1: 00        nop
1ab2: 23 0a 80  bbs1  $0a,$1a35
1ab5: b3 5a 7f  bbc5  $5a,$1b37
1ab8: a2 01     set5  $01
1aba: 7e 01     cmp   y,$01
1abc: 7e 01     cmp   y,$01
1abe: 7e 0a     cmp   y,$0a
1ac0: 89 01 34  adc   ($34),($01)
1ac3: 02 b3     set0  $b3
1ac5: 7d        mov   a,x
1ac6: 7f        reti
1ac7: 89 13 2c  adc   ($2c),($13)
1aca: 08 9c     or    a,#$9c
1acc: 21        tcall 2
1acd: 08 3a     or    a,#$3a
1acf: 02 2d     set0  $2d
1ad1: 04 37     or    a,$37
1ad3: 03 21 06  bbs0  $21,$1adc
1ad6: 3d        inc   x
1ad7: 01        tcall 0
1ad8: 92 01     clr4  $01
1ada: 1e 0b 80  cmp   x,$800b
1add: b3 4b 7f  bbc5  $4b,$1b5f
1ae0: a2 01     set5  $01
1ae2: 78 01 78  cmp   $78,#$01
1ae5: 1e 00 0f  cmp   x,$0f00
1ae8: 89 24 a3  adc   ($a3),($24)
1aeb: 02 f0     set0  $f0
1aed: 1a f8     decw  $f8
1aef: 1a 28     decw  $28
1af1: 04 9c     or    a,$9c
1af3: 25 04 24  and   a,$2404
1af6: 04 80     or    a,$80
1af8: 28 04     and   a,#$04
1afa: 9c        dec   a
1afb: 22 04     set1  $04
1afd: 21        tcall 2
1afe: 04 80     or    a,$80
1b00: b3 4b 7f  bbc5  $4b,$1b82
1b03: a2 01     set5  $01
1b05: 78 01 78  cmp   $78,#$01
1b08: 1e 00 0f  cmp   x,$0f00
1b0b: 89 25 a3  adc   ($a3),($25)
1b0e: 02 13     set0  $13
1b10: 1b 1b     asl   $1b+x
1b12: 1b 1e     asl   $1e+x
1b14: 04 9c     or    a,$9c
1b16: 14 04     or    a,$04+x
1b18: 13 04 80  bbc0  $04,$1a9b
1b1b: 1e 04 9c  cmp   x,$9c04
1b1e: 18 04 17  or    $17,#$04
1b21: 04 80     or    a,$80
1b23: b3 78 7f  bbc5  $78,$1ba5
1b26: a2 01     set5  $01
1b28: 78 01 78  cmp   $78,#$01
1b2b: 01        tcall 0
1b2c: 78 0a 89  cmp   $89,#$0a
1b2f: 00        nop
1b30: 96 13 01  adc   a,$0113+y
1b33: 03 86 04  bbs0  $86,$1b3a
1b36: 32 9f     clr1  $9f
1b38: 37 3c     and   a,($3c)+y
1b3a: 41        tcall 4
1b3b: 46        eor   a,(x)
1b3c: 50 55     bvc   $1b93
1b3e: 50 4b     bvc   $1b8b
1b40: 46        eor   a,(x)
1b41: 41        tcall 4
1b42: 3c        rol   a
1b43: 37 32     and   a,($32)+y
1b45: 80        setc
1b46: b3 46 7f  bbc5  $46,$1bc8
1b49: a2 01     set5  $01
1b4b: 3c        rol   a
1b4c: 01        tcall 0
1b4d: 3c        rol   a
1b4e: 01        tcall 0
1b4f: 3c        rol   a
1b50: 0a 89 1b  or1   c,$1b89,0
1b53: a6        sbc   a,(x)
1b54: 05 50 01  or    a,$0150
1b57: 94 01     adc   a,$01+x
1b59: 03 29 0a  bbs0  $29,$1b66
1b5c: 9f        xcn   a
1b5d: 95 4b 01  adc   a,$014b+x
1b60: 94 01     adc   a,$01+x
1b62: 03 24 0a  bbs0  $24,$1b6f
1b65: 95 46 01  adc   a,$0146+x
1b68: 94 01     adc   a,$01+x
1b6a: 05 1f 09  or    a,$091f
1b6d: 95 41 01  adc   a,$0141+x
1b70: 94 01     adc   a,$01+x
1b72: 02 1a     set0  $1a
1b74: 0a 95 46  or1   c,$0695,2
1b77: 01        tcall 0
1b78: 94 01     adc   a,$01+x
1b7a: 01        tcall 0
1b7b: 1f 0a 95  jmp   ($950a+x)

1b7e: 4b 01     lsr   $01
1b80: 94 01     adc   a,$01+x
1b82: 03 24 09  bbs0  $24,$1b8e
1b85: 95 a8 05  adc   a,$05a8+x
1b88: 55 1b 80  eor   a,$801b+x
1b8b: b5 1d b5  sbc   a,$b51d+x
1b8e: 1e a5 05  cmp   x,$05a5
1b91: b3 46 7f  bbc5  $46,$1c13
1b94: a2 01     set5  $01
1b96: 78 01 78  cmp   $78,#$01
1b99: 8c 00 0a  dec   $0a00
1b9c: 89 1b 50  adc   ($50),($1b)
1b9f: 01        tcall 0
1ba0: 9c        dec   a
1ba1: 94 01     adc   a,$01+x
1ba3: 01        tcall 0
1ba4: 32 0a     clr1  $0a
1ba6: 9f        xcn   a
1ba7: 95 4b 01  adc   a,$014b+x
1baa: 94 01     adc   a,$01+x
1bac: 03 2e 0a  bbs0  $2e,$1bb9
1baf: 95 46 01  adc   a,$0146+x
1bb2: 94 01     adc   a,$01+x
1bb4: 05 28 0a  or    a,$0a28
1bb7: 95 41 01  adc   a,$0141+x
1bba: 94 01     adc   a,$01+x
1bbc: 02 23     set0  $23
1bbe: 0a 95 3c  or1   c,$1c95,1
1bc1: 01        tcall 0
1bc2: 94 01     adc   a,$01+x
1bc4: 01        tcall 0
1bc5: 1e 0a 95  cmp   x,$950a
1bc8: 37 01     and   a,($01)+y
1bca: 94 01     adc   a,$01+x
1bcc: 03 19 0a  bbs0  $19,$1bd9
1bcf: 95 32 01  adc   a,$0132+x
1bd2: 94 01     adc   a,$01+x
1bd4: 05 14 0a  or    a,$0a14
1bd7: 95 2d 01  adc   a,$012d+x
1bda: 94 01     adc   a,$01+x
1bdc: 02 0f     set0  $0f
1bde: 0a 95 80  or1   c,$0095,4
1be1: b3 5a 7f  bbc5  $5a,$1c63
1be4: a2 01     set5  $01
1be6: 78 01 78  cmp   $78,#$01
1be9: 8c 00 0a  dec   $0a00
1bec: a5 05 89  sbc   a,$8905
1bef: 00        nop
1bf0: 96 13 01  adc   a,$0113+y
1bf3: 03 86 05  bbs0  $86,$1bfb
1bf6: 50 9f     bvc   $1b97
1bf8: 9c        dec   a
1bf9: 4d        push  x
1bfa: 4a 47 44  and1  c,$0447,2
1bfd: 41        tcall 4
1bfe: 3e 3b     cmp   x,$3b
1c00: 38 35 32  and   $32,#$35
1c03: 2f 2c     bra   $1c31
1c05: 29 26 23  and   ($23),($26)
1c08: 20        clrp
1c09: 1d        dec   x
1c0a: 1a 17     decw  $17
1c0c: 14 11     or    a,$11+x
1c0e: 80        setc
1c0f: b3 28 7f  bbc5  $28,$1c91
1c12: a2 01     set5  $01
1c14: 78 01 78  cmp   $78,#$01
1c17: 8c 00 0a  dec   $0a00
1c1a: a5 05 89  sbc   a,$8905
1c1d: 0f        brk
1c1e: 96 13 01  adc   a,$0113+y
1c21: 03 86 05  bbs0  $86,$1c29
1c24: 50 9c     bvc   $1bc2
1c26: 4d        push  x
1c27: 4a 47 44  and1  c,$0447,2
1c2a: 41        tcall 4
1c2b: 3e 3b     cmp   x,$3b
1c2d: 38 35 32  and   $32,#$35
1c30: 2f 2c     bra   $1c5e
1c32: 29 26 23  and   ($23),($26)
1c35: 20        clrp
1c36: 1d        dec   x
1c37: 1a 17     decw  $17
1c39: 14 11     or    a,$11+x
1c3b: 80        setc
1c3c: a7 07     sbc   a,($07+x)
1c3e: 42 1c     set2  $1c
1c40: b5 25 80  sbc   a,$8025+x
1c43: b3 7d 7f  bbc5  $7d,$1cc5
1c46: a2 01     set5  $01
1c48: 7d        mov   a,x
1c49: 01        tcall 0
1c4a: 7d        mov   a,x
1c4b: 14 28     or    a,$28+x
1c4d: 0f        brk
1c4e: 89 23 a3  adc   ($a3),($23)
1c51: 03 58 1c  bbs0  $58,$1c70
1c54: 6c 1c 7b  ror   $7b1c
1c57: 1c        asl   a
1c58: 18 01 90  or    $90,#$01
1c5b: 0a 8e 01  or1   c,$018e,0
1c5e: 04 00     or    a,$00
1c60: 9f        xcn   a
1c61: 26        and   a,(x)
1c62: 0a 92 01  or1   c,$0192,0
1c65: 8e        pop   psw
1c66: 01        tcall 0
1c67: 0a 00 00  or1   c,$0000,0
1c6a: 0f        brk
1c6b: 80        setc
1c6c: 8e        pop   psw
1c6d: 01        tcall 0
1c6e: 07 00     or    a,($00+x)
1c70: 17 0a     or    a,($0a)+y
1c72: 92 01     clr4  $01
1c74: 8e        pop   psw
1c75: 01        tcall 0
1c76: 1e 00 00  cmp   x,$0000
1c79: 0f        brk
1c7a: 80        setc
1c7b: 8e        pop   psw
1c7c: 01        tcall 0
1c7d: 0c 00 16  asl   $1600
1c80: 0a 92 01  or1   c,$0192,0
1c83: 8e        pop   psw
1c84: 01        tcall 0
1c85: 0a 00 00  or1   c,$0000,0
1c88: 0f        brk
1c89: 80        setc
1c8a: b3 7d 7f  bbc5  $7d,$1d0c
1c8d: a2 01     set5  $01
1c8f: 00        nop
1c90: 04 7e     or    a,$7e
1c92: 01        tcall 0
1c93: 7e 05     cmp   y,$05
1c95: 89 12 8f  adc   ($8f),($12)
1c98: 01        tcall 0
1c99: 6c 00 33  ror   $3300
1c9c: 0d        push  psw
1c9d: 80        setc
1c9e: b3 6e 7f  bbc5  $6e,$1d20
1ca1: a2 01     set5  $01
1ca3: 78 01 78  cmp   $78,#$01
1ca6: 26        and   a,(x)
1ca7: 00        nop
1ca8: 07 89     or    a,($89+x)
1caa: 19        or    (x),(y)
1cab: 1a 28     decw  $28
1cad: 80        setc
1cae: b3 50 7f  bbc5  $50,$1d30
1cb1: a2 01     set5  $01
1cb3: 00        nop
1cb4: 07 50     or    a,($50+x)
1cb6: 01        tcall 0
1cb7: 50 1a     bvc   $1cd3
1cb9: 89 19 1a  adc   ($1a),($19)
1cbc: 01        tcall 0
1cbd: 9c        dec   a
1cbe: 9f        xcn   a
1cbf: 90 fa     bcc   $1cbb
1cc1: 93 1a 50  bbc4  $1a,$1d14
1cc4: 1f 80 b3  jmp   ($b380+x)

1cc7: 78 7f a2  cmp   $a2,#$7f
1cca: 01        tcall 0
1ccb: 78 01 78  cmp   $78,#$01
1cce: 01        tcall 0
1ccf: 78 07 89  cmp   $89,#$07
1cd2: 15 32 06  or    a,$0632+x
1cd5: 89 01 9c  adc   ($9c),($01)
1cd8: 28 03     and   a,#$03
1cda: 9f        xcn   a
1cdb: 23 03 1e  bbs1  $03,$1cfc
1cde: 03 19 0a  bbs0  $19,$1ceb
1ce1: 80        setc
1ce2: a5 07 b3  sbc   a,$b307
1ce5: 3c        rol   a
1ce6: 7f        reti
1ce7: a2 01     set5  $01
1ce9: 00        nop
1cea: 09 50 01  or    ($01),($50)
1ced: 50 1e     bvc   $1d0d
1cef: 89 19 30  adc   ($30),($19)
1cf2: 01        tcall 0
1cf3: 90 28     bcc   $1d1d
1cf5: 9c        dec   a
1cf6: 9f        xcn   a
1cf7: 8f 01 05  mov   $05,#$01
1cfa: 55 27 01  eor   a,$0127+x
1cfd: 00        nop
1cfe: 01        tcall 0
1cff: a7 07     sbc   a,($07+x)
1d01: fd        mov   y,a
1d02: 1c        asl   a
1d03: 90 32     bcc   $1d37
1d05: 92 01     clr4  $01
1d07: 32 1f     clr1  $1f
1d09: 80        setc
1d0a: a6        sbc   a,(x)
1d0b: 07 80     or    a,($80+x)
1d0d: b3 5a 7f  bbc5  $5a,$1d8f
1d10: a2 01     set5  $01
1d12: 78 01 78  cmp   $78,#$01
1d15: 01        tcall 0
1d16: 78 19 89  cmp   $89,#$19
1d19: 02 31     set0  $31
1d1b: 03 89 19  bbs0  $89,$1d37
1d1e: 92 01     clr4  $01
1d20: 0b 14     asl   $14
1d22: 00        nop
1d23: 14 80     or    a,$80+x
1d25: b3 32 7f  bbc5  $32,$1da7
1d28: 89 0c a2  adc   ($a2),($0c)
1d2b: 01        tcall 0
1d2c: 7e 01     cmp   y,$01
1d2e: 7e 01     cmp   y,$01
1d30: 7e 08     cmp   y,$08
1d32: 92 01     clr4  $01
1d34: a3 02 3a  bbs5  $02,$1d71
1d37: 1d        dec   x
1d38: 49 1d 96  eor   ($96),($1d)
1d3b: 03 01 01  bbs0  $01,$1d3f
1d3e: 88 07     adc   a,#$07
1d40: 86        adc   a,(x)
1d41: 03 2c 9c  bbs0  $2c,$1ce0
1d44: 9f        xcn   a
1d45: 2e 2f 31  cbne  $2f,$1d79
1d48: 80        setc
1d49: 96 04 01  adc   a,$0104+y
1d4c: 01        tcall 0
1d4d: 88 09     adc   a,#$09
1d4f: 86        adc   a,(x)
1d50: 03 2c 9c  bbs0  $2c,$1cef
1d53: 9f        xcn   a
1d54: 2d        push  a
1d55: 2e 2f 80  cbne  $2f,$1cd8
1d58: a6        sbc   a,(x)
1d59: 08 80     or    a,#$80
1d5b: b3 78 7f  bbc5  $78,$1ddd
1d5e: 89 29 a5  adc   ($a5),($29)
1d61: 08 a2     or    a,#$a2
1d63: 01        tcall 0
1d64: 00        nop
1d65: 14 32     or    a,$32+x
1d67: 01        tcall 0
1d68: 32 14     clr1  $14
1d6a: 06        or    a,(x)
1d6b: 01        tcall 0
1d6c: 9c        dec   a
1d6d: 9f        xcn   a
1d6e: 90 04     bcc   $1d74
1d70: b8 00 09  sbc   $09,#$00
1d73: a3 02 79  bbs5  $02,$1def
1d76: 1d        dec   x
1d77: 81        tcall 8
1d78: 1d        dec   x
1d79: 88 02     adc   a,#$02
1d7b: b8 00 01  sbc   $01,#$00
1d7e: 81        tcall 8
1d7f: 86        adc   a,(x)
1d80: 1d        dec   x
1d81: 88 fe     adc   a,#$fe
1d83: b8 00 01  sbc   $01,#$00
1d86: a7 08     sbc   a,($08+x)
1d88: 73 1d 88  bbc3  $1d,$1d13
1d8b: 00        nop
1d8c: 92 01     clr4  $01
1d8e: 06        or    a,(x)
1d8f: 14 80     or    a,$80+x
1d91: b3 78 7f  bbc5  $78,$1e13
1d94: 89 2a a5  adc   ($a5),($2a)
1d97: 08 a2     or    a,#$a2
1d99: 01        tcall 0
1d9a: 00        nop
1d9b: 14 32     or    a,$32+x
1d9d: 01        tcall 0
1d9e: 32 14     clr1  $14
1da0: 2a 3c 9c  or1   c,!($1c3c,4)
1da3: 9f        xcn   a
1da4: 00        nop
1da5: 0a a7 08  or1   c,$08a7,0
1da8: a4 1d     sbc   a,$1d
1daa: 92 01     clr4  $01
1dac: 2a 14 80  or1   c,!($0014,4)
1daf: b3 78 7f  bbc5  $78,$1e31
1db2: 89 2a a5  adc   ($a5),($2a)
1db5: 08 a2     or    a,#$a2
1db7: 01        tcall 0
1db8: 00        nop
1db9: 14 50     or    a,$50+x
1dbb: 01        tcall 0
1dbc: 50 14     bvc   $1dd2
1dbe: 20        clrp
1dbf: 3c        rol   a
1dc0: 9c        dec   a
1dc1: 9f        xcn   a
1dc2: 00        nop
1dc3: 0a a7 08  or1   c,$08a7,0
1dc6: c2 1d     set6  $1d
1dc8: 92 01     clr4  $01
1dca: 20        clrp
1dcb: 14 80     or    a,$80+x
1dcd: b3 78 7f  bbc5  $78,$1e4f
1dd0: 89 00 a5  adc   ($a5),($00)
1dd3: 08 a2     or    a,#$a2
1dd5: 01        tcall 0
1dd6: 00        nop
1dd7: 14 28     or    a,$28+x
1dd9: 01        tcall 0
1dda: 28 14     and   a,#$14
1ddc: 8e        pop   psw
1ddd: 01        tcall 0
1dde: 78 08 35  cmp   $35,#$08
1de1: 3c        rol   a
1de2: 9c        dec   a
1de3: 9f        xcn   a
1de4: 00        nop
1de5: 0a a7 08  or1   c,$08a7,0
1de8: e4 1d     mov   a,$1d
1dea: 92 01     clr4  $01
1dec: 35 14 80  and   a,$8014+x
1def: b3 78 7f  bbc5  $78,$1e71
1df2: 89 13 a5  adc   ($a5),($13)
1df5: 08 a2     or    a,#$a2
1df7: 01        tcall 0
1df8: 50 01     bvc   $1dfb
1dfa: 50 14     bvc   $1e10
1dfc: 00        nop
1dfd: 14 9b     or    a,$9b+x
1dff: 9e        div   ya,x
1e00: 2f 01     bra   $1e03
1e02: 9c        dec   a
1e03: 9f        xcn   a
1e04: 90 aa     bcc   $1db0
1e06: 1e 07 90  cmp   x,$9007
1e09: 00        nop
1e0a: a7 08     sbc   a,($08+x)
1e0c: fe 1d     dbnz  y,$1e2b
1e0e: 80        setc
1e0f: a5 07 b3  sbc   a,$b307
1e12: 3c        rol   a
1e13: 7f        reti
1e14: a2 01     set5  $01
1e16: 00        nop
1e17: 09 3c 01  or    ($01),($3c)
1e1a: 3c        rol   a
1e1b: 1e 89 19  cmp   x,$1989
1e1e: 3a 01     incw  $01
1e20: 90 50     bcc   $1e72
1e22: 9c        dec   a
1e23: 9f        xcn   a
1e24: 8f 01 28  mov   $28,#$01
1e27: 0f        brk
1e28: 30 01     bmi   $1e2b
1e2a: 00        nop
1e2b: 01        tcall 0
1e2c: a7 07     sbc   a,($07+x)
1e2e: 2a 1e 90  or1   c,!($101e,4)
1e31: 32 92     clr1  $92
1e33: 01        tcall 0
1e34: 3a 1f     incw  $1f
1e36: 80        setc
1e37: a6        sbc   a,(x)
1e38: 07 80     or    a,($80+x)
1e3a: a5 07 b3  sbc   a,$b307
1e3d: 3c        rol   a
1e3e: 7f        reti
1e3f: a2 01     set5  $01
1e41: 00        nop
1e42: 05 46 01  or    a,$0146
1e45: 46        eor   a,(x)
1e46: 1e 89 19  cmp   x,$1989
1e49: 20        clrp
1e4a: 01        tcall 0
1e4b: 90 28     bcc   $1e75
1e4d: 9c        dec   a
1e4e: 9f        xcn   a
1e4f: 1b 01     asl   $01+x
1e51: 00        nop
1e52: 01        tcall 0
1e53: a7 07     sbc   a,($07+x)
1e55: 51        tcall 5
1e56: 1e 90 0a  cmp   x,$0a90
1e59: 92 01     clr4  $01
1e5b: 29 1f 80  and   ($80),($1f)
1e5e: a6        sbc   a,(x)
1e5f: 07 80     or    a,($80+x)
1e61: b3 55 7f  bbc5  $55,$1ee3
1e64: a2 01     set5  $01
1e66: 78 01 78  cmp   $78,#$01
1e69: 0a 28 0f  or1   c,$0f28,0
1e6c: 89 23 a3  adc   ($a3),($23)
1e6f: 02 74     set0  $74
1e71: 1e 9d 1e  cmp   x,$1e9d
1e74: 2c 01 9c  rol   $9c01
1e77: 9f        xcn   a
1e78: 90 0a     bcc   $1e84
1e7a: 2a 09 90  or1   c,!($1009,4)
1e7d: 32 30     clr1  $30
1e7f: 08 90     or    a,#$90
1e81: 00        nop
1e82: 30 01     bmi   $1e85
1e84: 90 1e     bcc   $1ea4
1e86: 2e 09 92  cbne  $09,$1e1b
1e89: 01        tcall 0
1e8a: 90 46     bcc   $1ed2
1e8c: 34 08     and   a,$08+x
1e8e: 90 00     bcc   $1e90
1e90: 34 01     and   a,$01+x
1e92: 90 32     bcc   $1ec6
1e94: 2f 03     bra   $1e99
1e96: 90 5a     bcc   $1ef2
1e98: 3a 04     incw  $04
1e9a: 90 00     bcc   $1e9c
1e9c: 80        setc
1e9d: 2d        push  a
1e9e: 01        tcall 0
1e9f: 9c        dec   a
1ea0: 9f        xcn   a
1ea1: 90 05     bcc   $1ea8
1ea3: 2b 09     rol   $09
1ea5: 90 32     bcc   $1ed9
1ea7: 31        tcall 3
1ea8: 08 90     or    a,#$90
1eaa: 00        nop
1eab: 31        tcall 3
1eac: 01        tcall 0
1ead: 90 1e     bcc   $1ecd
1eaf: 2f 09     bra   $1eba
1eb1: 92 01     clr4  $01
1eb3: 90 46     bcc   $1efb
1eb5: 35 08 90  and   a,$9008+x
1eb8: 00        nop
1eb9: 36 01 90  and   a,$9001+y
1ebc: 32 30     clr1  $30
1ebe: 03 90 5a  bbs0  $90,$1f1b
1ec1: 3b 04     rol   $04+x
1ec3: 90 00     bcc   $1ec5
1ec5: 80        setc
1ec6: b3 78 7f  bbc5  $78,$1f48
1ec9: a2 01     set5  $01
1ecb: 00        nop
1ecc: 04 78     or    a,$78
1ece: 08 1e     or    a,#$1e
1ed0: 07 89     or    a,($89+x)
1ed2: 25 88 01  and   a,$0188
1ed5: 38 01 9c  and   $9c,#$01
1ed8: 9f        xcn   a
1ed9: 90 3c     bcc   $1f17
1edb: 32 05     clr1  $05
1edd: 90 c8     bcc   $1ea7
1edf: 93 08 50  bbc4  $08,$1f32
1ee2: 0d        push  psw
1ee3: 80        setc
1ee4: b3 7d 7f  bbc5  $7d,$1f66
1ee7: a2 01     set5  $01
1ee9: 00        nop
1eea: 05 78 0a  or    a,$0a78
1eed: 28 07     and   a,#$07
1eef: 88 e8     adc   a,#$e8
1ef1: 89 27 86  adc   ($86),($27)
1ef4: 01        tcall 0
1ef5: 4c 9c 9f  lsr   $9f9c
1ef8: 46        eor   a,(x)
1ef9: 41        tcall 4
1efa: 3c        rol   a
1efb: 86        adc   a,(x)
1efc: 00        nop
1efd: 36 03 90  and   a,$9003+y
1f00: c8 93     cmp   x,#$93
1f02: 08 50     or    a,#$50
1f04: 0d        push  psw
1f05: 80        setc
1f06: b3 78 7f  bbc5  $78,$1f88
1f09: a2 01     set5  $01
1f0b: 78 01 78  cmp   $78,#$01
1f0e: 14 3c     or    a,$3c+x
1f10: 14 89     or    a,$89+x
1f12: 18 86 01  or    $01,#$86
1f15: 28 9f     and   a,#$9f
1f17: 9c        dec   a
1f18: 22 1b     set1  $1b
1f1a: 0f        brk
1f1b: 86        adc   a,(x)
1f1c: 00        nop
1f1d: 9e        div   ya,x
1f1e: 89 19 92  adc   ($92),($19)
1f21: 0a 1c 16  or1   c,$161c,0
1f24: 9f        xcn   a
1f25: 90 08     bcc   $1f2f
1f27: 32 09     clr1  $09
1f29: 80        setc
1f2a: b3 64 7f  bbc5  $64,$1fac
1f2d: a2 01     set5  $01
1f2f: 28 03     and   a,#$03
1f31: 7e 1e     cmp   y,$1e
1f33: 00        nop
1f34: 0a 89 19  or1   c,$1989,0
1f37: 2f 06     bra   $1f3f
1f39: 9f        xcn   a
1f3a: 2d        push  a
1f3b: 01        tcall 0
1f3c: 2b 01     rol   $01
1f3e: 8f 01 09  mov   $09,#$01
1f41: 00        nop
1f42: 28 1e     and   a,#$1e
1f44: 80        setc
1f45: b3 78 7f  bbc5  $78,$1fc7
1f48: a2 01     set5  $01
1f4a: 78 01 78  cmp   $78,#$01
1f4d: 0a 00 14  or1   c,$1400,0
1f50: 89 0c 86  adc   ($86),($0c)
1f53: 02 19     set0  $19
1f55: 9c        dec   a
1f56: 9f        xcn   a
1f57: 23 2d 37  bbs1  $2d,$1f91
1f5a: 80        setc
1f5b: b3 64 7f  bbc5  $64,$1fdd
1f5e: a2 01     set5  $01
1f60: 7e 01     cmp   y,$01
1f62: 7e 1e     cmp   y,$1e
1f64: 00        nop
1f65: 0a 89 19  or1   c,$1989,0
1f68: 14 06     or    a,$06+x
1f6a: 9f        xcn   a
1f6b: 1e 01 28  cmp   x,$2801
1f6e: 01        tcall 0
1f6f: 8f 01 09  mov   $09,#$01
1f72: 00        nop
1f73: 2c 1e 80  rol   $801e
1f76: b3 32 7f  bbc5  $32,$1ff8
1f79: a2 01     set5  $01
1f7b: 1a 03     decw  $03
1f7d: 7e 28     cmp   y,$28
1f7f: 00        nop
1f80: 0a 89 24  or1   c,$0489,1
1f83: 52 01     clr2  $01
1f85: 90 f0     bcc   $1f77
1f87: 1e 28 80  cmp   x,$8028
1f8a: b3 78 7f  bbc5  $78,$200c
1f8d: a2 01     set5  $01
1f8f: 78 01 78  cmp   $78,#$01
1f92: 14 00     or    a,$00+x
1f94: 14 89     or    a,$89+x
1f96: 24 28     and   a,$28
1f98: 01        tcall 0
1f99: 9f        xcn   a
1f9a: 90 c8     bcc   $1f64
1f9c: 9c        dec   a
1f9d: 32 03     clr1  $03
1f9f: 90 78     bcc   $2019
1fa1: 1b 0f     asl   $0f+x
1fa3: 80        setc
1fa4: b3 46 7f  bbc5  $46,$2026
1fa7: 89 1c a2  adc   ($a2),($1c)
1faa: 01        tcall 0
1fab: 7e 01     cmp   y,$01
1fad: 7e 01     cmp   y,$01
1faf: 7e 05     cmp   y,$05
1fb1: 92 01     clr4  $01
1fb3: a3 02 b9  bbs5  $02,$1f6f
1fb6: 1f c8 1f  jmp   ($1fc8+x)

1fb9: 96 03 01  adc   a,$0103+y
1fbc: 01        tcall 0
1fbd: 88 05     adc   a,#$05
1fbf: 86        adc   a,(x)
1fc0: 03 2c 9c  bbs0  $2c,$1f5f
1fc3: 9f        xcn   a
1fc4: 2f 31     bra   $1ff7
1fc6: 33 80 96  bbc1  $80,$1f5f
1fc9: 05 01 01  or    a,$0101
1fcc: 88 04     adc   a,#$04
1fce: 86        adc   a,(x)
1fcf: 03 2c 9c  bbs0  $2c,$1f6e
1fd2: 9f        xcn   a
1fd3: 30 32     bmi   $2007
1fd5: 34 80     and   a,$80+x
1fd7: b3 64 7f  bbc5  $64,$2059
1fda: a2 01     set5  $01
1fdc: 7e 01     cmp   y,$01
1fde: 7e 0a     cmp   y,$0a
1fe0: 00        nop
1fe1: 05 89 2c  or    a,$2c89
1fe4: 86        adc   a,(x)
1fe5: 05 88 02  or    a,$0288
1fe8: a3 04 f2  bbs5  $04,$1fdd
1feb: 1f f6 1f  jmp   ($1ff6+x)

1fee: fa 1f fe  mov   ($fe),($1f)
1ff1: 1f 44 9c  jmp   ($9c44+x)

1ff4: 44 80     eor   a,$80
1ff6: 42 9c     set2  $9c
1ff8: 42 80     set2  $80
1ffa: 45 9c 45  eor   a,$459c
1ffd: 80        setc
1ffe: 47 9c     eor   a,($9c+x)
2000: 47 80     eor   a,($80+x)
2002: b3 78 7f  bbc5  $78,$2084
2005: 89 2b 88  adc   ($88),($2b)
2008: 04 a2     or    a,$a2
200a: 01        tcall 0
200b: 78 01 78  cmp   $78,#$01
200e: 01        tcall 0
200f: 78 0a 2d  cmp   $2d,#$0a
2012: 05 2d 03  or    a,$032d
2015: 2d        push  a
2016: 03 2d 02  bbs0  $2d,$201b
2019: 2d        push  a
201a: 03 2d 03  bbs0  $2d,$2020
201d: 2d        push  a
201e: 05 80 b3  or    a,$b380
2021: 78 7f 89  cmp   $89,#$7f
2024: 2b 88     rol   $88
2026: 01        tcall 0
2027: a2 01     set5  $01
2029: 78 01 78  cmp   $78,#$01
202c: 01        tcall 0
202d: 78 0a 2d  cmp   $2d,#$0a
2030: 07 2d     or    a,($2d+x)
2032: 06        or    a,(x)
2033: 2d        push  a
2034: 05 2d 06  or    a,$062d
2037: 2d        push  a
2038: 04 2d     or    a,$2d
203a: 03 2d 03  bbs0  $2d,$2040
203d: 2d        push  a
203e: 03 2d 03  bbs0  $2d,$2044
2041: 2d        push  a
2042: 04 2d     or    a,$2d
2044: 05 80 b3  or    a,$b380
2047: 78 7f 89  cmp   $89,#$7f
204a: 2b 88     rol   $88
204c: f2 a2     clr7  $a2
204e: 01        tcall 0
204f: 78 01 78  cmp   $78,#$01
2052: 01        tcall 0
2053: 78 0a 2d  cmp   $2d,#$0a
2056: 09 2d 07  or    ($07),($2d)
2059: 2d        push  a
205a: 06        or    a,(x)
205b: 2d        push  a
205c: 05 2d 05  or    a,$052d
205f: 2d        push  a
2060: 06        or    a,(x)
2061: 2d        push  a
2062: 07 2d     or    a,($2d+x)
2064: 08 2d     or    a,#$2d
2066: 09 80 b3  or    ($b3),($80)
2069: 78 7f 89  cmp   $89,#$7f
206c: 2c 86 04  rol   $0486
206f: a2 01     set5  $01
2071: 78 01 78  cmp   $78,#$01
2074: 18 00 0a  or    $0a,#$00
2077: 39        and   (x),(y)
2078: 9c        dec   a
2079: 3d        inc   x
207a: 41        tcall 4
207b: 45 49 a2  eor   a,$a249
207e: 01        tcall 0
207f: 3c        rol   a
2080: 01        tcall 0
2081: 3c        rol   a
2082: 18 00 0a  or    $0a,#$00
2085: 39        and   (x),(y)
2086: 9c        dec   a
2087: 3d        inc   x
2088: 41        tcall 4
2089: 45 49 a2  eor   a,$a249
208c: 01        tcall 0
208d: 1e 01 1e  cmp   x,$1e01
2090: 18 00 0a  or    $0a,#$00
2093: 39        and   (x),(y)
2094: 9c        dec   a
2095: 3d        inc   x
2096: 41        tcall 4
2097: 45 49 80  eor   a,$8049
209a: b3 78 7f  bbc5  $78,$211c
209d: a2 01     set5  $01
209f: 78 01 78  cmp   $78,#$01
20a2: 01        tcall 0
20a3: 78 07 89  cmp   $89,#$07
20a6: 21        tcall 2
20a7: 2c 01 9c  rol   $9c01
20aa: 9f        xcn   a
20ab: 90 b9     bcc   $2066
20ad: 1e 0a 90  cmp   x,$900a
20b0: 03 32 14  bbs0  $32,$20c7
20b3: 80        setc
20b4: b3 78 7f  bbc5  $78,$2136
20b7: a2 01     set5  $01
20b9: 78 01 78  cmp   $78,#$01
20bc: 01        tcall 0
20bd: 78 07 89  cmp   $89,#$07
20c0: 21        tcall 2
20c1: 28 01     and   a,#$01
20c3: 9c        dec   a
20c4: 9f        xcn   a
20c5: 90 b9     bcc   $2080
20c7: 19        or    (x),(y)
20c8: 0a 90 03  or1   c,$0390,0
20cb: 32 14     clr1  $14
20cd: 80        setc
20ce: b5 45 b3  sbc   a,$b345+x
20d1: 32 7f     clr1  $7f
20d3: 98 a2 01  adc   $01,#$a2
20d6: 14 28     or    a,$28+x
20d8: 32 05     clr1  $05
20da: 0a 1e 89  or1   c,$091e,4
20dd: 0b 86     asl   $86
20df: 02 9a     set0  $9a
20e1: 1a 03     decw  $03
20e3: 9c        dec   a
20e4: 9a 1b     subw  ya,$1b
20e6: 03 86 05  bbs0  $86,$20ee
20e9: 9a 1c     subw  ya,$1c
20eb: 03 9a 1d  bbs0  $9a,$210b
20ee: 03 9a 1e  bbs0  $9a,$210f
20f1: 86        adc   a,(x)
20f2: 00        nop
20f3: 03 0a 92  bbs0  $0a,$2088
20f6: 01        tcall 0
20f7: 9a 1f     subw  ya,$1f
20f9: 86        adc   a,(x)
20fa: 00        nop
20fb: 03 1e 99  bbs0  $1e,$2097
20fe: 80        setc
20ff: b3 78 7f  bbc5  $78,$2181
2102: 89 2b a2  adc   ($a2),($2b)
2105: 01        tcall 0
2106: 78 01 78  cmp   $78,#$01
2109: 1e 3c 0a  cmp   x,$0a3c
210c: 2d        push  a
210d: 07 9c     or    a,($9c+x)
210f: 2d        push  a
2110: 06        or    a,(x)
2111: 2c 05 2c  rol   $2c05
2114: 06        or    a,(x)
2115: 2b 04     rol   $04
2117: 2b 03     rol   $03
2119: 2a 03 2a  or1   c,!($0a03,1)
211c: 03 29 03  bbs0  $29,$2122
211f: 92 01     clr4  $01
2121: 29 04 28  and   ($28),($04)
2124: 05 80 b3  or    a,$b380
2127: 78 7f a2  cmp   $a2,#$7f
212a: 01        tcall 0
212b: 00        nop
212c: 07 78     or    a,($78+x)
212e: 04 00     or    a,$00
2130: 04 89     or    a,$89
2132: 00        nop
2133: a3 03 3b  bbs5  $03,$2171
2136: 21        tcall 2
2137: 51        tcall 5
2138: 21        tcall 2
2139: 67 21     cmp   a,($21+x)
213b: 49 01 90  eor   ($90),($01)
213e: 3c        rol   a
213f: 9f        xcn   a
2140: 50 0d     bvc   $214f
2142: 90 00     bcc   $2144
2144: 49 01 90  eor   ($90),($01)
2147: 1e 92 05  cmp   x,$0592
214a: 0a 12 92  or1   c,$1212,4
214d: 00        nop
214e: 90 00     bcc   $2150
2150: 80        setc
2151: 49 01 90  eor   ($90),($01)
2154: 28 9f     and   a,#$9f
2156: 50 0d     bvc   $2165
2158: 90 00     bcc   $215a
215a: 4a 01 90  and1  c,$1001,4
215d: 14 92     or    a,$92+x
215f: 05 50 12  or    a,$1250
2162: 92 00     clr4  $00
2164: 90 00     bcc   $2166
2166: 80        setc
2167: 49 01 90  eor   ($90),($01)
216a: 32 9f     clr1  $9f
216c: 50 0d     bvc   $217b
216e: 90 00     bcc   $2170
2170: 49 01 90  eor   ($90),($01)
2173: 32 92     clr1  $92
2175: 05 0a 12  or    a,$120a
2178: 92 00     clr4  $00
217a: 90 00     bcc   $217c
217c: 80        setc
217d: b3 78 7f  bbc5  $78,$21ff
2180: 89 2a 88  adc   ($88),($2a)
2183: f2 a2     clr7  $a2
2185: 01        tcall 0
2186: 78 01 78  cmp   $78,#$01
2189: 28 00     and   a,#$00
218b: 0a 18 0b  or1   c,$0b18,0
218e: 19        or    (x),(y)
218f: 09 1b 07  or    ($07),($1b)
2192: 1d        dec   x
2193: 06        or    a,(x)
2194: 86        adc   a,(x)
2195: 05 9c 20  or    a,$209c
2198: 23 27 29  bbs1  $27,$21c4
219b: 2f 33     bra   $21d0
219d: 37 80     and   a,($80)+y
219f: b3 78 7f  bbc5  $78,$2221
21a2: 89 2b 88  adc   ($88),($2b)
21a5: ff        stop
21a6: a2 01     set5  $01
21a8: 78 01 78  cmp   $78,#$01
21ab: 14 00     or    a,$00+x
21ad: 0a 23 0a  or1   c,$0a23,0
21b0: 23 06 24  bbs1  $06,$21d7
21b3: 05 24 06  or    a,$0624
21b6: 25 04 26  and   a,$2604
21b9: 03 9c 27  bbs0  $9c,$21e3
21bc: 03 28 03  bbs0  $28,$21c2
21bf: 2a 03 2c  or1   c,!($0c03,1)
21c2: 04 2e     or    a,$2e
21c4: 05 80 b5  or    a,$b580
21c7: 4a b3 50  and1  c,$10b3,2
21ca: 14 a2     or    a,$a2+x
21cc: 01        tcall 0
21cd: 78 01 78  cmp   $78,#$01
21d0: 8c 00 02  dec   $0200
21d3: 89 19 11  adc   ($11),($19)
21d6: 0a 9f 1e  or1   c,$1e9f,0
21d9: 01        tcall 0
21da: 23 01 8f  bbs1  $01,$216c
21dd: 01        tcall 0
21de: 02 00     set0  $00
21e0: 27 8c     and   a,($8c+x)
21e2: 80        setc
21e3: b3 50 ec  bbc5  $50,$21d2
21e6: a2 01     set5  $01
21e8: 78 01 78  cmp   $78,#$01
21eb: 8c 00 01  dec   $0100
21ee: 89 19 18  adc   ($18),($19)
21f1: 0d        push  psw
21f2: 9f        xcn   a
21f3: 8f 01 01  mov   $01,#$01
21f6: 00        nop
21f7: 23 8c 80  bbs1  $8c,$217a
21fa: b3 78 7f  bbc5  $78,$227c
21fd: 89 2d a5  adc   ($a5),($2d)
2200: 08 a2     or    a,#$a2
2202: 01        tcall 0
2203: 00        nop
2204: 14 50     or    a,$50+x
2206: 01        tcall 0
2207: 50 14     bvc   $221d
2209: 8e        pop   psw
220a: 01        tcall 0
220b: 0f        brk
220c: 0a 36 01  or1   c,$0136,0
220f: 9c        dec   a
2210: 9f        xcn   a
2211: 90 28     bcc   $223b
2213: 3a 09     incw  $09
2215: 00        nop
2216: 01        tcall 0
2217: a7 08     sbc   a,($08+x)
2219: 15 22 92  or    a,$9222+x
221c: 01        tcall 0
221d: 2d        push  a
221e: 14 80     or    a,$80+x
2220: a6        sbc   a,(x)
2221: 08 80     or    a,#$80
2223: b3 78 7f  bbc5  $78,$22a5
2226: a2 01     set5  $01
2228: 50 01     bvc   $222b
222a: 50 01     bvc   $222d
222c: 50 07     bvc   $2235
222e: 89 2d a3  adc   ($a3),($2d)
2231: 02 36     set0  $36
2233: 22 4a     set1  $4a
2235: 22 3a     set1  $3a
2237: 01        tcall 0
2238: 9c        dec   a
2239: 9f        xcn   a
223a: 90 23     bcc   $225f
223c: 1e 0a 90  cmp   x,$900a
223f: 00        nop
2240: 9e        div   ya,x
2241: 89 21 1e  adc   ($1e),($21)
2244: 03 89 25  bbs0  $89,$226c
2247: 30 08     bmi   $2251
2249: 80        setc
224a: 38 01 9c  and   $9c,#$01
224d: 9f        xcn   a
224e: 90 23     bcc   $2273
2250: 1a 0a     decw  $0a
2252: 90 00     bcc   $2254
2254: 9e        div   ya,x
2255: 89 21 1c  adc   ($1c),($21)
2258: 03 89 25  bbs0  $89,$2280
225b: 2e 08 80  cbne  $08,$21de
225e: b3 78 7f  bbc5  $78,$22e0
2261: a2 01     set5  $01
2263: 50 01     bvc   $2266
2265: 50 01     bvc   $2268
2267: 50 07     bvc   $2270
2269: 89 2c 96  adc   ($96),($2c)
226c: 0d        push  psw
226d: 01        tcall 0
226e: 01        tcall 0
226f: a3 02 75  bbs5  $02,$22e7
2272: 22 78     set1  $78
2274: 22 35     set1  $35
2276: 06        or    a,(x)
2277: 80        setc
2278: 34 06     and   a,$06+x
227a: 80        setc
