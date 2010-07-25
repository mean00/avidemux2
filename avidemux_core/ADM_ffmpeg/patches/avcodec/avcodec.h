diff --git a/avidemux_core/ADM_ffmpeg/libavcodec/avcodec.h b/avidemux_core/ADM_ffmpeg/libavcodec/avcodec.h
index 14e049d..78e39a5 100644
--- a/avidemux_core/ADM_ffmpeg/libavcodec/avcodec.h
+++ b/avidemux_core/ADM_ffmpeg/libavcodec/avcodec.h
@@ -614,6 +614,8 @@ typedef struct RcOverride{
 #define CODEC_FLAG2_PSY           0x00080000 ///< Use psycho visual optimizations.
 #define CODEC_FLAG2_SSIM          0x00100000 ///< Compute SSIM during encoding, error[] values are undefined.
 #define CODEC_FLAG2_INTRA_REFRESH 0x00200000 ///< Use periodic insertion of intra blocks instead of keyframes.
+//MEANX: NEVER EVER USE CLOSED GOP ?
+#define CODEC_FLAG2_32_PULLDOWN   0x80000000 
 
 /* Unsupported options :
  *              Syntax Arithmetic coding (SAC)
@@ -1496,6 +1498,7 @@ typedef struct AVCodecContext {
      * - decoding: unused
      */
     int rc_max_rate;
+    int rc_max_rate_header; /*< That one is set in the header MEANX */
 
     /**
      * minimum bitrate
@@ -1510,6 +1513,8 @@ typedef struct AVCodecContext {
      * - decoding: unused
      */
     int rc_buffer_size;
+    int rc_buffer_size_header;  /*< That one is set in the header MEANX*/
+
     float rc_buffer_aggressivity;
 
     /**
