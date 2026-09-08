# Avidemux 

Avidemux is a simple, free, cross-platform video editor that is used for simple cutting, filtering, and encoding video tasks. Avidemux supports automated tasks using projects or scripting and is currently supported on Linux, BSD, MacOS X, and Microsoft Windows. Avidemux was primarily made with the C and C++ programming languages and was created to act as a desktop based video editing application.

Avidemux currently support the follwing file types:
* AVI
* DVD
* MPEG
* MP4
* ASF

[View our website](https://avidemux.sourceforge.net/)

# Table of Contents:
[Why Avidemux](https://github.com/kaimcfarlane/avidemux2/blob/kaimcfarlane-ReadMe_Updates/README.md#why-avidemux)  
[Main Features](https://github.com/kaimcfarlane/avidemux2/blob/kaimcfarlane-ReadMe_Updates/README.md#main-features)  
[Download Executables](https://github.com/kaimcfarlane/avidemux2/blob/kaimcfarlane-ReadMe_Updates/README.md#download-executables)  
[Installation](https://github.com/kaimcfarlane/avidemux2/blob/kaimcfarlane-ReadMe_Updates/README.md#installation---build-from-source)  
[Examples](https://github.com/kaimcfarlane/avidemux2/blob/kaimcfarlane-ReadMe_Updates/README.md#examples---different-windows)  
[Documentation](https://github.com/kaimcfarlane/avidemux2/blob/kaimcfarlane-ReadMe_Updates/README.md#documentation-and-support)  
[Collaborators](https://github.com/kaimcfarlane/avidemux2/blob/kaimcfarlane-ReadMe_Updates/README.md#collaborators)  

# Why Avidemux?

Avidemux provides a simple, easy, and free video editor platform that most can quicky setup and use without much video editing experience. Avidemux can run on every major operating system and supports the most widely used video file types.

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
bash bootStrap.bash
```
> Alternatively building on Qt5 instead of Qt6:
```
bash bootStrap.bash --enable-qt5
```

The compiled output will be in the `install` subdirectory of the build directory.

Avidemux can be run without installation by means of a start script derived
from the template `run_avidemux_template_qt6.sh`.

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

## Main Window
Holds general information like project video timeline and main editing options within the top banner.

![avidemux1](https://github.com/user-attachments/assets/e0cffa49-ed3c-4f28-8b8a-7f581cf709bb)

## Filters
Panel where you can apply different filters to current selected video.

![avidemux2](https://github.com/user-attachments/assets/b25c88b6-4aad-4f0d-982a-630031f1d0a6)

## Video Filter Manager
Menu where you can add/remove filters to your project.

![avidemux3](https://github.com/user-attachments/assets/57342fb9-c3e8-4cf1-ac36-ee1ef76cf0ba)

## Options
Settings where you can adjust output and general Avidemux settings.

![avidemux4](https://github.com/user-attachments/assets/5a963cb8-1129-4d50-9f7e-2fe5f333d4bc)


# Documentation and Support

[Wiki Docs](https://www.avidemux.org/admWiki/doku.php)

[Forum](https://www.avidemux.org/admForum/)

[Reddit Forum](https://www.reddit.com/r/Avidemux/)

[News](https://avidemux.sourceforge.net/news.html)

# Collaborators

Avidemux was written by Mean but code constributions from other's are always welcome and have been used.
