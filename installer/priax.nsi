;Written by William Oliveira (owilliam@gmail.com)
;feb 28, 2006

;controls the installer behaviour
!define PROJECT "Flizy"
!define PRIAX_DIR		"priax"		  				;determines the default installation directory
!define PRIAX_SETUP "prwiz.exe"					;this is the program responsible for PRIAX configuration
!define PRIAX_WINDOW_TITLE "Flizy v0.8.567" ;this is the window title, necessary to kill priax instances running
!define PRIAX_WINDOW_CLASS "SDL_app"    ;this is the window class name, necessary to kill priax instances running
!define PRIAX_INSTALL "instpriax.exe"	  ;the name of the install file
!define PRIAX_REG_BASE_KEY "Software\Principal"	;sets the company registry key
!define PRIAX_REG_PRODUCT_KEY ${REG_BASE_KEY}\${PROJECT} ;sets a registry key to store installation data

!define SHOW_WELCOME										;if defined: show welcome screen.
!define SHOW_LICENSE										;if defined: show license agreement (must have a license.txt file).
;!define SHOW_COMPONENTS								;if defined: user can select components to install (makes sense only if there are more than one).
;!define SHOW_CHOOSE_DIR								;if defined: user can choose target directory to install, otherwise use a fixed one.

!define CFG_CALL_PRIAXSETUP		 					;if defined: calls the config program after a successfull installation.
!define CFG_STARTUP_SHORTCUT						;if defined: puts a link to the application in the Start/Programs/Startup folder
!define CFG_DESKTOP_SHORTCUT						;if defined: puts a link to the application in the user's desktop
!define CFG_REMOVE_CONFIG_FILES 				;if defined: remove all config files.
!define CFG_USE_SYSTEM_DRIVE		  			;if defined: use the drive where windows is installed, else always use "c:\".
;!define CFG_INSTALL_FOR_ALL_USERS			;if defined: install for all users, otherwise installs for the current user only.
																				;be carefull, "all users" installations may require administrator previleges.

!define CHECK_EXTRA_DISK_SPACE 1024			;defines an extra disk space in kbytes required to do the installation (obviously beyond the room used to install all selected components)
!define CHECK_MINIMUM_RAM 128						;minimum CPU RAM requerid
!define CHECK_MINIMUM_CLOCK 600   			;minimum CPU clock required


;-----------------------------------------------------------------------------
!include "MUI.nsh"       ;the interface style chosen
!include "FileFunc.nsh"  ;to check disk space

;-----------------------------------------------------------------------------
;General
Name ${PROJECT}									;the project name
OutFile ${PRIAX_INSTALL}				;the target installer, this will be the name of the output program
;SetCompressor /solid lzma;			;use LZMA compression (available: lzma/zlib/bzip2)

;-----------------------------------------------------------------------------
;Variables
Var MUI_TEMP
Var STARTMENU_FOLDER
Var TARGETDRIVE

;-----------------------------------------------------------------------------
;Interface Settings
!define MUI_ABORTWARNING

XPStyle on
;-----------------------------------------------------------------------------
;Generic Macros
	!insertmacro DriveSpace

;-----------------------------------------------------------------------------
;Pages
!ifdef SHOW_WELCOME
	!insertmacro MUI_PAGE_WELCOME
!endif

!ifdef SHOW_LICENSE
	!insertmacro MUI_PAGE_LICENSE "License.txt"
!endif	

!ifdef SHOW_COMPONENTS
	!insertmacro MUI_PAGE_COMPONENTS
!endif	

!ifdef SHOW_CHOOSE_DIR
	!insertmacro MUI_PAGE_DIRECTORY
!endif

	;Start Menu Folder Page Configuration
!ifdef CFG_INSTALL_FOR_ALL_USERS
	!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKU"
!else
	!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
!endif

	!define MUI_STARTMENUPAGE_REGISTRY_KEY REG_PRODUCT_KEY
	!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

	!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
	!insertmacro MUI_PAGE_INSTFILES
	!insertmacro MUI_PAGE_FINISH

	;uninstall page
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;-----------------------------------------------------------------------------
;Languages
!insertmacro MUI_LANGUAGE "PortugueseBR"

;-----------------------------------------------------------------------------
;Installer Sections

;*****************************************************************************
;main, hidden section
Section "-main" SecUnique
	SetOutPath "$INSTDIR"

	;file list to install
	File "bin\priax.exe"
	File "bin\prwiz.exe"
	File "bin\sdl.dll"	
	File "bin\sdl_ttf.dll"
	
	;Store installation folder
!ifdef CFG_INSTALL_FOR_ALL_USERS
	WriteRegStr HKU REG_PRODUCT_KEY "" $INSTDIR
!else
	WriteRegStr HKCU REG_PRODUCT_KEY "" $INSTDIR
!endif

  ; write uninstall strings
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT}" "DisplayName" "${PROJECT} (Somente Remover)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT}" "UninstallString" "$INSTDIR\uninstall.exe"

	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
		;Create shortcuts
		CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER";
		CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Desinstalar.lnk" "$INSTDIR\Uninstall.exe" "" "" "" SW_SHOWNORMAL "" "Desinstalar PRIAX";
		CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Flizy.lnk" "$INSTDIR\priax.exe" "" "" "" SW_SHOWNORMAL "" "PRIAX";
		CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Configurador.lnk" "$INSTDIR\prwiz.exe" "" "" "" SW_SHOWNORMAL "" "Configurador do PRIAX";

!ifdef CFG_STARTUP_SHORTCUT
		CreateShortCut "$SMSTARTUP\priax.lnk" "$INSTDIR\priax.exe";
!endif

!ifdef CFG_DESKTOP_SHORTCUT
		CreateShortCut "$DESKTOP\priax.lnk" "$INSTDIR\priax.exe";
!endif	

	!insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;*****************************************************************************
Section "-sounds" SecSounds
	SetOutPath "$INSTDIR\sounds"
	;file list to install
	File "..\sounds\ring1.raw"
	File "..\sounds\ringback.raw"
SectionEnd

;*****************************************************************************
Section "-fonts" SecFonts
	SetOutPath "$INSTDIR\fonts"
	;file list to install
	File "..\fonts\arial.ttf"
	File "..\fonts\digital.ttf"
	File "..\fonts\logo.ttf"
	File "..\fonts\monospace.ttf"
SectionEnd

;*****************************************************************************
Section "-images" SecImages
	SetOutPath "$INSTDIR\images"
	;file list to install
	File "..\images\display.bmp"
	File "..\images\phone-mask.bmp"
	File "..\images\priax-32x32.bmp"
	File "..\images\priax-72x72.bmp"
	File "..\images\priax.ico"
SectionEnd

;-----------------------------------------------------------------------------
;Descriptions
	;Language strings
	LangString DESC_SecUnique ${LANG_PORTUGUESEBR} "Componentes Principais do PRIAX, \
					essenciais para o funcionamento do produto.";

	LangString DESC_SecFonts ${LANG_PORTUGUESEBR} "Tipos de Letras True Type/Open Type";

	LangString DESC_SecImages ${LANG_PORTUGUESEBR} "Imagens Utilizadas";

	;Assign language strings to sections
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
		!insertmacro MUI_DESCRIPTION_TEXT ${SecUnique} $(DESC_SecUnique)
		!insertmacro MUI_DESCRIPTION_TEXT ${SecImages} $(DESC_SecImages)
		!insertmacro MUI_DESCRIPTION_TEXT ${SecFonts} $(DESC_SecFonts)		
	!insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;-----------------------------------------------------------------------------
;Uninstaller Section
Section "Uninstall"

	;removing registry entries	
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT}"
	
	;removing installed files
	Delete "$INSTDIR\sdl.dll"	
	Delete "$INSTDIR\sdl_ttf.dll"	
	Delete "$INSTDIR\Uninstall.exe"
	Delete "$INSTDIR\priax.exe"
	Delete "$INSTDIR\prwiz.exe"
	
	;removing images
	Delete "$INSTDIR\images\display.bmp"
	Delete "$INSTDIR\images\phone-mask.bmp"
	Delete "$INSTDIR\images\priax-32x32.bmp"
	Delete "$INSTDIR\images\priax-72x72.bmp"
	Delete "$INSTDIR\images\priax.ico"

  ;removing fonts
	Delete "$INSTDIR\fonts\arial.ttf"
	Delete "$INSTDIR\fonts\digital.ttf"
	Delete "$INSTDIR\fonts\logo.ttf"
	Delete "$INSTDIR\fonts\monospace.ttf"
	
	;removing sounds
	Delete "$INSTDIR\sounds\ring1.raw"
	Delete "$INSTDIR\sounds\ringback.raw"

  ;removing config files
!ifdef CFG_REMOVE_CONFIG_FILES
  Delete "$INSTDIR\priax.conf"
	Delete "$INSTDIR\priax-calls.dat"
!endif

  ;removing temporary and log files
	Delete "$INSTDIR\stderr.txt"
	Delete "$INSTDIR\stdout.txt"

	RmDir "$INSTDIR\sounds"	
	RmDir "$INSTDIR\images"
	RmDir "$INSTDIR\fonts"
	RMDir "$INSTDIR"

  ;removing startup link
	Delete "$SMSTARTUP\priax.lnk"

	!insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

  ;removing start menu links
	Delete "$SMPROGRAMS\$MUI_TEMP\Desinstalar.lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\Flizy.lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\Configurador.lnk"
	Delete "$DESKTOP\priax.lnk"

	;Delete empty start menu parent diretories
	StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"

  ;removing empty directory entries in start menu
	startMenuDeleteLoop:
		ClearErrors
		RMDir $MUI_TEMP
		GetFullPathName $MUI_TEMP "$MUI_TEMP\.."

		IfErrors startMenuDeleteLoopDone
		StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
		
		startMenuDeleteLoopDone:
		DeleteRegKey /ifempty HKCU PRIAX_REG_PRODUCT_KEY
		DeleteRegKey /ifempty HKCU PRIAX_REG_BASE_KEY
		
SectionEnd

;-----------------------------------------------------------------------------
;functions

;*****************************************************************************
;before anything
Function .onInit
!ifdef CFG_INSTALL_FOR_ALL_USERS
	SetShellVarContext All
!else
	SetShellVarContext Current
!endif

!ifdef CFG_USE_SYSTEM_DRIVE
  StrCpy $TARGETDRIVE $WINDIR 3
!else
  StrCpy $TARGETDRIVE "C:\"
!endif

  StrCpy $INSTDIR "$TARGETDRIVE${PRIAX_DIR}"
  StrCpy $STARTMENU_FOLDER ${PROJECT}
  
  call CheckRunningInstances
  
  call CheckPriorInstallation

  call CheckDiskSpaceAvailability
  
  call CheckHardwareRequeriments
 
FunctionEnd

;*****************************************************************************
Function un.onInit
!ifdef CFG_INSTALL_FOR_ALL_USERS
	SetShellVarContext All
!else
	SetShellVarContext Current
!endif
FunctionEnd


;*****************************************************************************
;after all
Function .onInstSuccess
  push $0
	!ifdef CFG_CALL_PRIAXSETUP
		;executes the priax configurator
		;ExecWait "$INSTDIR\${PRIAX_SETUP}" $0
		Exec "$INSTDIR\${PRIAX_SETUP}"
		;IntCmp $0 0 +2
		  ;MessageBox MB_OK|MB_ICONSTOP "O Configurador não foi executado com sucesso."
	!endif
	pop $0
FunctionEnd

;*****************************************************************************
;verifies if this product was already installed (maybe another version)
;todo: check current and older version.
Function CheckPriorInstallation
	IfFileExists $INSTDIR\*.* 0 +2
		MessageBox MB_YESNO|MB_ICONQUESTION "Encontrada uma instalação anterior do PRIAX no diretório.$\n$INSTDIR$\nDeseja fazer uma nova Instalação em cima da Instalação Antiga?" IDYES true IDNO false
	true:
		return
	false:
		MessageBox MB_OK|MB_ICONSTOP "A instalação será encerrada.$\nSe desejar tentar de outra vez, execute o instalador novamente."
		abort
FunctionEnd

;*****************************************************************************
;verifies if there is room enough to make a safe install.
;The installation will be aborted if there is not enough space.
Function CheckDiskSpaceAvailability
  push $R0
  push $R1
  
  ${DriveSpace} $TARGETDRIVE "/D=F /S=K" $R0
  ;get size from section UNIQUE
  SectionGetSize 0 $R1
  ;adds the extra size
  IntOp $R1 $R1 + ${CHECK_EXTRA_DISK_SPACE}

	IntCmp $R1 $R0 +3 +3 0
    MessageBox MB_OK|MB_ICONSTOP "Não há espaço suficiente para instalar o PRIAX no diretório $\n$\n$INSTDIR$\n$\nLibere espaço no drive indicado e tente novamente.$\n$\nEspaço Necessário: $R1 K"
    abort
  pop $R1
  pop $R0
FunctionEnd

;*****************************************************************************
;verifies if there is room enough to make a safe install.
;The installation will be aborted if there is not enough space.
Function CheckHardwareRequeriments
	;- Processador de 1GHz
	;- 256MB de memória RAM
	;- 10MB de espaço livre em disco
	;- Placa de som Full Duplex (se possível detectar)
	push $0
	push $1
	push $2

  SetOutPath $TEMP\priax                ; create temp directory
  File "bin\pricheck.dll"               ; copy dll there
  
  ;--------Sound Board Check
  StrCpy $0 ${NSIS_MAX_STRLEN}          ; assign memory to $0
  push ${NSIS_MAX_STRLEN}
  pop $1
  ;DWORD CheckCompatibleSoundBoard(char *BoardName, DWORD BoardNameLength), returns 0 on success
  ;r0=string, r1=string size (to inform dll) r2=result
  System::Call 'pricheck::CheckCompatibleSoundBoard(t, i) i(.r0, r1).r2'
  IntCmp $2 0 +3
	  MessageBox MB_OK|MB_ICONSTOP "($2)$\nNão foi possível identificar uma placa de som compatível com o PRIAX.$\nA instalação será cancelada."
	  abort

  DetailPrint "Placa de Som encontrada: $0"

  ;--------CPU Check
  ;DWORD CheckCPUClock(DWORD *Clock), returns 0 on success
  ;r0=current CPU clock, r1=result, 0 = SUCCESS
  System::Call 'pricheck::CheckCPUClock(*i) i(.r0).r1'
  IntCmp $1 0 +3
	  MessageBox MB_OK|MB_ICONSTOP "($1)$\nNão foi possível obter informações da CPU.$\nVerifique se você possui direitos para executar a instalação.$\nA instalação será cancelada."
	  abort
	
	IntCmp $0 ${CHECK_MINIMUM_CLOCK} +3 0 +3
	  MessageBox MB_OK|MB_ICONSTOP "(CPU Clock=$0)$\nRequisitos de hardware insuficientes para o funcionamento do PRIAX$\nA instalação será cancelada."
	  abort
	  
  ;DetailPrint "CPU Clock OK: $0"	  
  ;MessageBox MB_OK "CPU Clock OK: $0"

  ;--------RAM amount Check
	;DWORD CheckRAM(DWORD *MegaRAM)
  ;r0=RAM amount, r1=result, 0 = SUCCESS	
  System::Call 'pricheck::CheckRAM(*i) i(.r0).r1'
  IntCmp $1 0 +3
	  MessageBox MB_OK|MB_ICONSTOP "($1)$\nNão foi possível obter informações da quantidade de RAM instalada.$\nVerifique se você possui direitos para executar a instalação.$\nA instalação será cancelada."
	  abort
	
	IntCmp $0 ${CHECK_MINIMUM_RAM} +3 0 +3
	  MessageBox MB_OK|MB_ICONSTOP "(RAM=$0)$\nRequisitos de hardware insuficientes para o funcionamento do PRIAX$\nA instalação será cancelada."
	  abort
  
  ;DetailPrint "RAM OK: $0"
  ;MessageBox MB_OK "RAM OK: $0"
	
  ;cleanup
  ;last plug-in call must not have /NOUNLOAD so NSIS will be able to delete
  ;the temporary DLL
  SetPluginUnload manual

  ;do nothing (but let the installer unload the System dll)
  System::Free 0

	pop $2 
	pop $1 
	pop $0 

FunctionEnd

;*****************************************************************************
;verifies if there's a PRIAX instance already running
Function CheckRunningInstances
	push $0
	push $1
	
RECHECK:
		FindWindow $0 "${PRIAX_WINDOW_CLASS}" "${PRIAX_WINDOW_TITLE}"
    IntCmp $0 0 +7
		MessageBox MB_YESNO "Existe uma instância do PRIAX em execução. Deseja Finalizar o Programa?" IDYES +3 IDNO 0
    MessageBox MB_OK "Não é possível fazer a instalação enquanto o PRIAX estiver em execução. Feche o programa e tente novamente mais tarde."
    abort
    SendMessage $0 0xbfff 0 0
    Sleep 2000
    goto RECHECK
	pop $1
	pop $0
FunctionEnd
