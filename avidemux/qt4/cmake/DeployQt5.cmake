# - Functions to help assemble a standalone Qt5 executable.
# A collection of CMake utility functions useful for deploying
# Qt5 executables.
#
# The following functions are provided by this module:
#   write_qt5_conf
#   resolve_qt5_paths
#   fixup_qt5_executable
#   install_qt5_plugin_path
#   install_qt5_plugin
#   install_qt5_executable
# Requires CMake 2.6 or greater because it uses function and
# PARENT_SCOPE. Also depends on BundleUtilities.cmake.
#
#  WRITE_QT5_CONF(<qt_conf_dir> <qt_conf_contents>)
# Writes a qt.conf file with the <qt_conf_contents> into <qt_conf_dir>.
#
#  RESOLVE_QT5_PATHS(<paths_var> [<executable_path>])
# Loop through <paths_var> list and if any don't exist resolve them
# relative to the <executable_path> (if supplied) or the CMAKE_INSTALL_PREFIX.
#
#  FIXUP_QT5_EXECUTABLE(<executable> [<qtplugins> <libs> <dirs> <plugins_dir> <request_qt_conf>])
# Copies Qt plugins, writes a Qt configuration file (if needed) and fixes up a
# Qt5 executable using BundleUtilities so it is standalone and can be
# drag-and-drop copied to another machine as long as all of the system
# libraries are compatible.
#
# <executable> should point to the executable to be fixed-up.
#
# <qtplugins> should contain a list of the names or paths of any Qt plugins
# to be installed.
#
# <libs> will be passed to BundleUtilities and should be a list of any already
# installed plugins, libraries or executables to also be fixed-up.
#
# <dirs> will be passed to BundleUtilities and should contain and directories
# to be searched to find library dependencies.
#
# <plugins_dir> allows an custom plugins directory to be used.
#
# <request_qt_conf> will force a qt.conf file to be written even if not needed.
#
#  INSTALL_QT5_PLUGIN_PATH(plugin executable copy installed_plugin_path_var <plugins_dir> <component> <configurations>)
# Install (or copy) a resolved <plugin> to the default plugins directory
# (or <plugins_dir>) relative to <executable> and store the result in
# <installed_plugin_path_var>.
#
# If <copy> is set to TRUE then the plugins will be copied rather than
# installed. This is to allow this module to be used at CMake time rather than
# install time.
#
# If <component> is set then anything installed will use this COMPONENT.
#
#  INSTALL_QT5_PLUGIN(plugin executable copy installed_plugin_path_var <plugins_dir> <component>)
# Install (or copy) an unresolved <plugin> to the default plugins directory
# (or <plugins_dir>) relative to <executable> and store the result in
# <installed_plugin_path_var>. See documentation of INSTALL_QT5_PLUGIN_PATH.
#
#  INSTALL_QT5_EXECUTABLE(<executable> [<qtplugins> <libs> <dirs> <plugins_dir> <request_qt_conf> <component>])
# Installs Qt plugins, writes a Qt configuration file (if needed) and fixes up
# a Qt5 executable using BundleUtilities so it is standalone and can be
# drag-and-drop copied to another machine as long as all of the system
# libraries are compatible. The executable will be fixed-up at install time.
# <component> is the COMPONENT used for bundle fixup and plugin installation.
# See documentation of FIXUP_QT5_BUNDLE.

#=============================================================================
# Copyright 2011 Mike McQuaid <m...@mikemcquaid.com>
# Copyright 2013 Mihai Moldovan <io...@ionic.de>
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2011 Kitware, Inc., Insight Software Consortium
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# The functions defined in this file depend on the fixup_bundle function
# (and others) found in BundleUtilities.cmake

include(BundleUtilities)
SET(DeployQt5_cmake_dir "${CMAKE_CURRENT_LIST_DIR}")
SET(DeployQt5_apple_plugins_dir "PlugIns")

function(write_qt5_conf qt_conf_dir qt_conf_contents)
        SET(qt_conf_path "${qt_conf_dir}/qt.conf")
        MESSAGE(STATUS "Writing ${qt_conf_path}")
        file(WRITE "${qt_conf_path}" "${qt_conf_contents}")
endfunction()

function(resolve_qt5_paths paths_var)
        SET(executable_path ${ARGV1})

        SET(paths_resolved)
        FOREACH(path ${${paths_var}})
                if(EXISTS "${path}")
                        LIST(APPEND paths_resolved "${path}")
                ELSE()
                        if(${executable_path})
                                LIST(APPEND paths_resolved
"${executable_path}/${path}")
                        ELSE()
                                LIST(APPEND paths_resolved
"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${path}")
                        ENDIF()
                ENDIF()
        ENDFOREACH()
        SET(${paths_var} ${paths_resolved} PARENT_SCOPE)
endfunction()

function(fixup_qt5_executable executable)
        SET(qtplugins ${ARGV1})
        SET(libs ${ARGV2})
        SET(dirs ${ARGV3})
        SET(plugins_dir ${ARGV4})
        SET(request_qt_conf ${ARGV5})

        MESSAGE(STATUS "fixup_qt5_executable")
        MESSAGE(STATUS "  executable='${executable}'")
        MESSAGE(STATUS "  qtplugins='${qtplugins}'")
        MESSAGE(STATUS "  libs='${libs}'")
        MESSAGE(STATUS "  dirs='${dirs}'")
        MESSAGE(STATUS "  plugins_dir='${plugins_dir}'")
        MESSAGE(STATUS "  request_qt_conf='${request_qt_conf}'")

        if(QT_LIBRARY_DIR)
                LIST(APPEND dirs "${QT_LIBRARY_DIR}")
        ENDIF()
        if(QT_BINARY_DIR)
                LIST(APPEND dirs "${QT_BINARY_DIR}")
        ENDIF()

        if(APPLE)
                SET(qt_conf_dir "${executable}/Contents/Resources")
                SET(executable_path "${executable}")
                SET(write_qt_conf TRUE)
                if(NOT plugins_dir)
                        SET(plugins_dir "${DeployQt5_apple_plugins_dir}")
                ENDIF()
        ELSE()
                get_filename_component(executable_path "${executable}" PATH)
                if(NOT executable_path)
                        SET(executable_path ".")
                ENDIF()
                SET(qt_conf_dir "${executable_path}")
                SET(write_qt_conf ${request_qt_conf})
        ENDIF()

        FOREACH(plugin ${qtplugins})
                SET(installed_plugin_path "")
                install_qt5_plugin("${plugin}" "${executable}" 1
installed_plugin_path)
                LIST(APPEND libs ${installed_plugin_path})
        ENDFOREACH()

        FOREACH(lib ${libs})
                if(NOT EXISTS "${lib}")
                        MESSAGE(FATAL_ERROR "Library does not exist: ${lib}")
                ENDIF()
        ENDFOREACH()

        resolve_qt5_paths(libs "${executable_path}")

        if(write_qt_conf)
                SET(qt_conf_contents "[Paths]\nPlugins = ${plugins_dir}")
                write_qt5_conf("${qt_conf_dir}" "${qt_conf_contents}")
        ENDIF()

        fixup_bundle("${executable}" "${libs}" "${dirs}")
endfunction()

function(install_qt5_plugin_path plugin executable copy
installed_plugin_path_var)
        SET(plugins_dir ${ARGV4})
        SET(component ${ARGV5})
        SET(configurations ${ARGV6})
        if(EXISTS "${plugin}")
                if(APPLE)
                        if(NOT plugins_dir)
                                SET(plugins_dir
"${DeployQt5_apple_plugins_dir}")
                        ENDIF()
                        SET(plugins_path
"${executable}/Contents/${plugins_dir}")
                ELSE()
                        get_filename_component(plugins_path "${executable}"
PATH)
                        if(NOT plugins_path)
                                SET(plugins_path ".")
                        ENDIF()
                        if(plugins_dir)
                                SET(plugins_path
"${plugins_path}/${plugins_dir}")
                        ENDIF()
                ENDIF()

                SET(plugin_group "")

                get_filename_component(plugin_path "${plugin}" PATH)
                get_filename_component(plugin_parent_path "${plugin_path}" PATH)
                get_filename_component(plugin_parent_dir_name
"${plugin_parent_path}" NAME)
                get_filename_component(plugin_name "${plugin}" NAME)
                string(TOLOWER "${plugin_parent_dir_name}"
plugin_parent_dir_name)

                if("${plugin_parent_dir_name}" STREQUAL "plugins")
                        get_filename_component(plugin_group "${plugin_path}"
NAME)
                        SET(${plugin_group_var} "${plugin_group}")
                ENDIF()
                SET(plugins_path "${plugins_path}/${plugin_group}")

                if(${copy})
                        file(MAKE_DIRECTORY "${plugins_path}")
                        file(COPY "${plugin}" DESTINATION "${plugins_path}")
                ELSE()
                        if(configurations AND (CMAKE_CONFIGURATION_TYPES OR
CMAKE_BUILD_TYPE))
                                SET(configurations CONFIGURATIONS
${configurations})
                        ELSE()
                                unSET(configurations)
                        ENDIF()
                        install(FILES "${plugin}" DESTINATION "${plugins_path}"
${configurations} ${component})
                ENDIF()
                SET(${installed_plugin_path_var}
"${plugins_path}/${plugin_name}" PARENT_SCOPE)
        ENDIF()
endfunction()

function(install_qt5_plugin plugin executable copy installed_plugin_path_var)
        SET(plugins_dir ${ARGV4})
        SET(component ${ARGV5})
        if(EXISTS "${plugin}")
                install_qt5_plugin_path("${plugin}" "${executable}" "${copy}"
"${installed_plugin_path_var}" "${plugins_dir}" "${component}")
        ELSE()
                #string(TOUPPER "QT_${plugin}_PLUGIN" plugin_var)
                SET(plugin_release)
                SET(plugin_debug)
                SET(plugin_tmp_path)
                SET(plugin_find_path "${Qt5Core_DIR}/../../../plugins/")
                get_filename_component(plugin_find_path "${plugin_find_path}"
REALPATH)
                if(COMMAND cmake_policy)
                    CMAKE_POLICY(SET CMP0009 NEW)
                    #CMAKE_POLICY(SET CMP0011 NEW) # disabling a warning about policy changing in this scope
                ENDIF()

                IF(APPLE)
                    SET(plugin_find_release_filename "lib${plugin}.dylib")
                    SET(plugin_find_debug_filename "lib${plugin}_debug.dylib")
                    file(GLOB_RECURSE pluginlist "${plugin_find_path}" "${plugin_find_path}/*/lib*.dylib")
                ENDIF()
                if(WIN32)
                    SET(plugin_find_release_filename "${plugin}.dll")
                    SET(plugin_find_debug_filename "${plugin}d.dll")
                    file(GLOB_RECURSE pluginlist "${plugin_find_path}" "${plugin_find_path}/*/*.dll")
                ENDIF()
                FOREACH(found_plugin ${pluginlist})
                  get_filename_component(curname "${found_plugin}" NAME)
                  if("${curname}" STREQUAL "${plugin_find_release_filename}")
                    SET(plugin_tmp_release_path "${found_plugin}")
                  ENDIF()
                  if("${curname}" STREQUAL "${plugin_find_debug_filename}")
                    SET(plugin_tmp_debug_path "${found_plugin}")
                  ENDIF()
                ENDFOREACH()

                if((NOT DEFINED plugin_tmp_release_path OR NOT EXISTS
"${plugin_tmp_release_path}") AND (NOT DEFINED plugin_tmp_debug_PATH OR NOT
EXISTS "${plugin_tmp_debug_path}"))
                        MESSAGE(WARNING "Qt plugin \"${plugin}\" not recognized
or found.")
                ENDIF()

                if(EXISTS "${plugin_tmp_release_path}")
                  SET(plugin_release "${plugin_tmp_release_path}")
                elseif(EXISTS "${plugin_tmp_debug_path}")
                  SET(plugin_release "${plugin_tmp_debug_path}")
                ENDIF()

                if(EXISTS "${plugin_tmp_debug_path}")
                  SET(plugin_debug "${plugin_tmp_debug_path}")
                elseif(EXISTS "${plugin_tmp_release_path}")
                  SET(plugin_debug "${plugin_tmp_release_path}")
                ENDIF()

                if(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
                        install_qt5_plugin_path("${plugin_release}"
"${executable}" "${copy}" "${installed_plugin_path_var}_release"
"${plugins_dir}" "${component}" "Release|RelWithDebInfo|MinSizeRel")
                        install_qt5_plugin_path("${plugin_debug}"
"${executable}" "${copy}" "${installed_plugin_path_var}_debug" "${plugins_dir}"
"${component}" "Debug")

                        if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
                                SET(${installed_plugin_path_var}
${${installed_plugin_path_var}_debug})
                        ELSE()
                                SET(${installed_plugin_path_var}
${${installed_plugin_path_var}_release})
                        ENDIF()
                ELSE()
                        install_qt5_plugin_path("${plugin_release}"
"${executable}" "${copy}" "${installed_plugin_path_var}" "${plugins_dir}"
"${component}")
                ENDIF()
        ENDIF()
        SET(${installed_plugin_path_var} ${${installed_plugin_path_var}}
PARENT_SCOPE)
endfunction()

function(install_qt5_executable executable)
        SET(qtplugins ${ARGV1})
        SET(libs ${ARGV2})
        SET(dirs ${ARGV3})
        SET(plugins_dir ${ARGV4})
        SET(request_qt_conf ${ARGV5})
        SET(component ${ARGV6})
        if(QT_LIBRARY_DIR)
                LIST(APPEND dirs "${QT_LIBRARY_DIR}")
        ENDIF()
        if(QT_BINARY_DIR)
                LIST(APPEND dirs "${QT_BINARY_DIR}")
        ENDIF()
        if(component)
                SET(component COMPONENT ${component})
        ELSE()
                unSET(component)
        ENDIF()

        get_filename_component(executable_absolute "${executable}" ABSOLUTE)
        if(EXISTS "${QT_QTCORE_LIBRARY_RELEASE}")
            gp_file_type("${executable_absolute}"
"${QT_QTCORE_LIBRARY_RELEASE}" qtcore_type)
        elseif(EXISTS "${QT_QTCORE_LIBRARY_DEBUG}")
            gp_file_type("${executable_absolute}" "${QT_QTCORE_LIBRARY_DEBUG}"
qtcore_type)
        ENDIF()
        if(qtcore_type STREQUAL "system")
                SET(qt_plugins_dir "")
        ENDIF()

        if(QT_IS_STATIC)
                MESSAGE(WARNING "Qt built statically: not installing plugins.")
        ELSE()
                FOREACH(plugin ${qtplugins})
                        SET(installed_plugin_paths "")
                        install_qt5_plugin("${plugin}" "${executable}" 0
installed_plugin_paths "${plugins_dir}" "${component}")
                        LIST(APPEND libs ${installed_plugin_paths})
                ENDFOREACH()
        ENDIF()

        resolve_qt5_paths(libs "")

        install(CODE
  "include(\"${DeployQt5_cmake_dir}/DeployQt5.cmake\")
  SET(BU_CHMOD_BUNDLE_ITEMS TRUE)
  FIXUP_QT5_EXECUTABLE(\"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${executable}\"
\"\" \"${libs}\" \"${dirs}\" \"${plugins_dir}\" \"${request_qt_conf}\")"
                ${component}
        )
endfunction()
