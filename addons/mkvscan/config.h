#define ADM_assert(x) if(!(x)) { printf("Assert "#x" failed at line %d file %s\n",__LINE__,__FILE__);exit(-1);}
