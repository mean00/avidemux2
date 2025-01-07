/**

*/
#pragma once

#ifdef _WIN32
#  if defined(ADM_openGLQT66_EXPORTS) || defined(ADM_openGLQT56_EXPORTS)
#        define ADM_OPENGL6_EXPORT __declspec(dllexport)
#  else
#        define ADM_OPENGL6_EXPORT __declspec(dllimport)
#  endif
#else
#  define ADM_OPENGL6_EXPORT
#endif
