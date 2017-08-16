@set HAVE_PAC=1
@for %%a in (*.pac) do @set HAVE_PAC=0
@if %HAVE_PAC%==1 @goto needs_pac

@if not exist HokutoUnPAC.exe @goto needs_dependency
@if not exist HokutoSplitDMF.exe @goto needs_dependency
@if not exist dmfMus.jar @goto needs_dependency

@call UnPACAll.bat
@call SplitDMFAll.bat
@call DMFToMidiAll.bat
@call VABJoin.bat
@exit /B 0

:needs_pac
@echo Please put BGMALL*.PAC into the same folder.
@pause
@exit /B 1

:needs_dependency
@echo Please put the following dependencies into the same folder.
@echo.
@echo - HokutoUnPAC.exe
@echo - HokutoSplitDMF.exe
@echo - dmfMus.jar
@pause
@exit /B 1
