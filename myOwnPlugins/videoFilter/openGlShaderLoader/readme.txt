
On Linux
=========
mkdir obj && cd obj && 
cmake -DCMAKE_INSTALL_PREFIX=/usr .. && make && sudo make install

You *NEED* to have avidemux_core-dev && avidemux_qt5-dev package installed
If it was not installed  in /usr, adjust CMAKE_INSTALL_PREFIX accordingly

On Windows
============

Start cmake-gui and add
CMAKE_INSTALL_PREFIX, type string, value =c:\Program Files\Avidemux 2.6-64bits or wherever you installed avidemux
You *MUST* install the SDK subpackage
