@setlocal
@set PMMOPTS=
@for %%a in (%*) do @java -jar %~dp0PetiteMM.jar %PMMOPTS% "%%~fa"
@endlocal
@pause
