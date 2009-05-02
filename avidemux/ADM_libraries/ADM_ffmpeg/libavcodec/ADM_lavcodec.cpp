#include "ADM_default.h"
#ifdef ADM_CPU_X86
#include "ADM_lavcodec.h"
#include "ADM_libraries/ADM_ffmpeg/ADM_lavcodec/dsputil_cpu.h"
#endif
/**
 * 		\fn lavcodec_mm_support
 * 		\brief Give lavcodec CPU supported ( FF_MM_MMX)
 */
extern "C"
{
int ADM_lavcodec_mm_support(void);
}
int ADM_lavcodec_mm_support(void)
{
int rval=0;

#ifdef ADM_CPU_X86
#undef MATCH
#define MATCH(x,y) if(CpuCaps::myCpuCaps &  CpuCaps::myCpuMask & ADM_CPUCAP_##x) rval|=MM_##x;

	MATCH(MMX,MMX);
	MATCH(MMXEXT,MMXEXT);
	MATCH(SSE,SSE);
	MATCH(SSE2,SSE2);
	MATCH(SSE3,SSE3);
	MATCH(SSSE3,SSSE3);
	MATCH(3DNOW,3DNOW);
	MATCH(3DNOWEXT,3DNOWEXT);
#endif

	return rval;
}
// EOF
