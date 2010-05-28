diff --git a/avidemux_core/ADM_ffmpeg/libavutil/lfg.c b/avidemux_core/ADM_ffmpeg/libavutil/lfg.c
index 1dad4e4..9b1db67 100644
--- a/avidemux_core/ADM_ffmpeg/libavutil/lfg.c
+++ b/avidemux_core/ADM_ffmpeg/libavutil/lfg.c
@@ -37,7 +37,7 @@ void av_cold av_lfg_init(AVLFG *c, unsigned int seed){
         c->state[i+2]= AV_RL32(tmp+8);
         c->state[i+3]= AV_RL32(tmp+12);
     }
-    c->index=0;
+    c->xindex=0;
 }
 
 void av_bmg_get(AVLFG *lfg, double out[2])
diff --git a/avidemux_core/ADM_ffmpeg/libavutil/lfg.h b/avidemux_core/ADM_ffmpeg/libavutil/lfg.h
index c952fc8..9648fce 100644
--- a/avidemux_core/ADM_ffmpeg/libavutil/lfg.h
+++ b/avidemux_core/ADM_ffmpeg/libavutil/lfg.h
@@ -23,7 +23,7 @@
 #define AVUTIL_LFG_H
 typedef struct {
     unsigned int state[64];
-    int index;
+    int xindex;
 } AVLFG;
 
 void av_lfg_init(AVLFG *c, unsigned int seed);
@@ -35,8 +35,8 @@ void av_lfg_init(AVLFG *c, unsigned int seed);
  * it may be good enough and faster for your specific use case.
  */
 static inline unsigned int av_lfg_get(AVLFG *c){
-    c->state[c->index & 63] = c->state[(c->index-24) & 63] + c->state[(c->index-55) & 63];
-    return c->state[c->index++ & 63];
+    c->state[c->xindex & 63] = c->state[(c->xindex-24) & 63] + c->state[(c->xindex-55) & 63];
+    return c->state[c->xindex++ & 63];
 }
 
 /**
@@ -45,9 +45,9 @@ static inline unsigned int av_lfg_get(AVLFG *c){
  * Please also consider av_lfg_get() above, it is faster.
  */
 static inline unsigned int av_mlfg_get(AVLFG *c){
-    unsigned int a= c->state[(c->index-55) & 63];
-    unsigned int b= c->state[(c->index-24) & 63];
-    return c->state[c->index++ & 63] = 2*a*b+a+b;
+    unsigned int a= c->state[(c->xindex-55) & 63];
+    unsigned int b= c->state[(c->xindex-24) & 63];
+    return c->state[c->xindex++ & 63] = 2*a*b+a+b;
 }
 
 /**
