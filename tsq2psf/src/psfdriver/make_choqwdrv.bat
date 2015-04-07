@set PSFDRV=choqwdrv

@del %PSFDRV%.bin
@del %PSFDRV%.obj

@rem Compile.
@rem Note that -G 0 is crucial here to avoid using $gp.
@ccpsx -G 0 -O2 -Wall -c %PSFDRV%.c

@rem Link.
@rem /o 0x80xxxxxx - origin of output. again, remember to change
@rem /p            - pure binary output
@rem /z            - fill BSS with zeroes
@psylink /o 0x80010000 /p /z %PSFDRV%.obj,%PSFDRV%.bin

@del %PSFDRV%.obj
