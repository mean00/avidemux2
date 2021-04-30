# Avidemux

Avidemux is a simple cross-platform video editor for Linux, Windows and MacOsX.

# Build Instructions

Get the main repository:
```
git clone https://github.com/mean00/avidemux2.git
```

Get submodules:
```
git submodule update --init --recursive
```

## Debian / Ubuntu and derivatives

Required packages:
```
sudo apt install gcc build-essential cmake pkg-config yasm libsqlite3-dev  qtbase5-dev qttools5-dev-tools wget
```

Recommended packages:
```
sudo apt install libxv-dev libvdpau-dev libva-dev libasound2-dev libpulse-dev libaom-dev libx264-dev libx265-dev libxvidcore-dev libvpx-dev libmad0-dev libmp3lame-dev libtwolame-dev libopus-dev libvorbis-dev libogg-dev libass-dev libaften-dev squashfs-tools
```

Build with one of the provided scripts, e.g.:
```
bash makeAppImage.sh
```

After the project has been compiled, it can be run:
```
bash run_avidemux_template.sh
```

> :warning: **In** _run_avidemux_template.sh_ **TOPSRCDIR="${HOME}/avidemux2"** probably need to be edited.  
:arrow_forward: Try **TOPSRCDIR="."** (current directory).


