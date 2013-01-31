get-childitem *.gba | foreach ($_) {
    $romid     = ./mp2ktool header romid $_.fullname
    $songtable = ./mp2ktool songtable $_.fullname
    $ret       = $LastExitCode

#   $mplayer   = ./mp2ktool mplaytable $_.fullname
#   if ($LastExitCode -ne 0) {
#       $mplayer = ""
#   }

    if ($ret -eq 0) {
        $romname = $_.Name.Trim()
        echo "$romid,$songtable,""$romname"","""""
    }
}
