#if defined(unix) && !defined ADM_CRASHDUMP_UNIX_H
#define ADM_CRASHDUMP_UNIX_H

void installSigHandler(void);
#define uninstallSigHandler()

#endif
