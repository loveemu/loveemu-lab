@rem @copy /B sgng-01.spc sgng.spc
@rem capspc --norel --song 1 --count 0x16 --loop 2 --gs --patchfix sgng_gs.lst sgng.spc sgng.mid sgng.html 2>stderr.txt

@echo.>stderr.txt
@for %%a in (sgng-*.spc) do capspc --loop 2 --gs --patchfix sgng_gs.lst %%a %%~na.mid %%~na.html 2>>stderr.txt
