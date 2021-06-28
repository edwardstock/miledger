cmake_minimum_required(VERSION 3.18)

project(miledger
        LANGUAGES CXX
        VERSION 0.1.1
        DESCRIPTION "Minter desktop console"
        )

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)


include(ConanInit)
add_conan_remote(minter https://minter.jfrog.io/artifactory/api/conan/conan)
add_conan_remote(edwardstock https://edwardstock.jfrog.io/artifactory/api/conan/conan)
conan_init()

list(APPEND CMAKE_MODULE_PATH "${CONAN_QT_ROOT}/lib/cmake/Qt5Widgets")
list(APPEND CMAKE_PREFIX_PATH "${CONAN_QT_ROOT}/lib/cmake")


include(Qt5WidgetsMacros)
include(compilers_flags)
include(penv_generator)
include(CheckIncludeFileCXX)

check_include_file_cxx("optional" HAVE_CXX_OPTIONAL)

include(${CONAN_QT_ROOT}/lib/cmake/Qt5Core/conan_qt_executables_variables.cmake)

set(QT_CONF_PREFIX "${CONAN_QT_ROOT}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cfg/qt.in.conf ${CMAKE_BINARY_DIR}/bin/qt.conf)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)


#find_package(QT NAMES Qt5 Qt6 HINTS ${CONAN_QT_ROOT} COMPONENTS Core Widgets Network Concurrent REQUIRED)
#find_package(Qt${QT_VERSION_MAJOR} HINTS ${CONAN_QT_ROOT} COMPONENTS Core Widgets Network Concurrent REQUIRED)

#message(STATUS "Qt${QT_VERSION_MAJOR} Dir: ${Qt${QT_VERSION_MAJOR}_DIR}")


if (WIN32)
	set(CMD_BIN "cmd.exe")
	set(CMD_BIN_ARG "/c")
else ()
	set(CMD_BIN "bash")
	set(CMD_BIN_ARG "-c")
endif ()

set(LEDGER_APP_VERSION "0.1.0")

if (APPLE)
	set(MILEDGER_APPLE 1)
	set(CMAKE_MACOSX_BUNDLE ON)
endif ()
if (UNIX AND NOT APPLE)
	set(MILEDGER_LINUX 1)
endif ()

add_definitions(-DCPR_CURL_NOSIGNAL=1)

option(MINTER_TESTNET "Build app for testnet environment" Off)

if (MINTER_TESTNET)
	set(MINTER_TESTNET_NAME "testnet")
	set(MINTER_CHAIN_ID "minter::chain_id::testnet")

	set(MINTER_DEFAULT_COIN "MNT")
	set(MINTER_EXPLORER_API_V "v2")
	set(MINTER_GATE_API_V "v2")
	set(MINTER_EXPLORER_API "https://explorer-api.${MINTER_TESTNET_NAME}.minter.network/api/${MINTER_EXPLORER_API_V}/")
	set(MINTER_EXPLORER_FRONT "https://explorer.${MINTER_TESTNET_NAME}.minter.network")
	set(MINTER_GATE_API "https://gate-api.${MINTER_TESTNET_NAME}.minter.network/api/${MINTER_GATE_API_V}/")
	set(MINTER_WS_URL "wss://explorer-rtm.${MINTER_TESTNET_NAME}.minter.network/connection/websocket")
	set(MINTER_COIN_AVATAR_URL "https://my.apps.minter.network/api/v1/avatar/by/coin/")
	set(MINTER_ADDRESS_AVATAR_URL "https://my.apps.minter.network/api/v1/avatar/by/address/")
	set(MINTER_EXPLORER_STATIC_URL "https://explorer-static.${MINTER_TESTNET_NAME}.minter.network")
else ()
	set(MINTER_CHAIN_ID "minter::chain_id::mainnet")

	set(MINTER_EXPLORER_API_V "v2")
	set(MINTER_GATE_API_V "v2")
	set(MINTER_DEFAULT_COIN "BIP")
	set(MINTER_EXPLORER_API "https://explorer-api.apps.minter.network/api/${MINTER_EXPLORER_API_V}/")
	set(MINTER_EXPLORER_FRONT "https://explorer.minter.network")
	set(MINTER_GATE_API "https://gate-api.apps.minter.network/api/${MINTER_GATE_API_V}/")
	set(MINTER_WS_URL "wss://explorer-rtm.apps.minter.network/connection/websocket")
	set(MINTER_COIN_AVATAR_URL "https://my.apps.minter.network/api/v1/avatar/by/coin/")
	set(MINTER_ADDRESS_AVATAR_URL "https://my.apps.minter.network/api/v1/avatar/by/address/")
	set(MINTER_EXPLORER_STATIC_URL "https://explorer-static.minter.network")
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cfg/miledger-config.in.h ${CMAKE_CURRENT_SOURCE_DIR}/include/miledger-config.h)
if (APPLE)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cfg/Info.plist ${CMAKE_BINARY_DIR}/Info.plist)
endif ()
if (WIN32)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cfg/version.in.h ${CMAKE_CURRENT_SOURCE_DIR}/version.h)
endif ()


set(QT_UIC_EXECUTABLE "${CONAN_BIN_DIRS_QT}/uic")
set(QT_MOC_EXECUTABLE "${CONAN_BIN_DIRS_QT}/moc")

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")

set(UI_RESOURCES
    base.qrc
    )

set(UI_SOURCES
    ui/mainwindow.ui
    ui/installappwindow.ui
    ui/consolewindow.ui
    ui/txsenddialog.ui
    ui/testwindow.ui
    )

qt5_wrap_ui(UI_HEADERS ${UI_SOURCES})
message(STATUS "UI Sources: ${UI_HEADERS}")
message(STATUS "UI Sources: ${UI_SOURCES}")

set(PROJECT_SOURCES
    ${UI_HEADERS}
    ${UI_RESOURCES}

    include/mainwindow.h
    include/installappwindow.h
    include/device_looper.h
    include/main_app.h
    include/miledger-config.h
    main.cpp
    src/mainwindow.cpp
    src/installappwindow.cpp
    src/device_looper.cpp
    src/main_app.cpp


    include/consolewindow.h
    src/consolewindow.cpp
    include/tab_send.h
    src/tab_send.cpp
    include/tab_base.h
    include/net/repository.h

    include/net/io_container.h
    include/net/request.h
    src/io_container.cpp
    src/request.cpp
    src/explorer_repo.cpp
    include/net/explorer_repo.h
    include/net/gate_repo.h
    src/gate_repo.cpp
    include/console_app.h
    src/console_app.cpp
    include/input_group.h
    include/validators.hpp
    include/input_fields.hpp
    include/tab_exchange.h
    src/tab_exchange.cpp
    include/exchange_calculator.h
    include/exchange_forms.h
    src/exchange_forms.cpp
    include/coin_model.h

    include/txsenddialog.h
    src/txsenddialog.cpp


    include/testwindow.h
    src/testwindow.cpp

    include/utils.h
    src/utils.cpp

    include/errors.h
    include/image_cache.h
    include/rxqt_instance.hpp
    include/style_helper.h
    include/settings.h
    src/settings.cpp
    include/optional.hpp)

if (WIN32)
	set(PROJECT_SOURCES
	    ${PROJECT_SOURCES}
	    ${APP_ICON_RESOURCE_WINDOWS}
	    )
elseif (APPLE)
	set(MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources/apple.icns)
	set(APP_ICON_MACOSX ${CMAKE_CURRENT_SOURCE_DIR}/resources/apple.icns)
	set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES
	                            MACOSX_PACKAGE_LOCATION "Resources")

	set(APP_LEDGER_LOADER ${CMAKE_BINARY_DIR}/loader)
	set_source_files_properties(${APP_LEDGER_LOADER} PROPERTIES
	                            MACOSX_PACKAGE_LOCATION "MacOS")


	set(PROJECT_SOURCES
	    ${PROJECT_SOURCES}
	    ${APP_ICON_MACOSX}
	    )
endif ()

if (WIN32 AND PACKAGING)
	add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})
else ()
	#if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

endif ()


if (ENABLE_SANITIZE)
	target_link_libraries(${PROJECT_NAME} PRIVATE asan)
	target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address -m64 -pipe -ffunction-sections -fdata-sections -O0 -g -fno-omit-frame-pointer)
endif ()

set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE CACHE BOOL "" FORCE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE CACHE BOOL "" FORCE)


if (APPLE)

	if (PACKAGING)
		set_target_properties(${PROJECT_NAME} PROPERTIES
		                      LINKER_LANGUAGE CXX
		                      LINK_FLAGS "-Wl,-rpath,@executable_path/../Frameworks"
		                      MACOSX_RPATH TRUE
		                      SKIP_BUILD_RPATH FALSE
		                      BUILD_WITH_INSTALL_RPATH FALSE
		                      INSTALL_RPATH_USE_LINK_PATH FALSE
		                      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/Info.plist"
		                      OUTPUT_NAME "MiLedger"
		                      )
	else ()
		set_target_properties(${PROJECT_NAME} PROPERTIES
		                      LINKER_LANGUAGE CXX
		                      LINK_FLAGS "-Wl,-rpath,${QT_DIR}/../../../lib/"
		                      MACOSX_RPATH TRUE
		                      SKIP_BUILD_RPATH FALSE
		                      BUILD_WITH_INSTALL_RPATH FALSE
		                      INSTALL_RPATH_USE_LINK_PATH FALSE
		                      INSTALL_NAME_DIR "${QT_DIR}/../../../lib/"
		                      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/Info.plist"
		                      )
	endif ()
endif ()

#target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
#target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Network)
#target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Concurrent)

#target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
#target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
#target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Network)
#target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Concurrent)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CONAN_QT_ROOT}/bin/archdatadir/plugins/platforms")
#target_link_libraries(${PROJECT_NAME} PRIVATE qxcb)

target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::mhwallet)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::minter_tx)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::minter_api)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::toolbox)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::rxcpp)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::cpr)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::libcurl)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::libpng)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::qt)

#include(material_widgets)
#target_link_libraries(${PROJECT_NAME} PRIVATE qt_material)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/rxqt/include)

if (PACKAGING)
	if (APPLE)
		find_program(MACDEPLOYQT_EXECUTABLE macdeployqt)
		if (MACDEPLOYQT_EXECUTABLE)
			message(STATUS "Found qt mac deploy: ${MACDEPLOYQT_EXECUTABLE}")
			add_custom_command(
				TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND "${MACDEPLOYQT_EXECUTABLE}" "${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}.app" "-dmg" "-always-overwrite"
				COMMENT "Running macdeployqt..."
			)
			#-always-overwrite
		endif ()
	elseif (WIN32)
		find_program(WINDEPLOYQT_EXECUTABLE windeployqt)
		if (WINDEPLOYQT_EXECUTABLE)
			message(STATUS "Found qt win deploy: ${WINDEPLOYQT_EXECUTABLE}")
			add_custom_command(
				TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND "${WINDEPLOYQT_EXECUTABLE}" "${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}.exe" "--no-virtualkeyboard"
				COMMENT "Running windeployqt..."
			)
		endif ()
	else ()
		set(APPIMAGE_ROOT ${CMAKE_BINARY_DIR}/AppDir)
		find_program(LINUXDEPLOYQT_EXECUTABLE linuxdeployqt NAMES linuxdeployqt.AppImage linuxdeployqt-continuous-x86_64.AppImage linuxdeployqt-7-x86_64.AppImage)

		if (NOT APPIMAGE OR APPIMAGE STREQUAL "nofuse")
			configure_file(
				${CMAKE_CURRENT_SOURCE_DIR}/cfg/linux_deploy_qt_nofuse.sh
				${CMAKE_BINARY_DIR}/linux_deploy_qt.sh
				FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
				@ONLY)
		else ()
			configure_file(
				${CMAKE_CURRENT_SOURCE_DIR}/cfg/linux_deploy_qt.sh
				${CMAKE_BINARY_DIR}/linux_deploy_qt.sh
				FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
				@ONLY)
		endif ()


		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cfg/desktop.ini ${APPIMAGE_ROOT}/usr/share/applications/${PROJECT_NAME}.desktop)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cfg/desktop.ini ${APPIMAGE_ROOT}/${PROJECT_NAME}.desktop)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/icon-512.png ${APPIMAGE_ROOT}/usr/share/icons/hicolor/512x512/apps/${PROJECT_NAME}.png COPYONLY)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/icon-256.png ${APPIMAGE_ROOT}/usr/share/icons/hicolor/256x256/apps/${PROJECT_NAME}.png COPYONLY)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/icon-256.png ${APPIMAGE_ROOT}/${PROJECT_NAME}.png COPYONLY)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/icon-128.png ${APPIMAGE_ROOT}/usr/share/icons/hicolor/128x128/apps/${PROJECT_NAME}.png COPYONLY)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/icon-64.png ${APPIMAGE_ROOT}/usr/share/icons/hicolor/64x64/apps/${PROJECT_NAME}.png COPYONLY)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/icon-32.png ${APPIMAGE_ROOT}/usr/share/icons/hicolor/32x32/apps/${PROJECT_NAME}.png COPYONLY)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/icon-16.png ${APPIMAGE_ROOT}/usr/share/icons/hicolor/16x16/apps/${PROJECT_NAME}.png COPYONLY)

		#		include(install)
		if (LINUXDEPLOYQT_EXECUTABLE)
			message(STATUS "Found qt linux deploy: ${LINUXDEPLOYQT_EXECUTABLE}")
			add_custom_command(
				TARGET ${PROJECT_NAME}
				POST_BUILD
				COMMAND "${CMAKE_BINARY_DIR}/linux_deploy_qt.sh"
				COMMENT "Running linuxdeployqt..."
			)
			#								add_custom_command(
			#									TARGET ${PROJECT_NAME}
			#									POST_BUILD
			#									WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
			#									COMMAND ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${QTDIR}/lib"
			#									"${LINUXDEPLOYQT_EXECUTABLE}"
			#									"${APPIMAGE_ROOT}/usr/bin/${PROJECT_NAME}"
			#									"-unsupported-allow-new-glibc"
			#									"-appimage"
			#									"-bundle-non-qt-libs"
			#									"-always-overwrite"
			#									"-bundle-non-qt-libs"
			#									"-extra-plugins=iconengines,platforms,imageformats,platformthemes"
			#									"-verbose=3"
			#
			#									COMMENT "Running linuxdeployqt..."
			#								)
			# linuxdeployqt AppDir/usr/share/applications/miledger.desktop -always-overwrite -unsupported-allow-new-glibc -appimage -bundle-non-qt-libs -extra-plugins=iconengines,platformthemes/libqgtk3.so
			# VERSION=0.1.0 LD_LIBRARY_PATH=/home/edward/qt/lib:$LD_LIBRARY_PATH linuxdeployqt AppDir/usr/bin/miledger -always-overwrite -unsupported-allow-new-glibc -appimage -bundle-non-qt-libs -extra-plugins=iconengines,platforms,imageformats,platformthemes -verbose=3
		else ()
			message(FATAL_ERROR "linuxdeployqt NOT FOUND. Please download it https://github.com/probonopd/linuxdeployqt/releases")
		endif ()

		#-unsupported-allow-new-glibc
	endif ()
endif ()

#get_cmake_property(_variableNames VARIABLES)
#list(SORT _variableNames)
#foreach (_variableName ${_variableNames})
#		message(STATUS "VAR: ${_variableName}=${${_variableName}}")
#endforeach ()

