#if defined(_WIN32) && !defined ADM_WIN32_H
#define ADM_WIN32_H

#include "ADM_core6_export.h"
#include <string>
#include "ADM_inttype.h"

ADM_CORE6_EXPORT uint8_t win32_netInit(void);
int shutdown_win32(void);
ADM_CORE6_EXPORT bool getWindowsVersion(char* version);
ADM_CORE6_EXPORT void redirectStdoutToFile(void);

int ansiStringToWideChar(const char *ansiString, int ansiStringLength, wchar_t *wideCharString);
ADM_CORE6_EXPORT int utf8StringToWideChar(const char *utf8String, int utf8StringLength, wchar_t *wideCharString);
int wideCharStringToUtf8(const wchar_t *wideCharString, int wideCharStringLength, char *utf8String);
int ansiStringToUtf8(const char *ansiString, int ansiStringLength, char *utf8String);
ADM_CORE6_EXPORT  std::string utf8StringToAnsi(const char *utf8String);

ADM_CORE6_EXPORT void getUtf8CommandLine(int *argc, char **argv[]);
ADM_CORE6_EXPORT void freeUtf8CommandLine(int argc, char *argv[]);

#endif
