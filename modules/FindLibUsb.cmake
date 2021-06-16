# - Find libusb for portable USB support
#
# If the LibUSB_ROOT environment variable
# is defined, it will be used as base path.
# The following standard variables get defined:
#  LibUSB_FOUND:    true if LibUsb was found
#  LibUSB_INCLUDE_DIR: the directory that contains the include file
#  LibUSB_LIBRARIES:  the libraries

if (PKG_CONFIG_FOUND)
	if (DEPENDS_DIR) #Otherwise use System pkg-config path
		set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${DEPENDS_DIR}/libusb/lib/pkgconfig")
	endif ()
	set(MODULE "libusb-1.0")
	if (CMAKE_SYSTEM_NAME MATCHES "Linux")
		set(MODULE "libusb-1.0>=1.0.20")
	endif ()
	if (LibUSB_FIND_REQUIRED)
		set(LibUSB_REQUIRED "REQUIRED")
	endif ()
	pkg_check_modules(LibUsb ${LibUSB_REQUIRED} ${MODULE})

	find_library(LibUsb_LIBRARY usb)
	set(LibUsb_LIBRARIES ${LibUsb_LIBRARY})

	return()
endif ()

find_path(LibUsb_INCLUDE_DIRS
          NAMES libusb.h
          PATHS
          "${DEPENDS_DIR}/libusb"
          "${DEPENDS_DIR}/libusbx"
          ENV LibUSB_ROOT
          PATH_SUFFIXES
          include
          libusb
          include/libusb-1.0
          )

set(LIBUSB_NAME libusb-1.0.so libusb-1.0.a libusb.a libusb.so usb.a usb.so usb usb-1.0 usb1.0)

find_library(LibUsb_LIBRARIES
             NAMES ${LIBUSB_NAME}
             PATHS
             "${DEPENDS_DIR}/libusb"
             "${DEPENDS_DIR}/libusbx"
             ENV LibUSB_ROOT
             PATH_SUFFIXES
             x64/Release/dll
             x64/Debug/dll
             Win32/Release/dll
             Win32/Debug/dll
             MS64
             MS64/dll
             )

if (WIN32)
	find_file(LibUsb_DLL
	          ${LIBUSB_NAME}-1.0.dll
	          PATHS
	          "${DEPENDS_DIR}/libusb"
	          "${DEPENDS_DIR}/libusbx"
	          ENV LibUSB_ROOT
	          PATH_SUFFIXES
	          x64/Release/dll
	          x64/Debug/dll
	          Win32/Release/dll
	          Win32/Debug/dll
	          MS64
	          MS64/dll
	          )
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUsb FOUND_VAR LibUsb_FOUND
                                  REQUIRED_VARS LibUsb_LIBRARIES LibUsb_INCLUDE_DIRS)