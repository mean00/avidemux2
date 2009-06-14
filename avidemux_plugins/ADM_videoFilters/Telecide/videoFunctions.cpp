#include "ADM_inttype.h"

#define MAGIC_NUMBER (0xdeadbeef)

uint8_t PutHintingData(unsigned char *video, unsigned int hint)
{
	unsigned char *p;
	unsigned int i, magic_number = MAGIC_NUMBER;
	bool error = false;

	p = video;
	for (i = 0; i < 32; i++)
	{
		*p &= ~1; 
		*p++ |= ((magic_number & (1 << i)) >> i);
	}
	for (i = 0; i < 32; i++)
	{
		*p &= ~1;
		*p++ |= ((hint & (1 << i)) >> i);
	}
	return error;
}

uint8_t GetHintingData(unsigned char *video, unsigned int *hint)
{
	unsigned char *p;
	unsigned int i, magic_number = 0;
	bool error = false;

	p = video;
	for (i = 0; i < 32; i++)
	{
		magic_number |= ((*p++ & 1) << i);
	}
	if (magic_number != MAGIC_NUMBER)
	{
		error = true;
	}
	else
	{
		*hint = 0;
		for (i = 0; i < 32; i++)
		{
			*hint |= ((*p++ & 1) << i);
		}
	}
	return error;
}

void BitBlt(uint8_t* dstp, int dst_pitch, const uint8_t* srcp,
			int src_pitch, int row_size, int height)
{
	for(uint32_t y=0;y<height;y++)
	{
		memcpy(dstp,srcp,row_size);
		dstp+=dst_pitch;
		srcp+=src_pitch;
	}
}