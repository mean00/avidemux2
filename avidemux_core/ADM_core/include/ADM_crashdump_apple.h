#if defined(APPLE) && !defined ADM_CRASHDUMP_APPLE_H
#define ADM_CRASHDUMP_APPLE_H

void installSigHandler(void);
#define uninstallSigHandler()

#endif
