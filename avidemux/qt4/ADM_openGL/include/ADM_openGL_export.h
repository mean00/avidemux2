/**

*/
#pragma once

#ifdef _WIN32
#  ifdef ADM_openGLQT56_EXPORTS
#        define ADM_OPENGL6_EXPORT __declspec(dllexport)
#  else
#        define ADM_OPENGL6_EXPORT __declspec(dllimport)
#  endif
#else
#  define ADM_OPENGL6_EXPORT
#endif
