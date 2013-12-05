@set HAVE_PAC=1
@for %%a in (*.pac) do @set HAVE_PAC=0
@if %HAVE_PAC%==1 @goto needs_pac

@if not exist HokutoLZSS.exe @call BuildAll.bat

@call UnPACAll.bat
@call UnLZSSAll.bat
@call SplitDMFAll.bat
@call DMFToMidiAll.bat
@exit /B 0

:needs_pac
@echo Please put BGMALL*.PAC into the same folder.
@pause
@exit /B 1
