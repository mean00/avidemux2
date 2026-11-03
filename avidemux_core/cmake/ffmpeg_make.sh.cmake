export PATH=${FF_ENV_PATH}:$PATH
# Default remains 4. Export FFMPEG_MAKE_JOBS to use fewer jobs when memory is tight.
jobs="@DOLLAR@{FFMPEG_MAKE_JOBS:-4}"
"${ffmpeg_gnumake_executable}" -j "$jobs" V=1
