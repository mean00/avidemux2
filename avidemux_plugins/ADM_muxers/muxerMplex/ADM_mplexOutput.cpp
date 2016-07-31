//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <sys/stat.h>
#include <errno.h>

#include "ADM_default.h"
#include "ADM_threads.h"
#include "interact.hpp"

#undef malloc
#undef realloc
#undef free




#include "cpu_accel.h"
#include "mjpeg_types.h"
#include "mjpeg_logging.h"
#include "mpegconsts.h"

#include "bits.hpp"
#include "outputstrm.hpp"
#include "multiplexor.hpp"

#include "ADM_inout.h"

extern admMutex mutex_slaveThread_problem;
extern admCond  *cond_slaveThread_problem;
extern char * kind_of_slaveThread_problem;
extern unsigned int kind_of_slaveThread_problem_rc;

FileOutputStream::FileOutputStream( const char *name_pat ) 
{
        strncpy( filename_pat, name_pat, MAXPATHLEN );
        snprintf( cur_filename, MAXPATHLEN, filename_pat, segment_num );
	strm_fd = -1;
}
      
int FileOutputStream::Open()
{
  char msg[512];
   while( !(strm = fopen( cur_filename, "wb" )) ){
      if( errno == ENOSPC
#ifndef __MINGW32__
                          || errno == EDQUOT
#endif
                                             ){
         ADM_assert(snprintf(msg,512,"can't open \"%s\": %s\n%s\n",
                             cur_filename,
                             (errno==ENOSPC?"filesystem full":"quota exceeded"),
                             "Please free up some space and press RETRY to try again.")!=-1);
         mutex_slaveThread_problem.lock();
           kind_of_slaveThread_problem = ADM_strdup(msg);
           cond_slaveThread_problem->wait(); /* implicit mutex_slaveThread_problem.unlock(); */
           ADM_dealloc(kind_of_slaveThread_problem);
           kind_of_slaveThread_problem = NULL;
         if( kind_of_slaveThread_problem_rc == 0 ){ /* ignore */
            /* it doesn't make any sense to continue */
            mjpeg_error_exit1( "Could not open for writing: %s", cur_filename );
         }
      }else{
         fprintf(stderr,"can't open \"%s\": %u (%s)\n", cur_filename, errno, strerror(errno));
         ADM_assert(0);
      }
   }
   strm_fd = fileno(strm);
   return 0;
}

void FileOutputStream::Close()
{ 
    fclose(strm);
    strm_fd = -1;
}


off_t
FileOutputStream::SegmentSize()
{
        struct stat stb;
    fstat(fileno(strm), &stb);
        off_t written = stb.st_size;
    return written;
}

void 
FileOutputStream::NextSegment( )
{
        Close();
        ++segment_num;
    
        cur_filename[strlen(cur_filename)-1]++; // increase
        Open();
}

void
FileOutputStream::Write( uint8_t *buf, unsigned int len )
{
  uint8_t *p = buf;
  unsigned int plen = len;
  int rc;
   ADM_assert(strm_fd != -1);
   while( (rc=write(strm_fd,p,plen)) != plen ){
      if( rc > 0 ){
         p+=rc;
         plen-=rc;
         continue;
      }
      if( rc == -1 && (errno == ENOSPC
#ifndef __MINGW32__
                                       || errno == EDQUOT
#endif
                                                          ) ){
        char msg[512];
         fprintf(stderr,"slaveThread: we have a problem. errno=%u\n",errno);
         ADM_assert(snprintf(msg,512,"can't write to file \"%s\": %s\n%s\n",
                             cur_filename,
                             (errno==ENOSPC?"filesystem full":"quota exceeded"),
                             "Please free up some space and press RETRY to try again.")!=-1);
         mutex_slaveThread_problem.lock();
           kind_of_slaveThread_problem = ADM_strdup(msg);
           cond_slaveThread_problem->wait(); /* implicit mutex_slaveThread_problem.unlock(); */
           ADM_dealloc(kind_of_slaveThread_problem);
           kind_of_slaveThread_problem = NULL;
         if( kind_of_slaveThread_problem_rc == 0 ){ /* ignore */
            /* it doesn't make any sense to continue */
            mjpeg_error_exit1( "Failed write: %s", cur_filename );
         }
      }else{
         mjpeg_error_exit1( "Failed write: %s", cur_filename );
      }
   }
}

