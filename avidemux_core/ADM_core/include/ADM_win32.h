#if defined(_WIN32) && !defined ADM_WIN32_H
#define ADM_WIN32_H

uint8_t win32_netInit(void);
int shutdown_win32(void);
bool getWindowsVersion(char* version);
void redirectStdoutToFile(void);

int ansiStringToWideChar(const char *ansiString, int ansiStringLength, wchar_t *wideCharString);
int utf8StringToWideChar(const char *utf8String, int utf8StringLength, wchar_t *wideCharString);
int wideCharStringToUtf8(const wchar_t *wideCharString, int wideCharStringLength, char *utf8String);
int ansiStringToUtf8(const char *ansiString, int ansiStringLength, char *utf8String);

void getUtf8CommandLine(int *argc, char **argv[]);
void freeUtf8CommandLine(int argc, char *argv[]);

#endif
