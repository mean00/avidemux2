#if defined(__APPLE__) && !defined ADM_CRASHDUMP_APPLE_H
#define ADM_CRASHDUMP_APPLE_H

ADM_CORE6_EXPORT void installSigHandler(void);
ADM_CORE6_EXPORT void uninstallSigHandler(void);

#endif
