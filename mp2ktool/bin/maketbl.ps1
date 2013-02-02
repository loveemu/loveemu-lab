get-childitem *.gba | foreach ($_) {
    $ofslistline = ./mp2ktool ofslist $_.fullname
    if ($LastExitCode -eq 0) {
        echo "$ofslistline"
    }
}
