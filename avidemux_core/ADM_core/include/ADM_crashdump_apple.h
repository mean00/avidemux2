#if defined(__APPLE__) && !defined ADM_CRASHDUMP_APPLE_H
#define ADM_CRASHDUMP_APPLE_H

void installSigHandler(void);
void uninstallSigHandler(void);

#endif
