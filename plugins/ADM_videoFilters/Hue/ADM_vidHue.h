/*

*/

typedef struct Hue_Param
{
  float     hue ;           /* -180 to 180 */
  float     saturation;     /* -180 to 180 */
}Hue_Param;
 void HueProcess_C(uint8_t *udst, uint8_t *vdst, uint8_t *usrc, uint8_t *vsrc, int dststride, int srcstride,
		    int w, int h, float hue, float sat);
