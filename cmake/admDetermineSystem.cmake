# Determine CPU and Operating System for GCC
#  ADM_BIG_ENDIAN  - big endian CPU detected
#  ADM_BSD_FAMILY  - BSD family operating system was detected
#  ADM_CPU_ARMEL   - ARM (min ARCH5) el
#  ADM_CPU_64BIT   - 64-bit CPU was detected
#  ADM_CPU_X86     - x86 CPU architecture was detected
#  ADM_CPU_X86_32  - x86 32-bit CPU architecture was detected
#  ADM_CPU_X86_64  - x86 64-bit CPU architecture was detected

MACRO (PERFORM_SYSTEM_TEST testFile testName testSupportedVarName)
	IF (${ARGC} EQUAL 4)
		SET (testFlags ${ARGV3})
	ELSE (${ARGC} EQUAL 4)
		SET (testFlags "")
	ENDIF (${ARGC} EQUAL 4)

	ADM_COMPILE(${testFile} "${testFlags}" "" "" ${testSupportedVarName} outputTest)

	IF (${testSupportedVarName})
		SET(testResult "Yes")
	ELSE (${testSupportedVarName})
		SET(testResult "No")
	ENDIF (${testSupportedVarName})

	IF (${testSupportedVarName} OR VERBOSE)
		MESSAGE(STATUS "Checking if compiler supports ${testName} - ${testResult}")
	ENDIF (${testSupportedVarName} OR VERBOSE)

	IF (NOT ${testSupportedVarName} AND VERBOSE)
		MESSAGE("Error Message: ${outputTest}")
	ENDIF (NOT ${testSupportedVarName} AND VERBOSE)
ENDMACRO (PERFORM_SYSTEM_TEST)

INCLUDE(TestBigEndian)

MESSAGE(STATUS "Checking Compiler support")
MESSAGE(STATUS "*************************")

########################################
# Check OS support
########################################
IF (UNIX)
	PERFORM_SYSTEM_TEST(os_bsd_check.cpp "BSD family" BSD_SUPPORTED)

	IF (BSD_SUPPORTED)
		SET(ADM_BSD_FAMILY 1)
	ENDIF (BSD_SUPPORTED)
ENDIF (UNIX)

########################################
# Check CPU support
########################################
PERFORM_SYSTEM_TEST(cpu_x86-64_check.cpp "x86 64-bit" X86_64_SUPPORTED)

IF (X86_64_SUPPORTED)
	SET(ADM_CPU_X86 1)
	SET(ADM_CPU_X86_64 1)
	SET(ADM_CPU_64BIT 1)
ELSE (X86_64_SUPPORTED)
	PERFORM_SYSTEM_TEST(cpu_x86_check.cpp "x86 32-bit" X86_32_SUPPORTED)

	IF (X86_32_SUPPORTED)
		SET(ADM_CPU_X86 1)
		SET(ADM_CPU_X86_32 1)
	ELSE (X86_32_SUPPORTED)
		PERFORM_SYSTEM_TEST(cpu_armel_check.cpp "ARM EL" ARMEL_SUPPORTED)

        IF(ARMEL_SUPPORTED)
			SET(ADM_CPU_ARMEL 1)
        ENDIF(ARMEL_SUPPORTED)
	ENDIF (X86_32_SUPPORTED)
ENDIF (X86_64_SUPPORTED)

IF (NOT ADM_CPU_X86_32 AND NOT ADM_CPU_X86_64 AND NOT ADM_CPU_ARMEL)
	MESSAGE(FATAL_ERROR "CPU not supported")
ENDIF (NOT ADM_CPU_X86_32 AND NOT ADM_CPU_X86_64 AND NOT ADM_CPU_ARMEL)

TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

IF (CMAKE_WORDS_BIGENDIAN)
	SET(ADM_BIG_ENDIAN 1)
ENDIF (CMAKE_WORDS_BIGENDIAN)

MESSAGE("")
