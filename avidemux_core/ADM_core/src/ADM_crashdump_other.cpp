#include "ADM_crashdump.h"

#include <unistd.h>

void ADM_backTrack(const char *info, int lineno, const char *file) { exit(1); }
void ADM_setCrashHook(ADM_saveFunction *save, ADM_fatalFunction *fatal,ADM_sigIntFunction *other) { }
