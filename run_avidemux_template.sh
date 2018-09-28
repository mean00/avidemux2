#!/bin/bash
# This is a script template to run Avidemux on Linux without installation.
# Adjust variables appropriately, copy it to a location included in $PATH
# and make it executable.

# TOPSRCDIR must match the location of the Avidemux source tree,
# default: ${HOME}/avidemux2
TOPSRCDIR="${HOME}/avidemux2"

# PREFIX must match the prefix passed to bootStrap.bash when compiling Avidemux,
# default when no prefix specified: /usr
PREFIX="/usr"

HERE="${TOPSRCDIR}/install${PREFIX}"
CORECONFIG=$(find install -name ADM_coreConfig.h)
if [ "x$?" = "x0" ]; then
    LIBDIR=$(grep ADM_RELATIVE_LIB_DIR ${CORECONFIG} | cut -f 3 -d " " | sed 's/\"//g')
else
    echo "ADM_coreConfig.h not found, can't determine the relative library directory. Aborting."
    exit 1
fi

if ! [ -z "${LIBDIR}" ] && ! [ -e "${HERE}/lib" ] && ! [ "x${LIBDIR}" = "xlib" ]; then
    ln -s "${HERE}/${LIBDIR}" "${HERE}/lib"
fi

export LD_LIBRARY_PATH="${HERE}/${LIBDIR}:${LD_LIBRARY_PATH}"
"${HERE}/bin/avidemux3_qt5" --portable "$@"

