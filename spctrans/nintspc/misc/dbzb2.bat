@echo.>stderr.txt
@for %%a in (*.spc) do nintspc --loop 2 --songlist 1e00 --blockptr 4d --durtbl 1d80 --veltbl 0c26 %%a %%~na.mid %%~na.html 2>>stderr.txt
