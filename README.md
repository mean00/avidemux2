# Avidemux

Avidemux is a simple cross-platform video editor for Linux, Windows and MacOsX.

# Download executables

[Latest release](https://github.com/mean00/avidemux2/releases/latest)

[Nightly builds](https://www.avidemux.org/nightly/)

# Build from source

Get the main repository:
```
git clone https://github.com/mean00/avidemux2.git
cd avidemux2
git submodule update --init --recursive
```


## Build on Linux

Install build dependecies:
```
bash createDebFromSourceUbuntu.bash --deps-only
```

Build Avidemux:
```
bash bootStrap.bash --with-system-libass
```

The compiled output will be in the `install` subdirectory of `avidemux2`.  
Avidemux can run without installation, only a start script has to be made from the template script `run_avidemux_template.sh`.  
After copying the template script, it needs some editing:  
`TOPSRCDIR="${HOME}/avidemux2"`  
the path needs to point to the *actual* location of the cloned repository. 


## Build on MacOS

Install [Homebrew](https://github.com/Homebrew/brew)

Install required build dependencies:
```
brew install cmake nasm yasm qt xvid x264 x265 libvpx aom opus fdk-aac lame libass mp4v2 a52dec
```

Build Avidemux:
```
export MACOSX_DEPLOYMENT_TARGET=$(xcrun --sdk macosx --show-sdk-version)
bash bootStrapOsx_Catalina.bash --enable-qt6
```

The generated disk image should be  in the `installer` subdirectory of `avidemux2`.  


## Build for Windows

[Cross-compiling Avidemux on Linux for Windows](https://github.com/mean00/avidemux2/blob/master/cross-compiling.txt)
