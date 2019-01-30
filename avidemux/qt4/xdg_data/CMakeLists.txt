cmake_minimum_required(VERSION 2.8.11)
string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/org.avidemux.Avidemux.appdata.xml.in ${CMAKE_CURRENT_SOURCE_DIR}/org.avidemux.Avidemux.appdata.xml)

install(FILES org.avidemux.Avidemux.appdata.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/metainfo)
install(FILES org.avidemux.Avidemux.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES org.avidemux.Avidemux.png DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps)
