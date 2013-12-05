@call "%VS100COMNTOOLS%\vsvars32.bat"
@for %%a in (*.cpp) do cl "%%a"
