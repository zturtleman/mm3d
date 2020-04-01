; Maverick Model 3D - NSI Script (Installer Script)
;
; Copyright (c) 2004-2007 Kevin Worcester
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
; USA.
;
; See the COPYING file for full license text.


!define VERSION "1.3.12"
!define FILE_VERSION "1_3_12"

Name "Maverick Model 3D ${VERSION}"
OutFile "mm3d-${FILE_VERSION}-win32-installer.exe"

SetCompressor lzma
Icon mm3d.ico
UninstallIcon mm3d.ico
BrandingText "Maverick Model 3D"
CRCCheck on
XPStyle on

InstallDir "$PROGRAMFILES\Maverick Model 3D"
InstallDirRegKey HKCU "Software\MaverickModel\Maverick Model 3D" "INSTDIR"

LicenseData COPYING

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Maverick Model 3D"

    SectionIn RO

    ; Create file type
    WriteRegStr HKCR "MaverickModel.Mm3dModelFile" "" "MM3D Model File"
    WriteRegStr HKCR "MaverickModel.Mm3dModelFile\shell\open\command" "" '"$INSTDIR\mm3d.x86.exe" "%1"'
    WriteRegStr HKCR "MaverickModel.Mm3dModelFile\shell\edit" "" "Edit Model"
    WriteRegStr HKCR "MaverickModel.Mm3dModelFile\shell\edit\command" "" '"$INSTDIR\mm3d.x86.exe" "%1"'

    SetOutPath "$INSTDIR"
    WriteRegStr HKCU "Software\MaverickModel\Maverick Model 3D" "INSTDIR" "$INSTDIR"

    File /oname=COPYING.txt COPYING
    File build\mingw32-x86\install\mm3d.x86.exe
    File /r build\mingw32-x86\install\doc

    SetOutPath "$INSTDIR\i18n"
    File build\mingw32-x86\install\i18n\*.qm

    SetOutPath "$INSTDIR"
    CreateDirectory "$INSTDIR\plugins"
    CreateDirectory "$INSTDIR\plugins\1.3"

    ; Qt dlls
    File build\mingw32-x86\install\Qt5Core.dll
    File build\mingw32-x86\install\Qt5Gui.dll
    File build\mingw32-x86\install\Qt5Svg.dll
    File build\mingw32-x86\install\Qt5Widgets.dll

    SetOutPath "$INSTDIR\iconengines"
    File build\mingw32-x86\install\iconengines\*.dll

    SetOutPath "$INSTDIR\imageformats"
    File build\mingw32-x86\install\imageformats\*.dll

    SetOutPath "$INSTDIR\platforms"
    File build\mingw32-x86\install\platforms\*.dll

    SetOutPath "$INSTDIR\styles"
    File build\mingw32-x86\install\styles\*.dll

    ; Qt translations
    SetOutPath "$INSTDIR\translations"
    File build\mingw32-x86\install\translations\*.qm

    SetOutPath "$INSTDIR"

    ; Qt MinGW compiler run-time
    File build\mingw32-x86\install\libgcc_s_dw2-1.dll
    File build\mingw32-x86\install\libstdc++-6.dll
    File build\mingw32-x86\install\libwinpthread-1.dll

    ; Additional dlls used by Qt
    File build\mingw32-x86\install\D3Dcompiler_47.dll
    File build\mingw32-x86\install\libEGL.dll
    File build\mingw32-x86\install\libGLESV2.dll
    File build\mingw32-x86\install\opengl32sw.dll

    WriteUninstaller "Uninstall.exe"

SectionEnd

Section "Start Menu Shortcuts"

    ; SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\Maverick Model 3D"
    CreateShortcut "$SMPROGRAMS\Maverick Model 3D\Maverick Model 3D.lnk"    "$INSTDIR\mm3d.x86.exe"
    CreateShortcut "$SMPROGRAMS\Maverick Model 3D\Help Documentation.lnk" "$INSTDIR\doc\html\olh_index.html"
    CreateShortcut "$SMPROGRAMS\Maverick Model 3D\License.lnk"            "$INSTDIR\doc\html\olh_license.html"
    CreateShortcut "$SMPROGRAMS\Maverick Model 3D\MM3D Web Page.lnk"      "https://clover.moe/mm3d"
    CreateShortcut "$SMPROGRAMS\Maverick Model 3D\Uninstall.lnk"          "$INSTDIR\Uninstall.exe"

SectionEnd

SubSection /e "Associate file types"

    Section "MM3D (Misfit Model 3D)"
        WriteRegStr HKCR ".mm3d" "" "MaverickModel.Mm3dModelFile"
    SectionEnd

    Section /o "Cal3d"
        WriteRegStr HKCR ".cal" "" "MaverickModel.Mm3dModelFile"
    SectionEnd

    Section /o "COB (Truespace)"
        WriteRegStr HKCR ".cob" "" "MaverickModel.Mm3dModelFile"
    SectionEnd

    Section /o "DXF (Autocad DXF)"
        WriteRegStr HKCR ".dxf" "" "MaverickModel.Mm3dModelFile"
    SectionEnd

    Section /o "LWO (Lightwave)"
        WriteRegStr HKCR ".lwo" "" "MaverickModel.Mm3dModelFile"
    SectionEnd

    Section /o "MS3D (Milkshape)"
        WriteRegStr HKCR ".ms3d" "" "MaverickModel.Mm3dModelFile"
    SectionEnd

    Section /o "MD2 (Quake)"
        WriteRegStr HKCR ".md2" "" "MaverickModel.Mm3dModelFile"
    SectionEnd

    Section /o "MD3 (Quake)"
        WriteRegStr HKCR ".md3" "" "MaverickModel.Mm3dModelFile"
    SectionEnd

    Section /o "OBJ (Alias Wavefront)"
        WriteRegStr HKCR ".obj" "" "MaverickModel.Mm3dModelFile"
    SectionEnd

SubSectionEnd

Section "Uninstall"

    Delete "$INSTDIR\COPYING.txt"
    Delete "$INSTDIR\mm3d.x86.exe"
    Delete "$INSTDIR\i18n\*.qm"

    ; Qt dlls
    Delete "$INSTDIR\Qt5Core.dll"
    Delete "$INSTDIR\Qt5Gui.dll"
    Delete "$INSTDIR\Qt5Svg.dll"
    Delete "$INSTDIR\Qt5Widgets.dll"
    Delete "$INSTDIR\iconengines\*.dll"
    Delete "$INSTDIR\imageformats\*.dll"
    Delete "$INSTDIR\platforms\*.dll"
    Delete "$INSTDIR\styles\*.dll"

    ; Qt translations
    Delete "$INSTDIR\translations\*.qm"

    ; Qt MinGW compiler run-time
    Delete "$INSTDIR\libgcc_s_dw2-1.dll"
    Delete "$INSTDIR\libstdc++-6.dll"
    Delete "$INSTDIR\libwinpthread-1.dll"

    ; Additional dlls used by Qt
    Delete "$INSTDIR\D3Dcompiler_47.dll"
    Delete "$INSTDIR\libEGL.dll"
    Delete "$INSTDIR\libGLESV2.dll"
    Delete "$INSTDIR\opengl32sw.dll"

    Delete "$INSTDIR\Uninstall.exe"

    RMDir /r /REBOOTOK "$INSTDIR\doc"
    RMDir /REBOOTOK "$INSTDIR\i18n"
    RMDir /REBOOTOK "$INSTDIR\plugins\1.3"
    RMDir /REBOOTOK "$INSTDIR\plugins"

    ; Qt directories
    RMDir /REBOOTOK "$INSTDIR\iconengines"
    RMDir /REBOOTOK "$INSTDIR\imageformats"
    RMDir /REBOOTOK "$INSTDIR\platforms"
    RMDir /REBOOTOK "$INSTDIR\styles"
    RMDir /REBOOTOK "$INSTDIR\translations"

    RMDir $INSTDIR

    ; SetShellVarContext all
    RMDir /r /REBOOTOK "$SMPROGRAMS\Maverick Model 3D"

    DeleteRegKey HKCR "MaverickModel.Mm3dModelFile"

SectionEnd

