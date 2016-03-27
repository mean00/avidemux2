
On Linux
=========
mkdir obj && cd obj && 
cmake -DCMAKE_INSTALL_PREFIX=/usr .. && make && sudo make install

You *NEED* to have avidemux_core-dev package installed
If it was not installed  in /usr, adjust CMAKE_INSTALL_PREFIX accordingly
