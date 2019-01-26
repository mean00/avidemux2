#!/bin/bash
install_packages=1
default_install_prefix="/usr"
install_prefix="$default_install_prefix"
rebuild=""
#
usage()
{
    echo "Usage: $0 [Options]"
    echo "***********************"
    echo "  --help or -h      : Print usage"
    echo "  --deps-only       : Just install build dependencies and exit"
    echo "  --prefix=DIR      : Install to directory DIR (default: $default_install_prefix)"
    echo "  --no-install      : Don't install generated debian avidemux packages"
    echo "  --rebuild         : Preserve existing build directories"
}
#
install_deps()
{
    echo "This will install all the packages necessary to build avidemux"
    echo "You will be asked to enter your password because installing build dependencies requires root permissions"
    # gcc, g++ and make get installed as dependencies of build-essential
    sudo apt-get update && sudo apt-get install build-essential cmake pkg-config yasm \
    libsqlite3-dev libfontconfig1-dev libfribidi-dev libxv-dev libvdpau-dev libva-dev libasound2-dev libpulse-dev \
    qttools5-dev-tools qtbase5-dev \
    libpng-dev libaften-dev libmp3lame-dev libx264-dev libxvidcore-dev libfaad-dev libfaac-dev libopus-dev libvorbis-dev libogg-dev libdca-dev \
    || { echo "The installation at least of some of the build dependencies failed. Aborting." && exit 2; }
    sudo apt-get install libx265-dev \
    || echo "Warning: libx265-dev cannot be installed using package management. Avidemux won't be able to encode in h265 unless the library and the headers have been installed manually. Continuing anyway." # there are no official libx265 packages for Ubuntu Trusty
}
#
install_avidemux()
{
    echo "Installing avidemux..."
    cd debs && sudo dpkg -i *
}
#
option_value()
{
    echo $(echo $* | cut -d '=' -f 2-)
}
#
option_name()
{
    echo $(echo $* | cut -d '=' -f 1 | cut -b 3-)
}
#
dir_check()
{
    op_name="$1"
    dir_path="$2"
    if [ "x$dir_path" != "x" ]; then
        if [[ "$dir_path" != /* ]]; then
            >&2 echo "Expected an absolute path for --$op_name=$dir_path, aborting."
            exit 1
        fi
    else
        >&2 echo "Empty path provided for --$op_name, aborting."
        exit 1
    fi
    case "$dir_path" in
      */)
          echo $(expr "x$dir_path" : 'x\(.*[^/]\)') # strip trailing slashes
          ;;
      *)
          echo "$dir_path"
          ;;
    esac
}
#
while [ $# != 0 ]; do
    config_option="$1"
    case "$config_option" in
        -h|--help)
            usage
            exit 0
            ;;
        --deps-only)
            install_deps
            exit 0
            ;;
        --prefix=*)
            install_prefix=$(dir_check $(option_name "$config_option") $(option_value "$config_option")) || exit 1
            ;;
        --no-install)
            install_packages=0
            ;;
        --rebuild)
            rebuild="$config_option"
            ;;
        *)
            echo "unknown parameter $config_option"
            usage
            exit 1
            ;;
    esac
    shift
done
#
install_deps
#
echo "Compiling avidemux, it will take 20 minutes or so"
logfile="/tmp/log-bootstrap-$(date +%F_%T).log"
bash bootStrap.bash --deb --prefix=$install_prefix $rebuild 2>&1 | tee ${logfile}
if [ ${PIPESTATUS[0]} -ne 0 ]; then
    echo "Build failed, please inspect ${logfile} and /tmp/logbuild* files."
    if [ $install_packages -eq 1 ]; then
        echo "Cancelling installation."
    fi
    exit 3
fi
#
if [ $install_packages -eq 1 ]; then
    install_avidemux
    exit $?
fi
#
exit 0
