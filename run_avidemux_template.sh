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
export LD_LIBRARY_PATH="${HERE}/lib64:${LD_LIBRARY_PATH}"
"${HERE}/bin/avidemux3_qt5" --portable "$@"

