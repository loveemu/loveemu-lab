@set target_asm=%~1
@set target_base=%~dpn1
@set target_basename=%~n1

@if "%target_asm%" == "" @goto err_no_input

@wla-65816 -ivo "%target_asm%" "%target_base%.obj"
@if ERRORLEVEL 1 @goto err_wla65816

@echo [objects] > "%target_base%.ofl"
@echo %target_basename%.obj >> "%target_base%.ofl"

@wlalink -irv "%target_base%.ofl" "%target_base%.smc"
@if ERRORLEVEL 1 @goto err_wlalink

@del "%target_base%.obj"
@del "%target_base%.ofl"
@del "%target_base%.lst"

@exit /b 0

:err_no_input
@echo No input file >&2
@goto err_quit

:err_wla65816
@goto err_quit

:err_wlalink
@del "%target_base%.obj"
@del "%target_base%.ofl"
@goto err_quit

:err_quit
@exit /b 1
