@rem This batch might be useful if you going to use "Tool to search VH/VBs from single file", such as PGconv.
@for %%a in (*.vh) do @copy /b "%%~dpna.vh" + "%%~dpna.vb" "%%~dpna.vab"
