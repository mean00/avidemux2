#include <vector>
#include <string>
using std::vector;
using std::string;

#include "ADM_default.h"
#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_indexFile.h"
#include "ADM_string.h"

#include <math.h>
#define TS_MAX_LINE 10000
#include "ADM_ts.h"

extern uint8_t ADM_InitMemcpy(void);
int main(int argc,char **argv)
{
    ADM_initBaseDir(argc,argv);
    ADM_InitMemcpy();


    if(argc!=2)
    {
        printf("dumpIndex xxx.ts.idx2\n");
        exit(1);
    }
    char *name=argv[1];
    tsHeader ts;
    indexFile idx;
    
    if(!idx.open(name))
    {
        printf("Cannot open file <%s>\n",name);
        exit(-1);
    }
    ts.readIndex(&idx);
    idx.close();
    return 0;
    
    
}
