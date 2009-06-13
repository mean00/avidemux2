/*
	Some useful functions to manipulate fields


*/

void vidFieldDecimate(uint8_t *src,uint8_t *target, uint32_t linessrc, uint32_t width);
void vidFieldKeepOdd(uint32_t w,uint32_t h,uint8_t *src,uint8_t *target);
void vidFieldKeepEven(uint32_t w,uint32_t h,uint8_t *src,uint8_t *target);
void vidFieldMerge(uint32_t w,uint32_t h,uint8_t *src,uint8_t *src2,uint8_t *target);
uint8_t vidFielStack(uint32_t w,uint32_t h,uint8_t *src,uint8_t *target);
uint8_t vidFielUnStack(uint32_t w,uint32_t h,uint8_t *src,uint8_t *target);
