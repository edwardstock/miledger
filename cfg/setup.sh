#!/usr/bin/env bash

if [ ! -f "/usr/local/bin/linuxdeployqt" ]; then
  curl -Lso /usr/local/bin/linuxdeployqt "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
  chmod +x /usr/local/bin/linuxdeployqt
fi

if [ ! -f "/tmp/cmake-3.21.0-rc1-linux-x86_64.sh" ]; then
  curl -Lso "/tmp/cmake-3.21.0-rc1-linux-x86_64.sh" https://github.com/Kitware/CMake/releases/download/v3.21.0-rc1/cmake-3.21.0-rc1-linux-x86_64.sh
  sh /tmp/cmake-3.21.0-rc1-linux-x86_64.sh --skip-license --prefix=/usr
fi
