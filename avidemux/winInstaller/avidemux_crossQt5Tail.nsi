
${MementoSection} "-Start menu Change Log" SecStartMenuChangeLog
    CreateDirectory $SMPROGRAMS\$StartMenuGroup
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetOutPath $INSTDIR
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Change Log ${CORE_VERSION}.lnk" "$INSTDIR\ChangeLog.html"
    !insertmacro MUI_STARTMENU_WRITE_END
${MementoSectionEnd}


${MementoSection} "-Start menu Qt" SecStartMenuQt
    CreateDirectory $SMPROGRAMS\$StartMenuGroup
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetOutPath $INSTDIR
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\${SHORTCUT_NAME}.lnk" $INSTDIR\avidemux.exe
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Avidemux job control (${BUILD_BITS} Bits).lnk" $INSTDIR\avidemux_jobs.exe
    !insertmacro MUI_STARTMENU_WRITE_END
${MementoSectionEnd}

${MementoSection} "-Start menu AVS Proxy GUI" SecStartMenuAvsProxyGui
    CreateDirectory $SMPROGRAMS\$StartMenuGroup
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetOutPath $INSTDIR
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\AVS Proxy GUI ${CORE_VERSION}.lnk" "$INSTDIR\avsproxy_gui.exe"
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\VS Proxy GUI ${CORE_VERSION}.lnk" "$INSTDIR\vsProxy_gui_qt5.exe"
    !insertmacro MUI_STARTMENU_WRITE_END
${MementoSectionEnd}


${MementoSection} "-Quick Launch Qt" SecQuickLaunchQt
    SetOutPath $INSTDIR
    CreateShortcut "$QUICKLAUNCH\${SHORTCUT_NAME}.lnk" $INSTDIR\avidemux.exe
${MementoSectionEnd}


${MementoSection} "-Desktop Qt" SecDesktopQt
    SetOutPath $INSTDIR
    CreateShortcut "$DESKTOP\${SHORTCUT_NAME}.lnk" $INSTDIR\avidemux.exe
${MementoSectionEnd}

${MementoSectionDone}

Section -post SecUninstaller
    SectionIn 1 2
    WriteRegStr HKLM "${REGKEY}" Path $INSTDIR
    WriteRegStr HKLM "${REGKEY}" Version ${PRODUCT_VERSION}
    SetOutPath $INSTDIR
    WriteUninstaller $INSTDIR\uninstall.exe
    WriteRegStr HKLM "${UNINST_REGKEY}" DisplayName "${SHORTCUT_NAME}"
    WriteRegStr HKLM "${UNINST_REGKEY}" DisplayVersion "${PRODUCT_VERSION}"
    WriteRegStr HKLM "${UNINST_REGKEY}" DisplayIcon $INSTDIR\uninstall.exe
    WriteRegStr HKLM "${UNINST_REGKEY}" UninstallString $INSTDIR\uninstall.exe
    WriteRegDWORD HKLM "${UNINST_REGKEY}" NoModify 1
    WriteRegDWORD HKLM "${UNINST_REGKEY}" NoRepair 1
SectionEnd

Section -CloseLogFile
	FileClose $UninstallLogHandle
	SetFileAttributes ${UninstallLogPath} HIDDEN
SectionEnd
 
Section Uninstall
	!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuGroup	


    Delete /REBOOTOK "$QUICKLAUNCH\${SHORTCUT_NAME}.lnk"
    Delete /REBOOTOK "$DESKTOP\${SHORTCUT_NAME}.lnk"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\${SHORTCUT_NAME}.lnk"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Avidemux job control (${BUILD_BITS} Bits).lnk"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Change Log ${CORE_VERSION}.lnk"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\AVS Proxy GUI ${CORE_VERSION}.lnk"
    RmDir /REBOOTOK $SMPROGRAMS\$StartMenuGroup
    DeleteRegValue HKLM "${REGKEY}" StartMenuGroup
    
    DeleteRegKey HKLM "${UNINST_REGKEY}"
    DeleteRegValue HKLM "${REGKEY}" Path
	DeleteRegValue HKLM "${REGKEY}" CreateDesktopIcon
	DeleteRegValue HKLM "${REGKEY}" CreateStartMenuGroup
	DeleteRegValue HKLM "${REGKEY}" CreateQuickLaunchIcon
    DeleteRegKey /IfEmpty HKLM "${REGKEY}"

	FileOpen $UninstallLogHandle "${UninstallLogPath}" r
UninstallLoop:
    ClearErrors
    FileRead $UninstallLogHandle $R0
    IfErrors UninstallEnd
	Push $R0
    Call un.TrimNewLines
    Pop $R0
    Delete "$R0"
    Goto UninstallLoop
UninstallEnd:
	FileClose $UninstallLogHandle
	Delete "${UninstallLogPath}"
	Delete "$INSTDIR\uninstall.exe"
	Push "\"
	Call un.RemoveEmptyDirs
	RMDir "$INSTDIR"
SectionEnd

##########################
# Installer functions
##########################
Function .onInit
UAC_Elevate:
	!insertmacro UAC_RunElevated
	StrCmp 1223 $0 UAC_ElevationAborted
	StrCmp 0 $0 0 UAC_Err
	StrCmp 1 $1 0 UAC_Success
	Quit

UAC_Err:
	MessageBox MB_ICONSTOP "Unable to elevate, error $0"
	Abort

UAC_ElevationAborted:
	Abort

UAC_Success:
	StrCmp 1 $3 +4
	StrCmp 3 $1 0 UAC_ElevationAborted
	MessageBox MB_ICONSTOP "This installer requires admin access."
	Goto UAC_Elevate

	Call LoadPreviousSettings
	ReadRegStr $PreviousVersion HKLM "${REGKEY}" Version

	${If} $PreviousVersion != ""
		${VersionCompare} ${PRODUCT_VERSION} $PreviousVersion $PreviousVersionState
	${EndIf}
	
    InitPluginsDir
    SetShellVarContext all
FunctionEnd

Function .onInstSuccess
	${MementoSectionSave}
FunctionEnd

Function LoadPreviousSettings
    ${MementoSectionRestore}
	!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuGroup

	ReadRegStr $CreateStartMenuGroup HKLM "${REGKEY}" CreateStartMenuGroup

	${If} $CreateStartMenuGroup == ""
		StrCpy $CreateStartMenuGroup 1
	${EndIf}

	ReadRegStr $CreateDesktopIcon HKLM "${REGKEY}" CreateDesktopIcon

	${If} $CreateDesktopIcon == ""
		StrCpy $CreateDesktopIcon 1
	${EndIf}

	${If} ${AtMostWinVista}
		ReadRegStr $CreateQuickLaunchIcon HKLM "${REGKEY}" CreateQuickLaunchIcon

		${If} $CreateQuickLaunchIcon == ""
			StrCpy $CreateQuickLaunchIcon 1
		${EndIf}
	${EndIf}

FunctionEnd

Function RunUninstaller
    ReadRegStr $R1  HKLM "${UNINST_REGKEY}" "UninstallString"

	${If} $R1 == ""
		Return
	${EndIf}

	;Run uninstaller
	HideWindow
	ClearErrors
	
	${If} $PreviousVersionState == 0
	${AndIf} $ReinstallUninstall == 1
		ExecWait '$R1 _?=$INSTDIR'
	${Else}
		ExecWait '$R1 /frominstall _?=$INSTDIR'
	${EndIf}

	IfErrors NoRemoveUninstaller
	IfFileExists "$INSTDIR\uninstall.exe" 0 NoRemoveUninstaller
		Delete "$R1"
		RMDir $INSTDIR

NoRemoveUninstaller:
FunctionEnd

Function CheckSelectedUIs
	!insertmacro SectionFlagIsSet ${SecGrpUI} ${SF_SELECTED} end checkPartial
checkPartial:
	!insertmacro SectionFlagIsSet ${SecGrpUI} ${SF_PSELECTED} end displayError
displayError:
    MessageBox MB_OK|MB_ICONSTOP "At least one User Interface must be selected."
    Abort
end:
FunctionEnd

LangString INSTALL_OPTS_PAGE_TITLE ${LANG_ENGLISH} "Choose Install Options"
LangString INSTALL_OPTS_PAGE_SUBTITLE ${LANG_ENGLISH} "Choose where to install Avidemux icons."
Var dlgInstallOptions
Var lblCreateIcons
Var chkDesktop
Var chkStartMenu
Var chkQuickLaunch

Function InstallOptionsPage
	Call IsInstallOptionsRequired
	!insertmacro MUI_HEADER_TEXT "$(INSTALL_OPTS_PAGE_TITLE)" "$(INSTALL_OPTS_PAGE_SUBTITLE)"

	nsDialogs::Create 1018
	Pop $dlgInstallOptions

	${If} $dlgInstallOptions == error
		Abort
	${EndIf}

	${NSD_CreateLabel} 0 0u 100% 12u "Create icons for Avidemux:"
	Pop $lblCreateIcons

	${NSD_CreateCheckBox} 0 18u 100% 12u "On my &Desktop"
	Pop $chkDesktop
	${NSD_SetState} $chkDesktop $CreateDesktopIcon
	${NSD_OnClick} $chkDesktop UpdateInstallOptions

	${NSD_CreateCheckBox} 0 36u 100% 12u "In my &Start Menu Programs folder"
	Pop $chkStartMenu
	${NSD_SetState} $chkStartMenu $CreateStartMenuGroup
	${NSD_OnClick} $chkStartMenu UpdateInstallOptions

	${If} ${AtMostWinVista}
		${NSD_CreateCheckBox} 0 54u 100% 12u "In my &Quick Launch bar"
		Pop $chkQuickLaunch
		${NSD_SetState} $chkQuickLaunch $CreateQuickLaunchIcon
		${NSD_OnClick} $chkQuickLaunch UpdateInstallOptions
	${EndIf}
  	#${If} ${IsWin2003}
		#WriteRegStr HKCU "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" "$INSTDIR\avidemux.exe" "WIN2000"
	#${EndIf}
  	#${If} ${IsWinXp}
		#WriteRegStr HKCU "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" "$INSTDIR\avidemux.exe" "WIN2000"
	#${EndIf}
	nsDialogs::Show
FunctionEnd

Function UpdateInstallOptions
	${NSD_GetState} $chkDesktop $CreateDesktopIcon
	${NSD_GetState} $chkStartMenu $CreateStartMenuGroup
	${NSD_GetState} $chkQuickLaunch $CreateQuickLaunchIcon
FunctionEnd

Function IsInstallOptionsRequired
Goto end
	!insertmacro SectionFlagIsSet ${SecUiQt} ${SF_SELECTED} end resetOptions
resetOptions:

    StrCpy $CreateDesktopIcon 0
    StrCpy $CreateStartMenuGroup 0
    StrCpy $CreateQuickLaunchIcon 0
    Abort

end:
FunctionEnd

Function IsStartMenuRequired
    StrCmp $CreateStartMenuGroup 1 +2
        Abort
FunctionEnd

Function ActivateInternalSections
    #AVS Proxy GUI shortcut:
    SectionGetFlags ${SecAvsProxy} $0
    IntOp $0 $0 & ${SF_SELECTED}
    IntOp $0 $0 & $CreateStartMenuGroup
    SectionSetFlags ${SecStartMenuAvsProxyGui} $0

    #Change Log shortcut:
    SectionSetFlags ${SecStartMenuChangeLog} $CreateStartMenuGroup

    #Qt shortcuts:
    SectionGetFlags ${SecUiQt} $0
    IntOp $0 $0 & ${SF_SELECTED}

    IntOp $1 $0 & $CreateDesktopIcon
    SectionSetFlags ${SecDesktopQt} $1

    IntOp $1 $0 & $CreateQuickLaunchIcon
    SectionSetFlags ${SecQuickLaunchQt} $1

    IntOp $1 $0 & $CreateStartMenuGroup
    SectionSetFlags ${SecStartMenuQt} $1
FunctionEnd

Function InstFilesPageShow
	${If} $ReinstallUninstall != ""
		Call RunUninstaller
		BringToFront
	${EndIf}
FunctionEnd

Function InstFilesPageLeave
	; Don't advance automatically if details expanded
	FindWindow $R0 "#32770" "" $HWNDPARENT
	GetDlgItem $R0 $R0 1016
	System::Call user32::IsWindowVisible(i$R0)i.s
	Pop $R0

	StrCmp $R0 0 +2
	SetAutoClose false
FunctionEnd

Function ConfigureFinishPage

    SectionGetFlags ${SecUiQt} $0
    IntOp $0 $0 & ${SF_SELECTED}
    StrCmp $0 ${SF_SELECTED} end

    DeleteINISec "$PLUGINSDIR\ioSpecial.ini" "Field 4"

end:
FunctionEnd

Function RunAvidemux
    SetOutPath $INSTDIR

    SectionGetFlags ${SecUiQt} $0
    IntOp $0 $0 & ${SF_SELECTED}

	!insertmacro UAC_AsUser_ExecShell "" "$INSTDIR\avidemux.exe" "" "" ""

    Goto end


end:
FunctionEnd

Var ReinstallUninstallButton

Function ReinstallPage
	${If} $PreviousVersion == ""
		Abort
	${EndIf}

	nsDialogs::Create /NOUNLOAD 1018
	Pop $0

	${If} $PreviousVersionState == 1
		!insertmacro MUI_HEADER_TEXT "Already Installed" "Choose how you want to install ${PRODUCT_FULLNAME}."
		nsDialogs::CreateItem /NOUNLOAD STATIC ${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 0 0 100% 40 "An older version of Avidemux is installed on your system.  Select the operation you want to perform and click Next to continue."
		Pop $R0
		nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_VCENTER}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS}|${WS_GROUP}|${WS_TABSTOP} 0 10 55 100% 30 "Upgrade Avidemux using previous settings (recommended)"
		Pop $ReinstallUninstallButton
		nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_TOP}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 10 85 100% 50 "Change settings (advanced)"
		Pop $R0

		${If} $ReinstallUninstall == ""
			StrCpy $ReinstallUninstall 1
		${EndIf}
	${ElseIf} $PreviousVersionState == 2
		!insertmacro MUI_HEADER_TEXT "Already Installed" "Choose how you want to install ${PRODUCT_FULLNAME}."
		nsDialogs::CreateItem /NOUNLOAD STATIC ${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 0 0 100% 40 "A newer version of Avidemux is already installed! It is not recommended that you downgrade to an older version. Select the operation you want to perform and click Next to continue."
		Pop $R0
		nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_VCENTER}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS}|${WS_GROUP}|${WS_TABSTOP} 0 10 55 100% 30 "Downgrade Avidemux using previous settings (recommended)"
		Pop $ReinstallUninstallButton
		nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_TOP}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 10 85 100% 50 "Change settings (advanced)"
		Pop $R0

		${If} $ReinstallUninstall == ""
			StrCpy $ReinstallUninstall 1
		${EndIf}
	${ElseIf} $PreviousVersionState == 0
		!insertmacro MUI_HEADER_TEXT "Already Installed" "Choose the maintenance option to perform."
		nsDialogs::CreateItem /NOUNLOAD STATIC ${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 0 0 100% 40 "${PRODUCT_FULLNAME} is already installed. Select the operation you want to perform and click Next to continue."
		Pop $R0
		nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_VCENTER}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS}|${WS_GROUP}|${WS_TABSTOP} 0 10 55 100% 30 "Add/Remove/Reinstall components"
		Pop $R0
		nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_TOP}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 10 85 100% 50 "Uninstall Avidemux"
		Pop $ReinstallUninstallButton

		${If} $ReinstallUninstall == ""
			StrCpy $ReinstallUninstall 2
		${EndIf}
	${Else}
		MessageBox MB_ICONSTOP "Unknown value of PreviousVersionState, aborting" /SD IDOK
		Abort
	${EndIf}

	${If} $ReinstallUninstall == "1"
		SendMessage $ReinstallUninstallButton ${BM_SETCHECK} 1 0
	${Else}
		SendMessage $R0 ${BM_SETCHECK} 1 0
	${EndIf}

	nsDialogs::Show
FunctionEnd

Function ReinstallPageLeave
	SendMessage $ReinstallUninstallButton ${BM_GETCHECK} 0 0 $R0

	${If} $R0 == 1
		; Option to uninstall old version selected
		StrCpy $ReinstallUninstall 1
	${Else}
		; Custom up/downgrade or add/remove/reinstall
		StrCpy $ReinstallUninstall 2
	${EndIf}

	${If} $ReinstallUninstall == 1
		${If} $PreviousVersionState == 0
			Call RunUninstaller
			Quit
		${Else}
			; Need to reload defaults. User could have
			; chosen custom, change something, went back and selected
			; the express option.
			Call LoadPreviousSettings
		${EndIf}
	${EndIf}
FunctionEnd


##########################
# Uninstaller functions
##########################
Function un.onInit
	SetShellVarContext all

UAC_Elevate:
	!insertmacro UAC_RunElevated
	StrCmp 1223 $0 UAC_ElevationAborted
	StrCmp 0 $0 0 UAC_Err
	StrCmp 1 $1 0 UAC_Success
	Quit

UAC_Err:
	MessageBox MB_ICONSTOP "Unable to elevate, error $0"
	Abort

UAC_ElevationAborted:
	Abort

UAC_Success:
	StrCmp 1 $3 +4
	StrCmp 3 $1 0 UAC_ElevationAborted
	MessageBox MB_ICONSTOP "This installer requires admin access."
	Goto UAC_Elevate
FunctionEnd

; TrimNewlines (copied from NSIS documentation)
; input, top of stack  (e.g. whatever$/r$/n)
; output, top of stack (replaces, with e.g. whatever)
; modifies no other variables.
Function un.TrimNewlines
	Exch $R0
	Push $R1
	Push $R2
	StrCpy $R1 0

loop:
	IntOp $R1 $R1 - 1
	StrCpy $R2 $R0 1 $R1
	StrCmp $R2 "$\r" loop
	StrCmp $R2 "$\n" loop
	IntOp $R1 $R1 + 1
	IntCmp $R1 0 no_trim_needed
	StrCpy $R0 $R0 $R1

no_trim_needed:
	Pop $R2
	Pop $R1
	Exch $R0
FunctionEnd
 
Function un.RemoveEmptyDirs
	Pop $9
	!define Index 'Line${__LINE__}'
	FindFirst $0 $1 "$INSTDIR$9*"
	StrCmp $0 "" "${Index}-End"
"${Index}-Loop:"
	StrCmp $1 "" "${Index}-End"
	StrCmp $1 "." "${Index}-Next"
	StrCmp $1 ".." "${Index}-Next"
	Push $0
	Push $1
	Push $9
	Push "$9$1\"
	Call un.RemoveEmptyDirs
	Pop $9
	Pop $1
	Pop $0
;"${Index}-Remove:"
	RMDir "$INSTDIR$9$1"
"${Index}-Next:"
	FindNext $0 $1
	Goto "${Index}-Loop"
"${Index}-End:"
	FindClose $0
	!undef Index
FunctionEnd

Function un.ConfirmPagePre
	${un.GetParameters} $R0
	${un.GetOptions} $R0 "/frominstall" $R1
	${Unless} ${Errors}
		Abort
	${EndUnless}
FunctionEnd

Function un.FinishPagePre
	${un.GetParameters} $R0
	${un.GetOptions} $R0 "/frominstall" $R1
	${Unless} ${Errors}
		SetRebootFlag false
		Abort
	${EndUnless}
FunctionEnd
