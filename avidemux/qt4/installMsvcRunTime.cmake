#
# Runtime
#
SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${AVIDEMUX_BIN_DIR})
INCLUDE(InstallRequiredSystemLibraries)
MESSAGE(STATUS "MSVC Runtime files = ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
ADM_INSTALL_LIB_FILES( "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
#
# Qt6
#
MACRO(COPY_QT6_DLL module)
  #get_target_property( Qt5_${module}Location ${Qt6${module}_LIBRARIES} LOCATION)
  GET_TARGET_PROPERTY(Qt6_${module}Location Qt6::${module} LOCATION)
  MESSAGE(STATUS "Adding ${Qt6_${module}Location} as lib to install")
  ADM_INSTALL_LIB_FILES( "${Qt6_${module}Location}")
ENDMACRO()

COPY_QT6_DLL(Core)
COPY_QT6_DLL(Gui)
COPY_QT6_DLL(Widgets)
COPY_QT6_DLL(Network)
#COPY_QT6_DLL(WinExtras)


# Need the platform one too => TODO
# Copy vista style too
#INSTALL(FILES "${QT_PLUGINS_DIR}/styles/qwindowsvistastyle.dll" DESTINATION "${AVIDEMUX_BIN_DIR}/styles")
#
INSTALL(FILES "${QT_PLUGINS_DIR}/platforms/qminimal.dll" DESTINATION "${AVIDEMUX_BIN_DIR}/platforms")
INSTALL(FILES "${QT_PLUGINS_DIR}/platforms/qwindows.dll" DESTINATION "${AVIDEMUX_BIN_DIR}/platforms")
