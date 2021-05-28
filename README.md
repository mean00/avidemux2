# Avidemux

Avidemux is a simple cross-platform video editor for Linux, Windows and MacOsX.

# Build Instructions

Get the main repository:
```
git clone https://github.com/mean00/avidemux2.git
```

It needs an additional i18n (translation files) submodule. You can get it with:
```
git submodule update --init --recursive
```

The application can be built with the following convenience script (it will check and install all dependencies):
```
bash makeAppImageBusterMinimal.sh
```


> ### Troubleshooting
> In case not all the packages can be installed (unmet dependencies, broken packages, etc.), you can try to manually install packages.
> Required packages:
> ```
> sudo apt install gcc build-essential cmake pkg-config yasm libsqlite3-dev  qtbase5-dev qttools5-dev-tools wget
> ```
> Recommended packages:
> ```
> sudo apt install libxv-dev libvdpau-dev libva-dev libasound2-dev libpulse-dev libaom-dev libx264-dev libx265-dev libxvidcore-dev libvpx-dev libmad0-dev libmp3lame-dev libtwolame-dev libopus-dev libvorbis-dev libogg-dev libass-dev libaften-dev squashfs-tools
> ```
> In case of missing non-essential package(s), try build with:
> ```
> bash makeAppImage.sh
> ```

# Run Instructions

After the project has been compiled, you need to copy **run_avidemux_template.sh** to a appropriate location (covered by $PATH envvar).
This copied script needs some editing:  
`TOPSRCDIR="${HOME}/avidemux2"`  
the path needs to point to the *actual* location of the cloned repository.  
Also execution permission should be granted.

