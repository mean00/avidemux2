//
// C++ Implementation: ADM_pp
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
//________________________________________________

//	PostProc : 1 Horiz deblock
//             2 Verti deblock
//             4 Dering
// strength between 0 and 5


#include "ADM_lavcodec.h"
#include "ADM_default.h"
#include "ADM_pp.h"


#define aprintf printf

void deletePostProc(ADM_PP *pp)
{
	aprintf("Deleting post proc\n");
	 if(pp->ppMode) {pp_free_mode(pp->ppMode);pp->ppMode=NULL;}
	 if(pp->ppContext) {pp_free_context(pp->ppContext);pp->ppContext=NULL;}

}
void updatePostProc(ADM_PP *pp )		    
{
char stringMode[60];
char stringFQ[60];

	stringMode[0]=0;
	deletePostProc(pp);
	aprintf("updating post proc\n");

	if(pp->postProcType&1) strcat(stringMode,"ha:a:128:7,");
	if(pp->postProcType&2) strcat(stringMode,"va:a:128:7,");
	if(pp->postProcType&4) strcat(stringMode,"dr:a,");
	if(pp->forcedQuant)  
		{
			sprintf(stringFQ,"fq:%d,",pp->forcedQuant);
			strcat(stringMode,stringFQ);
		}
			
	if(strlen(stringMode))  // something to do ?
		{
		uint32_t ppCaps=0;
		
#ifdef ADM_CPU_X86
		
	#define ADD(x,y) if( CpuCaps::has##x()) ppCaps|=PP_CPU_CAPS_##y;
		
		ADD(MMX,MMX);		
		ADD(3DNOW,3DNOW);
		ADD(MMXEXT,MMX2);
#endif		
#ifdef ADM_CPU_ALTIVEC
		ppCaps|=PP_CPU_CAPS_ALTIVEC;
#endif	
			pp->ppContext=pp_get_context(pp->w, pp->h, ppCaps
			   );		
			pp->ppMode=pp_get_mode_by_name_and_quality(
			stringMode, pp->postProcStrength);;
			ADM_assert(pp->ppMode);
			aprintf("Enabled type:%d strength:%d\n",
				pp->postProcType,pp->postProcStrength);
		}	   
	else    // if nothing is selected we may as well set back every thing to 0
		{
			pp->postProcStrength=0;
			aprintf("Disabled\n");
		}
}
//______________________________________________________________________________
 
void initPostProc(ADM_PP *pp,uint32_t w, uint32_t h)
{
	memset(pp,0,sizeof(ADM_PP));
	pp->w=w;
	pp->h=h;
        pp->swapuv=0;
	aprintf("Initializing postproc\n");
	

}
