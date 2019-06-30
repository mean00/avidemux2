#
# Runtime
#
SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${AVIDEMUX_BIN_DIR})
include(InstallRequiredSystemLibraries)
MESSAGE(STATUS "MSVC Runtime files = ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
ADM_INSTALL_LIB_FILES( "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
#
# Qt5
#
macro(COPY_QT5_DLL module)
    #get_target_property( Qt5_${module}Location ${Qt5${module}_LIBRARIES} LOCATION)
    get_target_property(Qt5_${module}Location Qt5::${module} LOCATION) 
    MESSAGE(STATUS "Adding ${Qt5_${module}Location} as lib to install")
    ADM_INSTALL_LIB_FILES( "${Qt5_${module}Location}")
endmacro(COPY_QT5_DLL module)

COPY_QT5_DLL(Core)
COPY_QT5_DLL(Gui)
COPY_QT5_DLL(Widgets)
COPY_QT5_DLL(Network)
COPY_QT5_DLL(WinExtras)


# Need the platform one too => TODO

# Copy vista style too
INSTALL(FILES "${QT_PLUGINS_DIR}/styles/qwindowsvistastyle.dll" DESTINATION "${AVIDEMUX_BIN_DIR}/styles")
#
INSTALL(FILES "${QT_PLUGINS_DIR}/platforms/qminimal.dll" DESTINATION "${AVIDEMUX_BIN_DIR}/platforms")
INSTALL(FILES "${QT_PLUGINS_DIR}/platforms/qwindows.dll" DESTINATION "${AVIDEMUX_BIN_DIR}/platforms")
