find_package(Git)
find_package(Tar)

if (NOT VERBOSE)
	set(ffmpegGitOutput OUTPUT_VARIABLE FFMPEG_GIT_OUTPUT)
	set(swscaleGitOutput OUTPUT_VARIABLE SWSCALE_GIT_OUTPUT)
endif (NOT VERBOSE)

# Checkout FFmpeg source and patch it
if (NOT EXISTS  "${FFMPEG_SOURCE_DIR}/${FFMPEG_VERSION}")
	message(STATUS "Checking out FFmpeg from git repository")
	execute_process(COMMAND ${GIT_EXECUTABLE} clone git://git.videolan.org/ffmpeg.git "${FFMPEG_SOURCE_DIR}"
					${ffmpegGitOutput})
	MESSAGE(STATUS "Going to revision ${FFMPEG_VERSION}")
	execute_process(COMMAND ${GIT_EXECUTABLE} checkout tags/n${FFMPEG_VERSION} -b ${FFMPEG_VERSION}
					WORKING_DIRECTORY "${FFMPEG_SOURCE_DIR}"
					${ffmpegSvnOutput})
	execute_process(COMMAND touch "${FFMPEG_SOURCE_DIR}/${FFMPEG_VERSION}")
	MESSAGE(STATUS "Archiving ffmpeg ${FFMPEG_VERSION}")
	MESSAGE(STATUS "${TAR_EXECUTABLE} cjf ${FFMPEG_ROOT_DIR}/${FFMPEG_SOURCE_ARCHIVE} --exclude .git source; DIR=${FFMPEG_BASE_DIR}")
	execute_process(COMMAND ${TAR_EXECUTABLE} cjf "${FFMPEG_ROOT_DIR}/${FFMPEG_SOURCE_ARCHIVE}" --exclude .git source
				WORKING_DIRECTORY "${FFMPEG_BASE_DIR}"
				)
	set(FFMPEG_PERFORM_PATCH 1)
endif (NOT EXISTS  "${FFMPEG_SOURCE_DIR}/${FFMPEG_VERSION}")

# Check version
IF(IS_DIRECTORY ${FFMPEG_SOURCE_DIR}/.git)
	admGetRevision( ${FFMPEG_SOURCE_DIR} ffmpeg_WC_REVISION)
	MESSAGE(STATUS "Current revision ${ffmpeg_WC_REVISION}")
ENDIF(IS_DIRECTORY ${FFMPEG_SOURCE_DIR}/.git)
message("")
