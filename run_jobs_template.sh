#!/bin/bash
# This is a script template to run Avidemux Jobs on Linux without installation.
# Adjust variables appropriately, copy it to a location included in $PATH
# and make it executable.

# TOPSRCDIR must match the location of the Avidemux source tree,
# default: ${HOME}/avidemux2
TOPSRCDIR="${HOME}/avidemux2"

# PREFIX must match the prefix passed to bootStrap.bash when compiling Avidemux,
# default when no prefix specified: /usr
PREFIX="/usr"

# Avidemux version
MAJOR="2"
MINOR="7"

fail()
{
    echo "$1. Aborting."
    exit 1
}
HERE="${TOPSRCDIR}/install${PREFIX}"
CORECONFIG="${HERE}/include/avidemux/${MAJOR}.${MINOR}/ADM_coreConfig.h"
if ! [ -e "${CORECONFIG}" ]; then
    fail "${CORECONFIG} not found, can't determine the relative library directory"
fi
LIBDIR=$(grep ADM_RELATIVE_LIB_DIR "${CORECONFIG}" | cut -f 3 -d " " | sed -e 's/^"//' -e 's/"$//')
if [ -z "${LIBDIR}" ]; then
    fail "ADM_RELATIVE_LIB_DIR empty or not set in ${CORECONFIG}"
fi

if ! [ -e "${HERE}/lib" ]; then
    ln -s "${HERE}/${LIBDIR}" "${HERE}/lib"
fi

export LD_LIBRARY_PATH="${HERE}/${LIBDIR}:${LD_LIBRARY_PATH}"
"${HERE}/bin/avidemux3_jobs_qt5" --portable

