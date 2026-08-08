#!/bin/bash

usage()
{
  echo "Usage: build.sh [clean] [arm64|x86_64|universal] [CUSTOM_BUILD_FLAGS=x] [noroot] [release|debug] [-h|-?] [-l (static|shared)] [-D CMAKE_OPTION] [-v]"
  echo "                                                                                         "
  echo "options:                                                                                 "
  echo "                                                                                         "
  echo "Positional options:                                                                          "
  echo "[clean]                  - perform clean build                                           "
  echo "[arm64|x86_64|universal] - Apple platform build type. Not applicable to other OS.        "
  echo "[CUSTOM_BUILD_FLAGS]     - custom CXX compiler flags                                     "
  echo "[noroot]                 - custom CXX compiler flags                                     "
  echo "[release|debug]          - Specify build type (defaults to Debug)                        "
  echo "                                                                                         "
  echo "Additional parameters:                                                                   "
  echo " -h | -?                 - this help.                                                    "
  echo " -l [static|shared]      - build static (default) or shared library.                     "
  echo " -D [CMAKE_OPTION]       - additional options to pass to cmake. Could be multiple.       "
  echo " -v                      - increase build verbosity (reserved for future use)            "
  echo "                                                                                         "
  echo "Environment variables:                                                                   "
  echo "CMAKE_OPTS               - any additional cmake options.                                 "
  echo "GIT_PULL_TOKEN           - authorization token for Microsoft-proprietary modules.        "
  echo "MACOSX_DEPLOYMENT_TARGET - optional parameter for setting macosx deployment target       "
  echo "Plus any other environment variables respected by CMake build system.                    "
  exit 0
}

export PATH=/usr/local/bin:$PATH

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Current directory: $DIR"
cd $DIR
. "$DIR/tools/build-common.sh"

export NOROOT=$NOROOT

# Evaluate arguments that are not switches
while [[ $# -gt 0 ]]; do
  ARG="$1"
  case "$ARG" in
    clean|noroot|release|debug|arm64|x86_64|universal|CUSTOM_BUILD_FLAGS*)
      case "$ARG" in
        clean)
          CLEAN=true
          echo "CLEAN = true"
          ;;
        noroot)
          export NOROOT=true
          echo "NOROOT = true"
          ;;
        release|debug)
          if [[ -n "$BUILD_TYPE" ]]; then
              echo "Error: BUILD_TYPE is already set to '$BUILD_TYPE'. Cannot overwrite with $ARG." 1>&2
              exit 1
          elif [[ "$ARG" == "release" ]]; then
              BUILD_TYPE="Release"
          elif [[ "$ARG" == "debug" ]]; then
              BUILD_TYPE="Debug"
          fi
          echo "BUILD_TYPE = $BUILD_TYPE"
          ;;
        arm64|x86_64|universal)
          if [[ -n "$APPLE_ARCH" ]]; then
              echo "Error: APPLE_ARCH is already set to '$APPLE_ARCH'. Cannot overwrite with $ARG." 1>&2
              exit 1
          else
              APPLE_ARCH="$ARG"
          fi
          echo "APPLE_ARCH = $APPLE_ARCH"
          ;;
        CUSTOM_BUILD_FLAGS*)
          CUSTOM_CMAKE_CXX_FLAG="${ARG:19:999}"
          echo "custom compiler flags = $CUSTOM_CMAKE_CXX_FLAG"
          ;;
        *)
          echo "Error: case not added: $ARG" 1>&2
          exit 1
          ;;
      esac
      shift
      ;;
    *)
      break
      ;;
  esac
done

if [[ -z "$BUILD_TYPE" ]]; then
  BUILD_TYPE="Debug"
  echo "Assuming default BUILD_TYPE = Debug"
fi

if [[ -z "$APPLE_ARCH" ]]; then
  APPLE_ARCH=$(/usr/bin/uname -m)
  echo "Using current machine APPLE_ARCH = $APPLE_ARCH"
fi

# Evaluate switches
LINK_TYPE=
CMAKE_OPTS="${CMAKE_OPTS:--DBUILD_SHARED_LIBS=OFF}"
while getopts "h?vl:D:" opt; do
    case "$opt" in
    h|\?) usage
        ;;
    :)  echo "Invalid option: $OPTARG requires an argument" 1>&2
        exit 0
        ;;
    v)  verbose=1
        ;;
    D)  CMAKE_OPTS="${CMAKE_OPTS} -D${OPTARG}"
        ;;
    l)  LINK_TYPE=$OPTARG
        ;;
    esac
done

# Detect args accidentally passed after the last switch argument
shift $((OPTIND -1))
if [[ $# -gt 0 ]]; then
    echo "Error: There are arguments remaining after parsing all positional arguments and switches: $@" 1>&2
    exit 1
fi

if [[ "$CLEAN" == "true" ]]; then
  matsdk_clean_build_outputs "build.sh"
fi

echo "CMAKE_OPTS from caller: $CMAKE_OPTS"

if [ "$LINK_TYPE" == "shared" ]; then
  CMAKE_OPTS="${CMAKE_OPTS} -DBUILD_SHARED_LIBS=ON"
fi

# Set target MacOS minver
default_mac_os_target=$([ "$APPLE_ARCH" == "arm64" ] && echo "11.10" || echo "10.10")
[ -z $MACOSX_DEPLOYMENT_TARGET ] && export MACOSX_DEPLOYMENT_TARGET=${default_mac_os_target}
echo "macosx deployment target="$MACOSX_DEPLOYMENT_TARGET

# Install build tools and recent sqlite3
BUILD_TOOLS_MARKER=.buildtools
OS_NAME=`uname -a`

if [ ! -f "$BUILD_TOOLS_MARKER" ]; then
  buildtools_cmd=()
  case "$OS_NAME" in
    *Darwin*) buildtools_cmd=(tools/setup-buildtools-apple.sh "$APPLE_ARCH") ;;
    *Linux*)  buildtools_cmd=(tools/setup-buildtools.sh) ;;
    *)        echo "WARNING: unsupported OS $OS_NAME, skipping build tools installation.." ;;
  esac

  if [[ ${#buildtools_cmd[@]} -gt 0 ]]; then
    if [[ -z "$NOROOT" ]]; then
      matsdk_try_buildtools_once "$BUILD_TOOLS_MARKER" \
        "No root: skipping build tools installation." \
        sudo "${buildtools_cmd[@]}"
    else
      echo "No root: skipping build tools installation."
      matsdk_mark_buildtools_checked "$BUILD_TOOLS_MARKER"
    fi
  else
    matsdk_mark_buildtools_checked "$BUILD_TOOLS_MARKER"
  fi
fi

matsdk_print_compiler_versions
matsdk_require_cmake_preset_support

# Skip Version.hpp changes
# git update-index --skip-worktree lib/include/public/Version.hpp

# .tgz package
CPACK_GENERATOR=TGZ

if [ -f /usr/bin/dpkg ]; then
  # .deb package
  export CPACK_GENERATOR=DEB
elif [ -f /usr/bin/rpmbuild ]; then
  # .rpm package
  export CPACK_GENERATOR=RPM
fi

# Fail on error
set -e

PRESET="matsdk-$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"
cmake_args=(cmake --preset "$PRESET")
if [[ "$OS_NAME" == *Darwin* ]]; then
  if [[ "$APPLE_ARCH" == "universal" ]]; then
    cmake_args+=("-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64")
  else
    cmake_args+=("-DCMAKE_OSX_ARCHITECTURES=$APPLE_ARCH")
  fi
fi
cmake_args+=(
  "-DCPACK_GENERATOR=$CPACK_GENERATOR"
)
if [[ -n "$CUSTOM_CMAKE_CXX_FLAG" ]]; then
  cmake_args+=("-DCMAKE_CXX_FLAGS=$CUSTOM_CMAKE_CXX_FLAG")
fi
matsdk_append_cmake_opts_to_cmake_args
matsdk_run_logged_command "${cmake_args[@]}"

rm -f out/*.deb out/*.rpm
matsdk_build_and_package_preset "$PRESET"
cd out

# Install newly generated package
if [ -f /usr/bin/dpkg ]; then
  # Ubuntu / Debian / Raspbian 
  [[ -z "$NOROOT" ]] && sudo dpkg -i *.deb || echo "No root: skipping package deployment."
elif [ -f /usr/bin/rpmbuild ]; then
  # Redhat / Centos
  [[ -z "$NOROOT" ]] && sudo rpm -i --force -v *.rpm || echo "No root: skipping package deployment."
fi

# Install SDK headers and lib to /usr/local
#
## TODO: [MG] - fix this section for shared library
## strip --strip-unneeded out/lib/libmat.so
## strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag out/lib/libmat.so

if [ "$CPACK_GENERATOR" == "TGZ" ]; then
  cd ..
  MATSDK_INSTALL_DIR="${MATSDK_INSTALL_DIR:-/usr/local}"
  echo "+-----------------------------------------------------------------------------------+"
  echo " This step may prompt for your sudo password to deploy SDK to $MATSDK_INSTALL_DIR  "
  echo "+-----------------------------------------------------------------------------------+"
  [[ -z "$NOROOT" ]] && sudo ./install.sh $MATSDK_INSTALL_DIR || echo "No root: skipping package deployment."
fi
