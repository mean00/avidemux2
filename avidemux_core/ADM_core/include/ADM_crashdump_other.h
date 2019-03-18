#if !defined(__APPLE__) && !defined(__unix__) && !defined(__MINGW32__) && !defined ADM_CRASHDUMP_OTHER_H &&!defined _MSC_VER
#define ADM_CRASHDUMP_OTHER_H

#define installSigHandler(...) {}
#define uninstallSigHandler(...) {}

#endif
