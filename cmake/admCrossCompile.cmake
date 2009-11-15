
if(CROSS)
        MESSAGE(STATUS " ** Win32 Cross compiling activated, overriding cmake internals **")
        # the name of the target operating system
        SET(CMAKE_SYSTEM_NAME Windows)
        SET(PFIX  $ENV{CROSS_PREFIX})        
        # which compilers to use for C and C++
        SET(CMAKE_C_COMPILER ${PFIX}-gcc)
        SET(CMAKE_CXX_COMPILER ${PFIX}-g++)
        SET(CMAKE_LINKER ${PFIX}-ld)
        SET(CMAKE_AR ${PFIX}-ar)
        
        # here is the target environment located
        SET(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc ${CROSS} )
        
        # adjust the default behaviour of the FIND_XXX() commands:
        # search headers and libraries in the target environment, search 
        # programs in the host environment
        set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif(CROSS)

