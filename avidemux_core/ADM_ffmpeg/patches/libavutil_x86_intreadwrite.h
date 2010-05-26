diff --git a/avidemux_core/ADM_ffmpeg/libavutil/x86/intreadwrite.h b/avidemux_core/ADM_ffmpeg/libavutil/x86/intreadwrite.h
index 4061d19..4740886 100644
--- a/avidemux_core/ADM_ffmpeg/libavutil/x86/intreadwrite.h
+++ b/avidemux_core/ADM_ffmpeg/libavutil/x86/intreadwrite.h
@@ -22,7 +22,11 @@
 #define AVUTIL_X86_INTREADWRITE_H
 
 #include <stdint.h>
+// MEANX
+#ifndef ADM_NO_CONFIG_H
 #include "config.h"
+#endif
+/// MEANX
 #include "libavutil/attributes.h"
 
 #if HAVE_MMX
