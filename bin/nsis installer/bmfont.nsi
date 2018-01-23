; bmfont.nsi
;

; Make sure we have the right priviledges to do our stuff
RequestExecutionLevel admin 

; The name of the installer
Name "Bitmap Font Generator v1.14 beta"

; The file to write
OutFile "install_bmfont_1.14_beta.exe"

SubCaption 0 ": Read Me"
LicenseText " " "Continue"
LicenseData "..\readme.txt"

; The default installation directory
InstallDir $PROGRAMFILES\AngelCode\BMFont
; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\AngelCode\BMFont "InstallDir"

ComponentText "This will install Bitmap Font Generator on your computer. Select the options to install."
; The text to prompt the user to enter a directory
DirText "Choose a directory to install in."

; The stuff to install
Section "BMFont (required)"
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  File "..\bmfont.exe"
  File "..\bmfont.com"
  File "..\readme.txt"
  File "..\documentation.html"
  SetOutPath "$INSTDIR\doc"
  File "..\doc\command_line.html"
  File "..\doc\export_options.html"  
  File "..\doc\export_options.png"  
  File "..\doc\file_format.html"  
  File "..\doc\font_settings.html"  
  File "..\doc\font_settings.png"  
  File "..\doc\known_issues.html"  
  File "..\doc\pixel_shader.html"  
  File "..\doc\render_text.html"  
  File "..\doc\measures.png"  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\AngelCode\BMFont "InstallDir" "$INSTDIR"
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BMFont" "DisplayName" "Bitmap Font Generator"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BMFont" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"
SectionEnd

; optional section
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\AngelCode\Bitmap Font Generator"
  CreateShortCut "$SMPROGRAMS\AngelCode\Bitmap Font Generator\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\AngelCode\Bitmap Font Generator\BMFont.lnk" "$INSTDIR\bmfont.exe" "" "$INSTDIR\bmfont.exe" 0
  CreateShortCut "$SMPROGRAMS\AngelCode\Bitmap Font Generator\ReadMe.lnk" "$INSTDIR\readme.txt" "" "$INSTDIR\readme.txt" 0
  CreateShortCut "$SMPROGRAMS\AngelCode\Bitmap Font Generator\Documentation.lnk" "$INSTDIR\documentation.html" "" "$INSTDIR\documentation.html" 0
SectionEnd

; uninstall stuff

UninstallText "This will uninstall Bitmap Font Generator. Hit next to continue."

; special uninstall section.
Section "Uninstall"
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BMFont"
  DeleteRegKey HKLM SOFTWARE\AngelCode\BMFont
  ; remove files
  Delete $INSTDIR\bmfont.exe
  Delete $INSTDIR\bmfont.com
  Delete $INSTDIR\readme.txt
  Delete $INSTDIR\documentation.html
  Delete $INSTDIR\doc\command_line.html
  Delete $INSTDIR\doc\export_options.html
  Delete $INSTDIR\doc\export_options.png
  Delete $INSTDIR\doc\file_format.html
  Delete $INSTDIR\doc\font_settings.html
  Delete $INSTDIR\doc\font_settings.png
  Delete $INSTDIR\doc\known_issues.html
  Delete $INSTDIR\doc\pixel_shader.html
  Delete $INSTDIR\doc\render_text.html
  Delete $INSTDIR\doc\measures.png
  ; Delete the config file that is generated automatically
  Delete $INSTDIR\bmfont.bmfc
  ; MUST REMOVE UNINSTALLER, too
  Delete $INSTDIR\uninstall.exe
  ; remove shortcuts, if any.
  Delete "$SMPROGRAMS\AngelCode\Bitmap Font Generator\Uninstall.lnk"
  Delete "$SMPROGRAMS\AngelCode\Bitmap Font Generator\BMFont.lnk"
  Delete "$SMPROGRAMS\AngelCode\Bitmap Font Generator\ReadMe.lnk"
  Delete "$SMPROGRAMS\AngelCode\Bitmap Font Generator\Documentation.lnk"
  ; remove directories used.
  RMDir "$SMPROGRAMS\AngelCode\Bitmap Font Generator"
  ; remove the AngelCode folder only if it is empty
  RMDir "$SMPROGRAMS\AngelCode"
  RMDir "$INSTDIR\doc"
  RMDir "$INSTDIR"
SectionEnd

; eof
