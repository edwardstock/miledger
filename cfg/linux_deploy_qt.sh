#!/usr/bin/env bash
set -e
set -x

if [ "@APPIMAGE@" == "nofuse" ]; then
  if [ ! -d "$(pwd)/squashfs-root/" ]; then
    @LINUXDEPLOYQT_EXECUTABLE@ --appimage-extract
  fi
  LINUXDEPLOYQT_EXTRACTED_EXE=$(pwd)/squashfs-root/AppRun
else
  LINUXDEPLOYQT_EXTRACTED_EXE=@LINUXDEPLOYQT_EXECUTABLE@
fi

_plugins=(iconengines platforms imageformats platformthemes)
_plugins_cnt=${#_plugins[@]}
_extra_plugs=""
_plugins_path="@CONAN_QT_ROOT@/res/archdatadir/plugins"
_plugins_t_path=@APPIMAGE_ROOT@/usr/plugins/

mkdir -p ${_plugins_t_path}

for ((plug_idx = 0, n = 0; plug_idx < _plugins_cnt; plug_idx++)); do
  if [ -d "${_plugins_path}/${_plugins[$plug_idx]}" ]; then
    cp -r ${_plugins_path}/${_plugins[$plug_idx]} ${_plugins_t_path}
    if (($plug_idx == 0)); then
      _extra_plugs="${_plugins[$plug_idx]}"
    else
      _extra_plugs="${_extra_plugs},${_plugins[$plug_idx]}"
    fi
  fi
done

PATH=@CONAN_QT_ROOT@/bin:$PATH \
  VERSION=@CMAKE_PROJECT_VERSION@ \
  LD_LIBRARY_PATH=@CONAN_QT_ROOT@/lib:$LD_LIBRARY_PATH \
  ${LINUXDEPLOYQT_EXTRACTED_EXE} @APPIMAGE_ROOT@/usr/bin/@PROJECT_NAME@ \
  -qmake=@CONAN_QT_ROOT@/bin/qmake \
  -always-overwrite \
  -unsupported-allow-new-glibc \
  -appimage -bundle-non-qt-libs \
  -extra-plugins=${_extra_plugs} \
  -verbose=1
