@type nul >DMFToMidiAll.log
@for %%a in (*.dmf) do @java -jar dmfMus.jar "%%a" >>DMFToMidiAll.log 2>&1

@rem redirect to each logs
@rem %%a >%%~dpna.mid.log 2>&1
