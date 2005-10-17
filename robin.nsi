; robin.nsi
;
; Robin and griffin install script.
;--------------------------------
; Includes
!include "MUI.nsh"
!include "Sections.nsh"
!include "LogicLib.nsh"

SetCompressor /SOLID lzma

;--------------------------------
; General
Name "Robin python-wrapper"
!ifndef VERSION
    !define VERSION '1.0'
!endif

Icon "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
UninstallIcon "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

OutFile "robin.exe"

InstType "Full"
InstallDir "$PROGRAMFILES\Robin"

InstallDirRegKey HKLM "Software\Robin" ""

;--------------------------------
; Interface Settings
!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_SMALLDESC

;--------------------------------
; Pages
!define MUI_WELCOMEPAGE_TITLE "Welcome to the Robin ${VERSION} Setup Wizard"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of Robin library and griffin wrapping engine ${VERSION}.\n\n$_CLICK"
!insertmacro MUI_PAGE_WELCOME

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_LINK "Visit the Robin site for the latest version."
!define MUI_FINISHPAGE_LINK_LOCATION "http://robin-py.sf.net/"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; The stuff to install
Section "Robin library files (Required)" SecRobin
    SectionIn 1 RO

    Call GetPythonDir ; Sets the python directory in $0.
    WriteRegStr HKLM "Software\Robin\" "PythonDir" "$0"
    WriteRegStr HKLM "Software\Robin\" "RobinVer" "${VERSION}"
    SetOutPath "$0\Lib\robin"
    File "robin.py"
    File "griffin.py"
    File "src\robin\modules\document.py"
    File "src\robin\modules\robinhelp.py"
    File "src\robin\modules\pickle_weakref.py"
    File "src\robin\modules\stl.py"
    SetOutPath "$0\Lib\robin\html"
    File "src\robin\modules\html\__init__.py"
    File "src\robin\modules\html\textformat.py"

    setOutPath "$0\DLLs"
    File "robin-1.0.dll"
    File "robin_pyfe-1.0.dll"
    File "robin_stl.dll"

    SetOutPath "$INSTDIR"
    WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Robin" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Robin" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Robin" "DisplayName" "Robin auto-wrapping system"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Robin" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Robin" "URLInfoAbout" "http://robin-py.sourceforge.net/"
    WriteUninstaller "uninstall.exe"

    CreateDirectory "$SMPROGRAMS\Robin"
    CreateShortCut "$SMPROGRAMS\Robin\Uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

Section "Griffin wrapping program" SecGriffin
    SearchPath $2 java.exe
    ${If} $2 != ""
    ${OrIf} ${Cmd} `MessageBox MB_YESNO|MB_ICONQUESTION "Java could not be found on this machine. It is nesessary for installing 'Griffin'. Do you want to install it anyway?" IDYES`
        SectionIn 1
        WriteRegStr HKLM "Software\Robin\" "GriffinVer" "${VERSION}"
        SetOutPath "$INSTDIR"
        File "griffin"
        File "Griffin.jar"
        File "build\stl.tag"
        File "src\griffin\modules\stl\stl.st.xml"
        SetOutPath "$INSTDIR\dox-xml"
        File "build\dox-xml\classstd_1_1vector.xml"
        File "build\dox-xml\combine.xslt"
        File "build\dox-xml\compound.xsd"
        File "build\dox-xml\index.xml"
        File "build\dox-xml\index.xsd"
        File "build\dox-xml\namespacestd.xml"
        File "build\dox-xml\tag__vector_8h.xml"
        SetOutPath "$INSTDIR\premises"
        File "premises\antlr-2.7.5.jar"
        File "premises\junit.jar"
        File "premises\jython.jar"
        File "premises\xercesImpl.jar"
        File "premises\xmlParserAPIs.jar"
    ${EndIf}
SectionEnd

;--------------------------------
; Uninstaller
UninstallText "This will uninstall Robin and Griffin."

Section un.Griffin unSecGriffin
    SectionIn RO
    DeleteRegValue HKLM "Software\Robin" "GriffinVer"

    Delete "$INSTDIR\griffin"
    Delete "$INSTDIR\Griffin.jar"
    Delete "$INSTDIR\stl.tag"
    Delete "$INSTDIR\stl.st.xml"
    Delete "$INSTDIR\dox-xml\*.*"
    RMDir  "$INSTDIR\dox-xml"
    Delete "$INSTDIR\premises\*.*"
    RMDir  "$INSTDIR\premises"
SectionEnd

Section /o un.Robin unSecRobin
    ReadRegStr $0 HKLM "Software\Robin" "PythonDir"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Robin"
    DeleteRegValue HKLM "Software\Robin" "RobinVer"
    DeleteRegKey HKLM "Software\Robin"
    Delete "$0\Lib\robin\html\*.*"
    RMDir  "$0\Lib\robin\html"
    Delete "$0\Lib\robin\*.*"
    RMDir  "$0\Lib\robin"
    Delete "$0\DLLs\robin-1.0.dll"
    Delete "$0\DLLs\robin_pyfe-1.0.dll"
    Delete "$0\DLLs\robin_stl.dll"
    Delete "$INSTDIR\*.*"
    RMDir  "$INSTDIR"
    Delete "$SMPROGRAMS\Robin\*.*"
    RMDir  "$SMPROGRAMS\Robin"
SectionEnd

;--------------------------------
; Descriptions
LangString DESC_Robin ${LANG_ENGLISH} "This is the robin library, for loading and running robin wraped C++ libraries in python."
LangString DESC_Griffin ${LANG_ENGLISH} "This is the Griffin library, for wrapin C++ libraries to python."
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecRobin} $(DESC_Robin)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecGriffin} $(DESC_Griffin)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
  
;--------------------------------
; Functions
Function .onInit
    Call GetPythonDir
FunctionEnd

Function un.onInit
    ReadRegStr $0 HKLM "Software\Robin" "GriffinVer"
    ${If} $0 == ""
        SectionSetFlags ${unSecGriffin} ${SECTION_OFF}
        SectionSetText  ${unSecGriffin} ""
        SectionSetFlags ${unSecRobin} ${SF_RO}
        !insertmacro SelectSection ${unSecRobin}
    ${EndIf}
FunctionEnd

Function GetPythonDir
    ReadRegStr $0 HKLM "Software\Python\PythonCore\2.4\InstallPath" ""
    ${Unless} ${FileExists} "$0\python.exe"
        MessageBox MB_OK "Couldn't find python installation. Aborting"
        Abort
    ${EndUnless}
FunctionEnd

