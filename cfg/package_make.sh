#!/usr/bin/env bash

set -e

INTERNAL_OPTS=""
TYPE="appimage"
DRY_RUN=""
CMAKE_OPTS=""
BUILD_ROOT="${PWD}/_build"
BUILD_TYPE="Debug"
ENABLE_CONAN="Off"

function usage() {
  echo ""
  echo ""
  echo "./package_make.sh"
  echo -e "    -h, --help       |  This help"
  echo -e "    -t, --type       |  Choose what package to build: appimage or native OS package (no supported yet)"
  echo -e "    --dry-run        |  Only echo commands without real building"
  echo -e "    -o, --options    |  Add string with additional cmake configure options. Example: -o \"-DVAR=1 -DVAR2=2\""
  echo -e "    -b, --buildroot  |  Path where to build project. Default: ${BUILD_ROOT}"
  echo ""
}

PARAMS=""
while (("$#")); do
  case "$1" in
  -h | --help)
    usage
    exit 0
    ;;
  --dry-run)
    DRY_RUN=1
    shift
    ;;
  -t | --type)
    if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
      TYPE=$2
      shift 2
    else
      echo "Error: Argument for $1 is missing" >&2
      exit 1
    fi
    ;;
  -b | --buildroot)
    if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
      BUILD_ROOT=$2
      shift 2
    else
      echo "Error: Argument for $1 is missing" >&2
      exit 1
    fi
    ;;
  -o | --options)
    if [ -n "$2" ]; then
      CMAKE_OPTS=$2
      shift 2
    else
      echo "Error: Argument for $1 is missing" >&2
      exit 1
    fi
    ;;
  -* | --*=) # unsupported flags
    echo "Error: Unsupported flag $1" >&2
    exit 1
    ;;
  *) # preserve positional arguments
    PARAMS="$PARAMS $1"
    shift
    ;;
  esac
done

_sysname=$(uname)

if [ "${_sysname}" == "Darwin" ]; then
  threadCount=$(sysctl -a machdep.cpu | grep thread_count | awk '{print $2}')
else
  cpusCnt=$(lscpu | grep -E '^CPU\(' | awk '{print $2}') # cpus count
  tpcpu=$(lscpu | grep -E '^Thread' | awk '{print $4}')  # threads per core
  threadCount=$((cpusCnt * tpcpu))

  if ((threadCount <= 0)); then
    threadCount=2
  fi
fi

if [ "${TYPE}" == "" ]; then
  echo "Please select type to build: appimage"
  exit 1
fi

if [ "${TYPE}" == "appimage" ]; then
  INTERNAL_OPTS="-DPACKAGING=On -DAPPIMAGE=nofuse"
  ENABLE_CONAN="On"
fi

mkdir -p ${BUILD_ROOT} && cd ${BUILD_ROOT}

rm -rf ./*
cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DENABLE_CONAN=${ENABLE_CONAN} ${INTERNAL_OPTS} ${CMAKE_OPTS}

if [ "${DRY_RUN}" != "1" ]; then
  make -j ${threadCount}
  cd ${BUILD_ROOT} && ./linux_deploy_qt.sh
fi
