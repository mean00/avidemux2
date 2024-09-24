# Avidemux

Avidemux is a simple, free, cross-platform video editor that is used for simple cutting, filtering, and encoding video tasks. Avidemux supports automated tasks using projects or scripting and is currently suppored on Linux, BSD, MacOS X, and Microsoft Windows.

Avidemux currently support the follwing file types:
* AVI
* DVD
* MPEG
* MP4
* ASF

[View our website](https://avidemux.sourceforge.net/)

# Why Avidemux?

Avidemux provides a simple, easy, and free video editor platform that most can quicky setup and use without much video editing knowledge. Avidemux can run on every major operating system and supports the most widely used video file types.

# Main Features

* Video Cuts
* Video Rotation and Flips
* Video Filters (tranform, interlacing, colors, noise, shaprness, miscellanious)

# Download executables

[Latest release](https://github.com/mean00/avidemux2/releases/latest)

[Nightly builds](https://www.avidemux.org/nightly/)

# Installation - Build from source

**Avidemux build directory must be located on a case-sensitive file system.**

Out-of-tree build is supported. If build is conducted in-tree, the source
directory has to be located on a case-sensitive file system either.

To get Avidemux source code from the main repository and the translations,
run the following command:
```
git clone --recursive https://github.com/mean00/avidemux2.git && cd avidemux2
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

The compiled output will be in the `install` subdirectory of the build directory.

Avidemux can be run without installation by means of a start script derived
from the template `run_avidemux_template.sh`.

1. Make a copy of this script file.
2. If Avidemux has been built in a different location than `${HOME}/avidemux2`,
edit the value of variable `BUILDTOP` to point to the *actual* build directory.
Adjust the value of variable `PREFIX` if necessary.
3. Copy the script to a directory listed in `$PATH` and make it executable.


## Build on macOS

Install [Homebrew](https://github.com/Homebrew/brew)

Install required build dependencies:
```
brew install cmake pkg-config nasm yasm qt xvid x264 x265 libvpx aom opus fdk-aac lame libass mp4v2 a52dec
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
Only if Avidemux has been built on a different Apple Silicon system,
execute the following command to replace ad-hoc signatures of the binaries:
```
sh avidemux/osxInstaller/macos-adhoc-sign-installed-app.sh
```
When Avidemux app has been installed to a non-default location, adjust the value
of `BUNDLE_CONTENT` variable in the aforementioned file accordingly.

## Build for Windows

[Cross-compiling Avidemux on Linux for Windows](https://github.com/mean00/avidemux2/blob/master/cross-compiling.txt)

# Examples - Different Windows

<img width="597" alt="Screenshot 2024-09-24 at 11 18 47 AM" src="https://github.com/user-attachments/assets/5d4648bd-a885-4ab6-a277-7657be405ae2">

# Documentation and Support

[Wiki Docs](https://www.avidemux.org/admWiki/doku.php)

[Forum](https://www.avidemux.org/admForum/)

[Reddit Forum](https://www.reddit.com/r/Avidemux/)

[News](https://avidemux.sourceforge.net/news.html)

## Contributors

Avidemux was written by Mean but code from other's are always welcome and have been used.
