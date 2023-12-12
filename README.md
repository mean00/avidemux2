# Avidemux

Avidemux is a simple cross-platform video editor for Linux, Windows and macOS.

# Download executables

[Latest release](https://github.com/mean00/avidemux2/releases/latest)

[Nightly builds](https://www.avidemux.org/nightly/)

# Build from source

To get Avidemux source code from the main repository and the translations,
run the following commands in a directory located on a case-sensitive file system:
```
git clone https://github.com/mean00/avidemux2.git
cd avidemux2
git submodule update --init --recursive
```


## Build on Linux

Install build dependencies:

> Debian / Ubuntu and variants:
```
bash createDebFromSourceUbuntu.bash --deps-only
```
> Fedora:
```
bash createRpmFromSourceFedora.bash --deps-only
```
Build Avidemux:
```
bash bootStrap.bash --with-system-libass
```

The compiled output will be in the `install` subdirectory of `avidemux2`.

Avidemux can be run without installation by means of a start script derived
from the template `run_avidemux_template.sh`.

1. Make a copy of this script file.
2. Only if Avidemux repository has been cloned to a different location than `${HOME}/avidemux2`,
edit the value of variable `TOPSRCDIR` to point to the *actual* location of the source tree.
3. Copy the script to a directory listed in `$PATH` and make it executable.


## Build on macOS

Install [Homebrew](https://github.com/Homebrew/brew)

Install required build dependencies:
```
brew install cmake nasm yasm qt xvid x264 x265 libvpx aom opus fdk-aac lame libass mp4v2 a52dec
```

Build Avidemux (Apple Silicon):  
It may be necessary to install Xcode, not just Command Line Tools, else creation of app bundle fails.
```
bash bootStrapMacOS_Monterey.arm64.sh
```

Build Avidemux (Intel):
```
export MACOSX_DEPLOYMENT_TARGET=$(xcrun --sdk macosx --show-sdk-version)
bash bootStrapOsx_Catalina.bash --enable-qt6
```
On both Apple platforms, the disk image should be generated in the `installer`
subdirectory of `avidemux2`.

Post-installation (Apple Silicon):  
Execute the following command to ad-hoc sign the binaries:
```
sh avidemux/osxInstaller/macos-adhoc-sign-installed-app.sh
```
When Avidemux app has been installed to a non-default location, adjust the value
of `BUNDLE_CONTENT` variable in the aforementioned file accordingly.

## Build for Windows

[Cross-compiling Avidemux on Linux for Windows](https://github.com/mean00/avidemux2/blob/master/cross-compiling.txt)
