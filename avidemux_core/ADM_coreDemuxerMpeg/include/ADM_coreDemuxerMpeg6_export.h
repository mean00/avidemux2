
#pragma once
#  ifndef ADM_COREDEMUXER6_EXPORT
#    ifdef ADM_coreDemuxerMpeg6_EXPORTS
       /* We are building this library */
#      ifdef _WIN32
#        define ADM_COREDEMUXER6_EXPORT __declspec(dllexport)
#      else
#        define ADM_COREDEMUXER6_EXPORT __attribute__((visibility("default")))
#      endif
#    else
       /* We are using this library */
#      ifdef _WIN32
#        define ADM_COREDEMUXER6_EXPORT __declspec(dllimport)
#      else
#        define ADM_COREDEMUXER6_EXPORT __attribute__((visibility("default")))
#      endif
#    endif
#  endif


