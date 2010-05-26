diff --git a/avidemux_core/ADM_ffmpeg/libavutil/x86/bswap.h b/avidemux_core/ADM_ffmpeg/libavutil/x86/bswap.h
index 26dc4e2..c7a87dd 100644
--- a/avidemux_core/ADM_ffmpeg/libavutil/x86/bswap.h
+++ b/avidemux_core/ADM_ffmpeg/libavutil/x86/bswap.h
@@ -25,7 +25,11 @@
 #define AVUTIL_X86_BSWAP_H
 
 #include <stdint.h>
+// MEANX
+#ifndef ADM_NO_CONFIG_H
 #include "config.h"
+#endif
+// /MEANX
 #include "libavutil/attributes.h"
 
 #define bswap_16 bswap_16
