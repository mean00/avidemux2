diff --git a/avidemux_core/ADM_ffmpeg/libavutil/lfg.h b/avidemux_core/ADM_ffmpeg/libavutil/lfg.h
index ac89d12..56e86a1 100644
--- a/avidemux_core/ADM_ffmpeg/libavutil/lfg.h
+++ b/avidemux_core/ADM_ffmpeg/libavutil/lfg.h
@@ -21,7 +21,7 @@
 
 #ifndef AVUTIL_LFG_H
 #define AVUTIL_LFG_H
-
+#define index xindex // MEANX
 typedef struct {
     unsigned int state[64];
     int index;
