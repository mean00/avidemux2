typedef struct
{
  const char *pipename;
  int hpipe;
  int flags;
} AVS_PIPES;

extern "C" void wine_tmp_path (AVS_PIPES *avsp, int num);
