set(LEDGER_APP_VERSION "0.1.0")

if (WIN32)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/install/ledger/dist/win32/loader.exe ${CMAKE_BINARY_DIR}/bin/loader.exe COPYONLY)
	set(LEDGER_APP_INSTALLER loader.exe)
elseif (APPLE)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/install/ledger/dist/darwin/loader ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}.app/Contents/MacOS/loader COPYONLY)
	set(LEDGER_APP_INSTALLER loader)
else ()
	set(LEDGER_APP_INSTALLER miledger_loader)
	if (APPIMAGE)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/install/ledger/dist/linux/loader ${CMAKE_BINARY_DIR}/AppDir/usr/bin/${LEDGER_APP_INSTALLER} COPYONLY)
	else ()
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/install/ledger/dist/linux/loader ${CMAKE_BINARY_DIR}/bin/${LEDGER_APP_INSTALLER} COPYONLY)
	endif ()


endif ()
