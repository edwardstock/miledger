cmake_minimum_required(VERSION 3.5)

project(miledger
        LANGUAGES CXX
        VERSION 0.1.0
        DESCRIPTION "Minter desktop console"
        )
#-DMINTER_TESTNET=On
#-DCMAKE_PREFIX_PATH=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/
#-DQT_DIR=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6
#-DQT_QMAKE_EXECUTABLE=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/bin/qmake.exe
#-DQt6_DIR=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6
#-DQt6Widgets_DIR=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6Widgets
#-DQt6WidgetsTools_DIR=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6WidgetsTools
#-DQt6Concurrent_DIR=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6Concurrent
#-DQt6Gui_DIR=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6Gui
#-DQt6GuiTools_DIR=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6GuiTools
#-DQt6EntryPoint_DIR=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6EntryPoint
#-DQt6Core_DIR=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6Core
#-DQt6CoreTools_DIR=/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6CoreTools

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)

include(ConanInit)
add_conan_remote(minter https://minter.jfrog.io/artifactory/api/conan/conan)
add_conan_remote(edwardstock https://edwardstock.jfrog.io/artifactory/api/conan/conan)
conan_init()

include(penv_generator)
include(compilers_flags)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Qt6_USE_STATIC_LIBS ON)
set(Qt6_USE_STATIC_RUNTIME ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Network Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network Concurrent REQUIRED)


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

qt_wrap_ui(UI_HEADERS ${UI_SOURCES})

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
    )

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

if (APPLE)
	#	set(CMAKE_MACOSX_RPATH ON)
	#/Users/edward/Qt/6.1.0/clang_64/lib/cmake/Qt6
	#	set(CMAKE_INSTALL_NAME_DIR ${QT_DIR}/../../../bin)
	#	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
	#	set(CMAKE_INSTALL_RPATH "${QT_DIR}/../../../lib" CACHE STRING "" FORCE)
	#	set(CMAKE_BUILD_RPATH "${QT_DIR}/../../../lib" CACHE STRING "" FORCE)
endif (APPLE)

qt_add_executable(${PROJECT_NAME}
                  ${PROJECT_SOURCES}
                  )

if (ENABLE_SANITIZE)
	target_link_libraries(${PROJECT_NAME} PRIVATE asan)
	target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address -m64 -pipe -ffunction-sections -fdata-sections -O0 -g -fno-omit-frame-pointer)
endif ()

set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE CACHE BOOL "" FORCE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE CACHE BOOL "" FORCE)


if (APPLE)

	if (PACKAGING)
		#		set(CMAKE_MACOSX_RPATH "../Frameworks")
		#		set(CMAKE_INSTALL_RPATH "../Frameworks")
		#		set(CMAKE_BUILD_RPATH  "../Frameworks")
		#		set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE CACHE BOOL "" FORCE)
		#		set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE CACHE BOOL "" FORCE)
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

#if (UNIX AND NOT APPLE)
#	find_library(UDEV_LIB udev NAME libudev.a libudev.so udev)
#	if (UDEV_LIB-NOT_FOUND)
#		message(FATAL_ERROR "libudev not found!")
#	endif ()
#
#	find_package(LibUsb 1.0 REQUIRED)
#
#	message(STATUS "libudev: ${UDEV_LIB}")
#	message(STATUS "libusb: ${LibUSB_LIBRARIES} ${LibUsb_INCLUDE_DIRS}")
#	target_include_directories(${PROJECT_NAME} PUBLIC ${LibUsb_INCLUDE_DIRS})
#
#	target_link_libraries(${PROJECT_NAME} PRIVATE ${UDEV_LIB})
#	target_link_libraries(${PROJECT_NAME} PRIVATE ${LibUSB_LIBRARIES})
#
#elseif (APPLE)
#	find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
#	find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
#	find_library(USB_LIB usb REQUIRED NAMES libusb-1.0.a libusb-1.0-dylib libusb-1.0.0.dylib)
#	message(STATUS "libusb: ${USB_LIB}")
#	message(STATUS "Framework IOKit: ${IOKIT_FRAMEWORK}")
#	message(STATUS "Framework CoreFoundation: ${CORE_FOUNDATION_FRAMEWORK}")
#	target_link_libraries(${PROJECT_NAME} PRIVATE ${USB_LIB})
#	target_link_libraries(${PROJECT_NAME} PRIVATE ${IOKIT_FRAMEWORK})
#	target_link_libraries(${PROJECT_NAME} PRIVATE ${CORE_FOUNDATION_FRAMEWORK})
#elseif (MSVC)
#	#		find_library(MS_HID_LIB hid NAMES hid.lib hid.dll)
#	#		message(STATUS "WIN32      hid: ${MS_HID_LIB}")
#	#
#	#		find_library(MS_KERNEL32_LIB kernel32 NAMES kernel32.lib kernel32.dll)
#	#		message(STATUS "WIN32 kernel32: ${MS_KERNEL32_LIB}")
#	#
#	#		find_library(MS_COMDL32_LIB comdlg32 NAMES comdlg32.lib comdlg32.dll)
#	#		message(STATUS "WIN32 comdlg32: ${MS_COMDL32_LIB}")
#	#
#	#		find_library(MS_SETUPAPI_LIB setupapi NAMES setupapi.lib setupapi.dll)
#	#		message(STATUS "WIN32 setupapi: ${MS_SETUPAPI_LIB}")
#	#
#	#		target_link_libraries(${PROJECT_NAME} PRIVATE ${MS_HID_LIB} ${MS_KERNEL32_LIB} ${MS_COMDL32_LIB})
#	#		target_link_libraries(${PROJECT_NAME} PRIVATE ${MS_HID_LIB} ${MS_SETUPAPI_LIB})
#	include(FindWinHID)
#	target_link_libraries(${PROJECT_NAME} PRIVATE ${WINHID_LIBRARY} ${WINHID_SETUPAPI_LIBRARY})
#	target_include_directories(${PROJECT_NAME} PRIVATE ${WINHID_INCLUDE_DIR})
#	target_include_directories(${PROJECT_NAME} PRIVATE ${WINHID_CRT_INCLUDE_DIR})
#
#elseif (MINGW)
#	include(FindWinHID)
#	target_link_libraries(${PROJECT_NAME} PRIVATE ${WINHID_LIBRARY} ${WINHID_SETUPAPI_LIBRARY})
#	target_include_directories(${PROJECT_NAME} PRIVATE ${WINHID_INCLUDE_DIR})
#	target_include_directories(${PROJECT_NAME} PRIVATE ${WINHID_CRT_INCLUDE_DIR})
#endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Network)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Concurrent)

target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::mhwallet)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::minter_tx)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::minter_api)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::toolbox)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::rxcpp)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::cpr)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::libcurl)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::libpng)

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
				COMMAND "${MACDEPLOYQT_EXECUTABLE}" "${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}.app" "-dmg"
				COMMENT "Running macdeployqt..."
			)
			#-always-overwrite
		endif ()
	endif ()
endif ()

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#	message(STATUS "VAR: ${_variableName}=${${_variableName}}")
#endforeach()

