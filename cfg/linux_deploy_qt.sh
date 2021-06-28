#!/usr/bin/env bash
set -e
set -x
PATH=@CONAN_QT_ROOT@/bin:$PATH \
  VERSION=@CMAKE_PROJECT_VERSION@ \
  LD_LIBRARY_PATH=@CONAN_QT_ROOT@/lib:$LD_LIBRARY_PATH \
  @LINUXDEPLOYQT_EXECUTABLE@ @APPIMAGE_ROOT@/usr/bin/@PROJECT_NAME@ \
  -always-overwrite \
  -unsupported-allow-new-glibc \
  -appimage -bundle-non-qt-libs \
  -extra-plugins=iconengines,platforms,imageformats,platformthemes \
  -verbose=3
