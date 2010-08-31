
On Linux
=========
mkdir obj && cd obj && 
cmake -G "CodeBlocks - Unix Makefiles" -DAVIDEMUX_SOURCE_DIR=path-to-avidemux-source/ -DCMAKE_INSTALL_PREFIX=/usr .. && make && sudo make install
