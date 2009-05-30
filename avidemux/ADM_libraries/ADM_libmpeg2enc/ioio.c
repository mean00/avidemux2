
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "ADM_default.h""
#include "mjpeg_logging.h"

#define SILENT

void mjpeg_log(log_level_t level, const char format[], ...)
{
#ifndef SILENT
 static char print_buffer[1024];
  	va_list 	list;
		va_start(list,	format);
		vsnprintf(print_buffer,1023,format,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
		printf("%s\n",print_buffer);
#endif

}
void mjpeg_info( const char format[], ...)
{
#if !defined( SILENT) || 0
  static char print_buffer[1024];
  	va_list 	list;
		va_start(list,	format);
		vsnprintf(print_buffer,1023,format,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
		printf("%s\n",print_buffer);
#endif
}
void mjpeg_warn( const char format[], ...)
{
#if  !defined( SILENT) || 1
 static char print_buffer[1024];
  	va_list 	list;
		va_start(list,	format);
		vsnprintf(print_buffer,1023,format,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
		printf("[MPLEX]%s\n",print_buffer);
#endif

}
void mjpeg_debug( const char format[], ...)
{
#ifndef SILENT
 static char print_buffer[1024];
  	va_list 	list;
		va_start(list,	format);
		vsnprintf(print_buffer,1023,format,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
		printf("%s\n",print_buffer);
#endif

}
void mjpeg_error( const char format[], ...)
{

 static char print_buffer[1024];
  	va_list 	list;
		va_start(list,	format);
		vsnprintf(print_buffer,1023,format,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
		printf("%s\n",print_buffer);
		printf("\n");

}
void
mjpeg_error_exit1(const char format[], ...)
{

 static char print_buffer[1024];
  	va_list 	list;
		va_start(list,	format);
		vsnprintf(print_buffer,1023,format,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
		printf("%s\n",print_buffer);

	exit(-1);

}
int mjpeg_default_handler_verbosity(int verbosity)
{


}


