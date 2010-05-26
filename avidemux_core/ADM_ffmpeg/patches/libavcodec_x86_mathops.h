diff --git a/avidemux_core/ADM_ffmpeg/libavcodec/x86/mathops.h b/avidemux_core/ADM_ffmpeg/libavcodec/x86/mathops.h
index 5949dfe..c7f54c5 100644
--- a/avidemux_core/ADM_ffmpeg/libavcodec/x86/mathops.h
+++ b/avidemux_core/ADM_ffmpeg/libavcodec/x86/mathops.h
@@ -22,7 +22,12 @@
 #ifndef AVCODEC_X86_MATHOPS_H
 #define AVCODEC_X86_MATHOPS_H
 
+// MEANX
+#ifndef ADM_NO_CONFIG_H
 #include "config.h"
+#endif
+// MEANX
+
 #include "libavutil/common.h"
 
 #if ARCH_X86_32
