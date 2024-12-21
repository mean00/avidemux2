/**
 * \file ADM_crashdump_mingw
 * \brief Catch low level error, mingw version
 */
#if defined(_MSC_VER) 

#pragma once
ADM_CORE6_EXPORT void installSigHandler(void);
ADM_CORE6_EXPORT void uninstallSigHandler(void);

#endif
