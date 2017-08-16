@echo.>stderr.txt
@for %%a in (*.spc) do softcspc --loop 2 --patchfix plok.lst %%a %%~na.mid %%~na.html 2>>stderr.txt
