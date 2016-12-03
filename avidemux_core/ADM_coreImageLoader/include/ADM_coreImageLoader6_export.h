#pragma once

#ifdef ADM_COREIMAGELOADER6_STATIC_DEFINE
#  define ADM_COREIMAGELOADER6_EXPORT
#  define ADM_COREIMAGELOADER6_NO_EXPORT
#else
#  ifdef ADM_coreImageLoader6_EXPORTS
       /* We are building this library */
#      ifdef _WIN32
#        define ADM_COREIMAGELOADER6_EXPORT __declspec(dllexport)
#      else
#        define ADM_COREIMAGELOADER6_EXPORT __attribute__((visibility("default")))
#      endif
#    else
       /* We are using this library */
#      ifdef _WIN32
#        define ADM_COREIMAGELOADER6_EXPORT __declspec(dllimport)
#      else
#        define ADM_COREIMAGELOADER6_EXPORT __attribute__((visibility("default")))
#      endif
#    endif
#endif

