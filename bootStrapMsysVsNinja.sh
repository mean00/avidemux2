#!F:/msys64/usr/bin/bash.exe -l
#
# Avidemux - Windows/MSVC build driver (MSYS2 + vcpkg)
# Generator: Ninja Multi-Config  ->  cl.exe + ccache
#
# Why not "Visual Studio 17 2022":
#   * cl.exe is used either way (use_msvc sets up the x64 MSVC environment),
#   * but CMAKE_<LANG>_COMPILER_LAUNCHER is only honoured by the Ninja (and
#     Makefile) generators -> ccache works here, and is silently ignored by MSBuild,
#   * Ninja Multi-Config keeps the --config semantics of the VS generator.
#   * Ninja rejects -A x64: the arch is whatever cl.exe use_msvc selected.
#
fail()
{
    echo "FAILURE AT STEP $1"
    exit 1
}

cd "/f/$(cygpath -u "$WORKSPACE")" 2>/dev/null || cd "$(cygpath -u "$WORKSPACE")" || fail "workspace cd"
source ~/.bashrc
use_msvc

echo "Shell : $SHELL"
echo "PWD   : $PWD"
echo "cl    : $(command -v cl)"
echo "ninja : $(command -v ninja)"
echo "ccache: $(command -v ccache) $(ccache --version 2>/dev/null | head -1)"

export AVIDEMUX_START_POINT=$PWD
export GEN="Ninja Multi-Config"
export CONFIG=RelWithDebInfo

# admMainChecks.cmake does SET(CMAKE_C_FLAGS $ENV{CFLAGS}) /
# SET(CMAKE_CXX_FLAGS $ENV{CXXFLAGS}), replacing CMake's defaults. With the
# multi-config VS generator MSBuild supplied them; with Ninja these variables
# *are* the compile line, so the base MSVC flags have to be exported here.
export CFLAGS="-DWIN32 -D_WINDOWS"
export CXXFLAGS="-DWIN32 -D_WINDOWS /EHsc"

build_modules() {
    local folder_name=$1
    local source=$2
    shift 2

    echo "*** building module $source into folder $folder_name"
    cd "$AVIDEMUX_START_POINT" || fail $folder_name
    rm -Rf "$folder_name" && mkdir "$folder_name" || fail $folder_name
    cd "$folder_name" || fail $folder_name

    cmake -G "$GEN" \
          -DLRELEASE_EXECUTABLE=F:/vcpkg/vcpkg/installed/x64-windows/tools/Qt6/bin/lrelease.exe \
          -DCMAKE_INSTALL_PREFIX="$(cygpath -m "$AVIDEMUX_START_POINT")/win64" \
          -DCMAKE_CONFIGURATION_TYPES="$CONFIG" \
          -DCMAKE_BUILD_TYPE="$CONFIG" \
          -DCMAKE_TRY_COMPILE_CONFIGURATION="$CONFIG" \
          -DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=Embedded \
          -DCMAKE_TOOLCHAIN_FILE="$(cygpath -m "$VCPKG_ROOT")/scripts/buildsystems/vcpkg.cmake" \
          -DVCPKG_TARGET_TRIPLET=x64-windows \
          -S "../$source" "$@" || fail "$folder_name (configure)"

    # /Z7 (Embedded) + Ninja + launcher => ccache can actually store the objects
    if grep -rqs ccache --include='*.ninja' . ; then
        echo "    ccache: wired into the ninja rules"
    else
        echo "    ccache: WARNING - not found in the ninja rules"
    fi

    cmake --build . --config "$CONFIG" --target install || fail "$folder_name (build/install)"
}

# The core must be installed into win64 *before* the other projects: they resolve
# ADM_HEADER_DIR = <prefix>/include/avidemux/2.8 and abort if it is missing.
#build_modules buildCore  avidemux_core
#build_modules buildQt4   avidemux/qt4          -DENABLE_QT6=True

build_modules buildPluginCommon avidemux_plugins -DPLUGIN_UI=COMMON
build_modules buildPluginQt4    avidemux_plugins -DPLUGIN_UI=QT4 -DENABLE_QT6=True
#build_modules buildPluginCli    avidemux_plugins -DPLUGIN_UI=CLI
build_modules buildSettings     avidemux_plugins -DPLUGIN_UI=SETTINGS

echo
echo "ccache stats:"
ccache -s | head -6

echo "** ALL DONE **"
