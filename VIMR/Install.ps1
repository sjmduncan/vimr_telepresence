function MakeShortcut($SourceExe, $DestinationPath){
  Write-Host $DestinationPath
  $WshShell = New-Object -comObject WScript.Shell
  $Shortcut = $WshShell.CreateShortcut($DestinationPath)
  $Shortcut.TargetPath = "$pwd\..\$SourceExe"
  $Shortcut.WorkingDirectory = "$pwd\..\"
  $Shortcut.IconLocation = "$pwd\lib\Icon128.ico"
  $Shortcut.Save()
}

MakeShortcut "editor-host.bat"            "$HOME\Desktop\host telepresence.lnk"
MakeShortcut "editor-join.bat"            "$HOME\Desktop\join telepresence.lnk"
MakeShortcut "vimr-all-calibrate.bat"       "$HOME\Desktop\calibrate cameras.lnk"
MakeShortcut "vimr-all-restart-cameras.bat" "$HOME\Desktop\restart cameras.lnk"
MakeShortcut "vimr-all-stop.bat"        "$HOME\Desktop\stop telepresence.lnk"
