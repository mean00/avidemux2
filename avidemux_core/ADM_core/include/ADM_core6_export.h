
#ifndef ADM_CORE6_EXPORT_H
#define ADM_CORE6_EXPORT_H

#ifdef ADM_CORE6_STATIC_DEFINE
#  define ADM_CORE6_EXPORT
#  define ADM_CORE6_NO_EXPORT
#else
#  ifndef ADM_CORE6_EXPORT
#    ifdef ADM_core6_EXPORTS
       /* We are building this library */
#      ifdef _WIN32
#        define ADM_CORE6_EXPORT __declspec(dllexport)
#      else
#        define ADM_CORE6_EXPORT __attribute__((visibility("default")))
#      endif
#    else
       /* We are using this library */
#      ifdef _WIN32
#        define ADM_CORE6_EXPORT __declspec(dllimport)
#      else
#        define ADM_CORE6_EXPORT __attribute__((visibility("default")))
#      endif
#    endif
#  endif

#  ifndef ADM_CORE6_NO_EXPORT
#    ifdef _WIN32
#      define ADM_CORE6_NO_EXPORT 
#    else
#      define ADM_CORE6_NO_EXPORT __attribute__((visibility("hidden")))
#    endif
#  endif
#endif

#ifndef ADM_CORE6_DEPRECATED
#  ifdef __GNUC__
#    define ADM_CORE6_DEPRECATED __attribute__ ((__deprecated__))
#    define ADM_CORE6_DEPRECATED_EXPORT ADM_CORE6_EXPORT __attribute__ ((__deprecated__))
#    define ADM_CORE6_DEPRECATED_NO_EXPORT ADM_CORE6_NO_EXPORT __attribute__ ((__deprecated__))
#  elif defined(_WIN32)
#    define ADM_CORE6_DEPRECATED __declspec(deprecated)
#    define ADM_CORE6_DEPRECATED_EXPORT ADM_core6_EXPORT __declspec(deprecated)
#    define ADM_CORE6_DEPRECATED_NO_EXPORT ADM_CORE6_NO_EXPORT __declspec(deprecated)
#  endif
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define ADM_CORE6_NO_DEPRECATED
#endif

#ifdef __GNUC__
#  define ADM_CORE6_NORETURN __attribute__((noreturn))
#elif defined(_WIN32)
#  define ADM_CORE6_NORETURN __declspec(noreturn)
#endif

#endif
