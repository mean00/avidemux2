#pragma once

#if 0
    #define aprintf(a,...) ADM_info(a,##__VA_ARGS__)
#else
    #define aprintf(...) {}
#endif
// EOF

