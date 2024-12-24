FIND_PACKAGE(Subversion)

IF(NOT VERBOSE)
  SET(ffmpegSvnOutput OUTPUT_VARIABLE FFMPEG_SVN_OUTPUT)
  SET(swscaleSvnOutput OUTPUT_VARIABLE SWSCALE_SVN_OUTPUT)
ENDIF()

# Checkout FFmpeg source and patch it
IF(NOT IS_DIRECTORY "${FFMPEG_SOURCE_DIR}/.svn")
  MESSAGE(STATUS "Checking out FFmpeg")
  EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} co svn://svn.ffmpeg.org/ffmpeg/trunk -r ${FFMPEG_VERSION} --ignore-externals "${FFMPEG_SOURCE_DIR}"
          ${ffmpegSvnOutput})
  MESSAGE(STATUS "Archiving ffmpeg ${FFMPEG_VERSION}")
  MESSAGE(STATUS "${TAR_EXECUTABLE} czf ${FFMPEG_ROOT_DIR}/${FFMPEG_SOURCE_ARCHIVE} ffmpeg, DIR=${FFMPEG_SOURCE_DIR}/..")
  EXECUTE_PROCESS(COMMAND ${TAR_EXECUTABLE} czf "${FFMPEG_ROOT_DIR}/${FFMPEG_SOURCE_ARCHIVE}" source
        WORKING_DIRECTORY "${FFMPEG_BASE_DIR}"
        )

  MESSAGE(STATUS "Checking out libswscale")
  EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} co svn://svn.ffmpeg.org/mplayer/trunk/libswscale -r ${SWSCALE_VERSION} "${FFMPEG_SOURCE_DIR}/libswscale"
          ${swscaleSvnOutput})
  MESSAGE(STATUS "Archiving libswscale ${SWSCALE_VERSION}")
  MESSAGE(STATUS " ${TAR_EXECUTABLE} czf ${FFMPEG_ROOT_DIR}/${SWSCALE_SOURCE_ARCHIVE} libswscale,  DIR=${FFMPEG_SOURCE_DIR}/" )
  EXECUTE_PROCESS(COMMAND ${TAR_EXECUTABLE} czf "${FFMPEG_ROOT_DIR}/${SWSCALE_SOURCE_ARCHIVE}" libswscale
        WORKING_DIRECTORY "${FFMPEG_SOURCE_DIR}/"
        )

  SET(FFMPEG_PERFORM_PATCH 1)
ENDIF()

# Check version
admGetRevision( ${FFMPEG_SOURCE_DIR} ffmpeg_WC_REVISION)
MESSAGE(STATUS "FFmpeg revision: ${ffmpeg_WC_REVISION}")

IF(NOT ${ffmpeg_WC_REVISION} EQUAL ${FFMPEG_VERSION})
  MESSAGE(STATUS "Updating to revision ${FFMPEG_VERSION}")
  SET(FFMPEG_PERFORM_BUILD 1)
  SET(FFMPEG_PERFORM_PATCH 1)

  EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} revert -R "${FFMPEG_SOURCE_DIR}"
          ${ffmpegSvnOutput})
  EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} up -r ${FFMPEG_VERSION} --ignore-externals "${FFMPEG_SOURCE_DIR}"
          ${ffmpegSvnOutput})

  MESSAGE("")
ENDIF()

MESSAGE("")
admGetRevision( ${FFMPEG_SOURCE_DIR}/libswscale swscale_WC_REVISION)
MESSAGE(STATUS "libswscale revision: ${swscale_WC_REVISION}")

IF(NOT ${swscale_WC_REVISION} EQUAL ${SWSCALE_VERSION})
  MESSAGE(STATUS "Updating to revision ${SWSCALE_VERSION}")
  SET(FFMPEG_PERFORM_BUILD 1)
  SET(FFMPEG_PERFORM_PATCH 1)

  EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} revert -R "${FFMPEG_SOURCE_DIR}/libswscale"
          ${swscaleSvnOutput})
  EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} up -r ${SWSCALE_VERSION} "${FFMPEG_SOURCE_DIR}/libswscale"
          ${swscaleSvnOutput})
ENDIF()
