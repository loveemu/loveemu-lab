@echo.>stderr.txt
@for %%a in (*.spc) do rarespc --loop 2 %%a %%~na.mid %%~na.html 2>>stderr.txt
