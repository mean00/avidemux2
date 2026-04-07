# Use Unicode for modern Windows compatibility
Unicode True

!include "LogicLib.nsh"

# --- Basic Config ---
Name "Avidemux 64-bit"
OutFile "Avidemux_64bit_Setup.exe"
# This forces the "shield" icon and UAC prompt
RequestExecutionLevel admin 
# Points to C:\Program Files instead of C:\Program Files (x86)
InstallDir "$PROGRAMFILES64\Avidemux64"

# --- Visual Pages ---
Page directory
Page instfiles

# --- Installation Section ---
Section "Install"
    # Tell NSIS we are dealing with a 64-bit system/registry
    SetRegView 64

    # Define where files go
    SetOutPath "$INSTDIR"
    
    # Copy files from your local 'avidemux64' folder
    # IMPORTANT: Ensure this folder is in the same directory as this .nsi file
    File /r "../../avidemux64/*.*"

    # Create the uninstaller binary
    WriteUninstaller "$INSTDIR\uninstall.exe"

    # Shortcuts
    CreateDirectory "$SMPROGRAMS\Avidemux64"
    CreateShortcut "$SMPROGRAMS\Avidemux64\Avidemux.lnk" "$INSTDIR\avidemux_portable.exe"
    CreateShortcut "$SMPROGRAMS\Avidemux64\Uninstall.lnk" "$INSTDIR\uninstall.exe"
    CreateShortcut "$DESKTOP\Avidemux.lnk" "$INSTDIR\avidemux_portable.exe"

    # Add to "Apps & Features" (Add/Remove Programs) for 64-bit
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Avidemux64" \
                 "DisplayName" "Avidemux 64-bit"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Avidemux64" \
                 "DisplayIcon" "$INSTDIR\avidemux.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Avidemux64" \
                 "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Avidemux64" \
                 "Publisher" "Your Name or Organization"
SectionEnd

# --- Uninstallation Section ---
Section "Uninstall"
    # Switch to 64-bit mode to find the correct registry keys and folders
    SetRegView 64

    # Remove all files and the directory
    Delete "$INSTDIR\*.*"
    RMDir /r "$INSTDIR"

    # Remove Start Menu and Desktop items
    Delete "$SMPROGRAMS\Avidemux64\Avidemux.lnk"
    Delete "$SMPROGRAMS\Avidemux64\Uninstall.lnk"
    RMDir "$SMPROGRAMS\Avidemux64"
    Delete "$DESKTOP\Avidemux.lnk"

    # Clean up registry
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Avidemux64"
SectionEnd

# --- Admin Check ---
Function .onInit
    UserInfo::GetAccountType
    Pop $0
    ${If} $0 != "Admin"
        MessageBox MB_ICONSTOP "Admin rights required! Please right-click and 'Run as Administrator'."
        Quit
    ${EndIf}
FunctionEnd

