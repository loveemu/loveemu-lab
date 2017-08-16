@echo.>stderr.txt
@for %%a in (mmx-*.spc) do capspc --loop 2 --gs --patchfix mmx_gs.lst %%a %%~na.mid %%~na.html 2>>stderr.txt
