	chdir exedir
	var_0 = 0
	goto * label_1
#deffunc dll_getfunc val, str, int
	mref var_1, 16
	mref var_2, 33
	mref var_3, 2
	if var_3 & -256 = 0 {
		var_3 = var_4 . var_3
	}
	ll_getproc var_1, var_2, var_3
	if var_1 = 0 {
		dialog "can not find '" + var_2 + "'\ndll=" + var_3
	}
	return
#deffunc getptr val, val
	mref var_1, 16
	mref var_2, 1025
	if ( var_2 & 65535 = 2 ) {
		mref var_3, 25
	}
	else {
		mref var_3, 17
	}
	ll_getptr var_3
	var_1 = var_0
	return
#deffunc _init_llmod int, int, int, int
	if var_4 {
		return
	}
	sdim var_5, 64, 16
	ll_retset var_0
	var_5 . 0 = "kernel32"
	var_5 . 1 = "user32"
	var_5 . 2 = "shell32"
	var_5 . 3 = "comctl32"
	var_5 . 4 = "comdlg32"
	var_5 . 5 = "gdi32"
	repeat 6
		ll_libload var_4 . cnt, var_5 . cnt
	loop
	var_5 = "SendMessageA", "CreateWindowExA", "GetActiveWindow"
	repeat 3
		ll_getproc var_6 . cnt, var_5 . cnt, var_4 . 1
	loop
	alloc var_5, 64
	mref var_7, 64
	gosub label_9
	return
#deffunc dllproc str, val, int, int
	mref var_8, 32
	mref var_9, 17
	mref var_10, 2
	mref var_11, 3
	if var_11 & -256 {
		var_0 = var_11
	}
	else {
		var_0 = var_4 . var_11
	}
	ll_getproc var_12, var_8, var_0
	if var_12 {
		ll_callfunc var_9, var_10, var_12
		var_7 = var_0
	}
	else {
		dialog "can not find '" + var_8 + "'\ndll=" + var_11
		getkey var_13, 16
		if var_13 {
			end
		}
	}
	return
#deffunc getmjrdll val, int
	mref var_14, 16
	mref var_15, 1
	var_14 = var_4 . var_15
	return
#deffunc getmjrfunc val, int
	mref var_14, 16
	mref var_15, 1
	var_14 = var_6 . var_15
	return
*label_9
	alloc var_16, 4 * 64
	return
#deffunc _cls int
	mref var_14
	var_12 = var_6 . 0
	var_17 . 1 = 16, 0, 0
	repeat 64
		var_17 = var_16 . cnt
		if var_17 {
			ll_callfunc var_17, 4, var_12
		}
	loop
	gosub label_9
	cls var_14
	return
#deffunc _get_instance val
	mref var_14, 16
	mref var_18, 67
	var_14 = var_18 . 14
	return
#deffunc _get_active_window val
	mref var_14, 16
	ll_callfnv var_6 . 2
	var_14 = var_0
	return
#deffunc sendmsg val
	mref var_14, 16
	ll_callfunc var_14, 4, var_6 . 0
	var_7 = var_0
	return
#deffunc setwndlong val, int
	mref var_14, 16
	mref var_15, 1
	if var_15 {
		var_5 = "G"
		var_13 = 2
	}
	else {
		var_5 = "S"
		var_13 = 3
	}
	var_5 + = "etWindowLongA"
	dllproc var_5, var_14, var_13, 1
	return
#deffunc _null_sep_str val, int
	mref var_14, 24
	mref var_15, 1
	strlen var_19, var_14
	var_13 = 0
	var_17 = 0
	repeat var_19
		peek var_13, var_14, cnt
		if var_13 = var_15 {
			poke var_14, cnt, 0
			var_17 +
		}
	loop
	var_7 = var_17
	return
#deffunc _makewnd val, str
	mref var_20, 16
	mref var_21, 33
	mref var_18, 67
	var_22 = -1
	repeat 64
		if var_16 . cnt = 0 {
			var_22 = cnt
			break
		}
	loop
	if var_22 = -1 {
		var_7 = -1
		return
	}
	if var_20 . 2 = 0 {
		var_20 . 2 = var_18 . 29
	}
	if var_20 . 3 = 0 {
		var_20 . 3 = var_18 . 30
	}
	var_17 = var_20 . 6
	var_5 = var_21
	getptr var_17 . 1, var_5
	var_23 = ""
	getptr var_17 . 2, var_23
	var_17 . 3 = var_20 . 4, var_20 . 0, var_20 . 1, var_20 . 2, var_20 . 3, 0, 0, 0, 0
	if var_20 . 5 {
		var_17 . 8 = var_20 . 5
	}
	else {
		var_17 . 8 = var_18 . 13
	}
	_get_instance var_17 . 10
	ll_callfunc var_17, 12, var_6 . 1
	var_20 = var_0
	var_16 . var_22 = var_20
	if var_17 . 7 < var_18 . 31 {
		var_13 = var_18 . 31
	}
	else {
		var_13 = var_17 . 7
	}
	pos csrx, csry + var_13
	var_17 = var_20, - 12, 4096 + var_22
	setwndlong var_17
	var_7 = 0
	return
#deffunc _is_wnd int
	mref var_14
	dllproc "IsWindow", var_14, 1, 1
	return
#deffunc _hspobjhandle int
	mref var_14
	mref var_18, 67
	if ( var_14 < 0 ) | ( var_14 > 63 ) {
		var_7 = 0
	}
	else {
		var_14 + = 41
		var_7 = var_18 . var_14
	}
	return
#deffunc _hspobjid int
	mref var_14
	mref var_18, 67
	var_7 = -1
	repeat 64, 41
		if var_18 . cnt = var_14 {
			var_7 = cnt - 41
			break
		}
	loop
	return
#deffunc _objsel int
	mref var_14
	if var_14 = -1 {
		mref var_18, 67
		dllproc "GetFocus", var_13, 0, 1
		if stat = var_18 . 13 {
			var_7 = -1
			return
		}
		var_13 = stat
		_hspobjid var_13
		if stat ! -1 {
			var_13 = stat
		}
		var_7 = var_13
	}
	else {
		var_13 = var_14
		_hspobjhandle var_13
		if stat {
			var_13 = stat
		}
		dllproc "SetFocus", var_13, 1, 1
	}
	return
#deffunc _clrobj int, int
	mref var_14, 0
	mref var_15, 1
	var_13 = var_14
	_hspobjhandle var_13
	if stat {
		clrobj var_14
		return
	}
	var_17 = var_13, 16, 0, 0
	ll_callfunc var_17, 4, var_6 . 0
	repeat 64
		if var_16 . cnt = var_13 {
			var_16 . cnt = 0
			break
		}
	loop
	var_7 = var_0
	return
#deffunc charupper val
	mref var_14, 1024
	dllproc "CharUpperA", var_14 . 7, 1, 1
	return
#deffunc charlower val
	mref var_14, 1024
	dllproc "CharLowerA", var_14 . 7, 1, 1
	return
*label_1
	_init_llmod
	goto * label_37
#deffunc __xdim val, int
	mref var_24, 1024
	mref var_25, 1
	var_26 = var_24 . 7, var_25 << 2, 64
	ll_callfunc var_26, 4, var_27
	return
*label_37
	dup var_27, var_28 . 1
	dup var_26, var_28 . 2
	ll_getptr var_26 . 4
	ll_ret var_26 . 3
	ll_libload var_28, "kernel32.dll"
	ll_getproc var_27, "VirtualProtect", var_28
	goto * label_39
*label_40
	mref var_29, 64
	ll_libload var_30, "kernel32.dll"
	ll_getproc var_31, "GetCommandLineA", var_30
	dim var_32, 6
	__xdim var_32, 6
	var_32 = 69485707, 947964043, - 1979157504, - 2076180144
	var_32 . 4 = 737703378, 50113
	ll_getptr var_32
	ll_ret var_33
	dim var_34, 28
	__xdim var_34, 28
	var_34 = 136594571, 1955288659, - 768472028, - 645265270
	var_34 . 4 = -2145324160, - 75472957, 1712224059, - 1065154421
	var_34 . 8 = 243885570, - 352139645, 586776803, - 226490763
	var_34 . 12 = -655671295, 1948318080, 167346181, - 763100043
	var_34 . 16 = 243797108, - 914093942, 1178607732, 113688299
	var_34 . 20 = 21531136, 553222208, - 108988556, - 1963822071
	var_34 . 24 = -621388079, 593220123, 861848514, 12803008
	ll_getptr var_34
	ll_ret var_35
	return
#deffunc setargv val, val, int
	mref var_36, 16
	var_36 = 0
	mref var_37, 57
	mref var_38, 2
	ll_callfunc var_39, 0, var_31
	ll_ret var_40
	ll_callfunc var_40, 1, var_33
	ll_ret var_41
	sdim var_42, var_41 + 1
	sdim var_43, var_41 + 1
	ll_peek var_43, var_40
	var_44 = 1 + ( hspstat & 1 )
	ll_getptr var_42
	ll_ret var_39
	var_39 . 1 = var_40
	repeat
		if var_39 . 1 = 0 {
			break
		}
		ll_callfunc var_39, 2, var_35
		ll_ret var_39 . 1
		if var_44 {
			var_44 - -
			continue
		}
		if var_38 & 1 {
			peek var_45, var_42, 0
			if ( var_45 = 45 ) | ( var_45 = 47 ) {
				continue
			}
		}
		var_37 . var_36 = var_42
		var_36 + +
	loop
	sdim var_42, 64
	var_29 = var_36
	return
#deffunc exit__setargv__ int, int, int, int
	ll_libfree var_30
	return
*label_39
	gosub * label_40
	goto * label_45
*label_46
	mref var_46, 64
	dim var_47, 6
	__xdim var_47, 6
	var_47 = 1458342741, - 1962127989, 109709429, 109609012, - 160085690, 12803422
	ll_getptr var_47
	ll_ret var_48
	ll_libload var_49, "shell32.dll"
	ll_getproc var_50, "ShellExecuteA", var_49
	return
#deffunc __exec str
	mref var_51, 32
	strlen var_52, var_51
	sdim var_53, var_52 + 1
	var_53 = var_51
	var_54 = 0, 0, 0, 0, 0, 1
	ll_getptr var_53
	ll_ret var_54 . 2
	ll_callfunc var_54, 6, var_50
	sdim var_53, 64
	return
#deffunc cnvsign8 val, int, int
	mref var_55, 1024
	mref var_56, 1
	mref var_57, 2
	var_54 = var_55 . 7 + var_57, var_56
	ll_callfunc var_54, 2, var_48
	return
#deffunc exit__gba2wavp__ int, int, int, int
	ll_libfree var_49
	return
*label_45
	gosub * label_46
	sdim var_58, 260
	var_58 = curdir + "\\"
	sdim var_59, 260, 64
	setargv var_60, var_59, 1
	sdim var_61, 512
	sdim var_62, 260
	sdim var_63, 260
	sdim var_64, 5
	sdim var_65, 260
	sdim var_66, 512
	screen 0, 320, 240, 5, dispx + 20, dispy + 20
	gsel 0, 1
	cls
	title "gba2wav Plus!" + " " + "1.1"
	mes "do not see..."
	if var_60 = 0 {
		dialog "gba;*.bin", 16, "gba roms"
		if stat = 0 {
			end
		}
		else {
			var_62 = refstr
		}
	}
	else {
		var_62 = var_59 . 0
	}
	exist var_62
	var_67 = strsize
	if var_67 < 0 {
		dialog "GBA-ROM not found", 0, "gba2wav Plus!"
		end
	}
	if var_67 < 256 {
		dialog "Invalid file", 0, "gba2wav Plus!"
		end
	}
	bload var_62, var_61, 4, 172
	poke var_61, 4, 0
	var_68 = 1
	repeat 4
		peek var_69, var_61, cnt
		if ( var_69 < 32 ) | ( var_69 > 127 ) {
			var_68 = 0
			break
		}
	loop
	if var_68 = 0 {
		dialog "Invalid signature", 0, "gba2wav Plus!"
		end
	}
	var_64 = var_61
	charlower var_64
	sdim var_70, var_67
	bload var_62, var_70
	exist var_58 + "list" + "\\" + var_64 + ".txt"
	if strsize >= 0 {
		var_63 = var_58 + "list" + "\\" + var_64 + ".txt"
	}
	else {
		dialog "txt", 16, "Import List"
		if stat = 0 {
			end
		}
		else {
			var_63 = refstr
		}
	}
	gosub * label_52
	exist var_63
	if strsize < 0 {
		dialog "List not found", 0, "gba2wav Plus!"
		end
	}
	sdim var_71, 1048576
	notesel var_72
	noteload var_63
	notemax var_73
	var_74 = 1
	repeat var_73
		var_75 = cnt + 1
		title "[" + var_75 + "/" + var_73 + "] - " + "gba2wav Plus!" + " " + "1.1"
		await
		noteget var_66, cnt
		if var_66 = "" {
			continue
		}
		peek var_69, var_66, 0
		if var_69 = 59 {
			continue
		}
		strmid var_61, var_66, 0, 2
		if var_61 = "//" {
			continue
		}
		if var_61 = "/*" {
			var_74 = 0
			continue
		}
		if var_61 = "*/" {
			var_74 = 1
			continue
		}
		if var_74 = 0 {
			continue
		}
		if var_69 = 91 {
			getstr var_61, var_66, 1, 93
			var_61 = "$" + var_61
			int var_61
			var_76 = var_61
			str var_61
			gosub * label_55
		}
	loop
	strlen var_69, var_71
	if var_69 {
		bsave var_64 + ".m3u", var_71, var_69
	}
	__exec var_65
	end
*label_52
	var_65 = var_58
	chdir var_58
	dirlist var_61, "output", 5
	if stat = 0 {
		mkdir "output"
	}
	var_65 + = "\\" + "output"
	chdir var_65
	dirlist var_61, var_64, 5
	if stat = 0 {
		mkdir var_64
	}
	var_65 + = "\\" + var_64
	chdir var_65
	return
*label_55
	if var_76 >= var_67 {
		return
	}
	int var_77
	var_77 = var_76
	str var_77, 24
	if var_76 + 12 >= var_67 {
		return
	}
	memcpy var_78, var_70, 4, ?, var_76 + 4
	var_78 = var_78 & 16777215
	if var_78 + 15 >= var_67 {
		return
	}
	wpeek var_79, var_70, var_78 + 12
	peek var_69, var_70, var_78 + 14
	var_79 + = var_69 << 16
	var_79 + +
	if var_78 + 15 + var_79 > var_67 {
		return
	}
	memcpy var_80, var_70, 4, ?, var_78 + 5
	var_80 = var_80 & 131071 >> 2
	if var_80 = 0 {
		return
	}
	dim var_81, 11
	var_81 = 1179011410, 0, 1163280727, 544501094
	var_81 . 4 = 16, 65537, 0, 0
	var_81 . 8 = 524289, 1635017060, 0
	var_69 = 44 + var_79
	memcpy var_81, var_69, 4, 4
	var_69 = var_80
	memcpy var_81, var_69, 4, 24
	var_69 = var_80
	memcpy var_81, var_69, 4, 28
	var_69 = var_79
	memcpy var_81, var_69, 4, 40
	sdim var_82, 44 + var_79
	memcpy var_82, var_81, 44
	memcpy var_82, var_70, var_79, 44, var_78 + 15
	cnvsign8 var_82, var_79, 44
	skiperr 1
	bsave var_77 + ".wav", var_82, 44 + var_79
	if err {
		if err = 12 {
			dialog "Cannot create [" + var_77 + ".wav" + "]", 0, "gba2wav Plus!"
		}
		else {
			dialog "HSP Runtime Error (" + err + ")", 0, "HSP Error"
			end
		}
	}
	else {
		var_71 + = var_77 + ".wav" + "\n"
	}
	skiperr 0
	return
	end
