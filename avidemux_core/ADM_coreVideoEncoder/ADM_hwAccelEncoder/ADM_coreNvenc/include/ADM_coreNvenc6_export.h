
#ifndef ADM_CORENVENC6_EXPORT_H
#define ADM_CORENVENC6_EXPORT_H

#ifdef ADM_CORENVENC6_STATIC_DEFINE
#define ADM_CORENVENC6_EXPORT
#define ADM_CORENVENC6_NO_EXPORT
#else
#ifndef ADM_CORENVENC6_EXPORT
#ifdef ADM_coreNvenc6_EXPORTS
/* We are building this library */
#ifdef _WIN32
#define ADM_CORENVENC6_EXPORT __declspec(dllexport)
#else
#define ADM_CORENVENC6_EXPORT __attribute__((visibility("default")))
#endif
#else
/* We are using this library */
#ifdef _WIN32
#define ADM_CORENVENC6_EXPORT __declspec(dllimport)
#else
#define ADM_CORENVENC6_EXPORT __attribute__((visibility("default")))
#endif
#endif
#endif

#ifndef ADM_CORENVENC6_NO_EXPORT
#ifdef _WIN32
#define ADM_CORENVENC6_NO_EXPORT
#else
#define ADM_CORENVENC6_NO_EXPORT __attribute__((visibility("hidden")))
#endif
#endif
#endif

#endif
