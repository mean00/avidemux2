#if defined(__unix__) && !defined ADM_CRASHDUMP_UNIX_H
#define ADM_CRASHDUMP_UNIX_H

ADM_CORE6_EXPORT void installSigHandler(void);
ADM_CORE6_EXPORT void uninstallSigHandler(void);

#endif
