# include "config.h"

#include <math.h>

#include "ADM_default.h"

#include "prefs.h"



#include "ADM_assert.h" 
#include "DIA_idx_pg.h"

DIA_progressIndexing::DIA_progressIndexing(const char *name)
{}
DIA_progressIndexing::~DIA_progressIndexing()
{}
 uint8_t       DIA_progressIndexing::update(uint32_t done,uint32_t total, uint32_t nbImage, uint32_t hh, uint32_t mm, uint32_t ss)
{
  return 1;
}
uint8_t       DIA_progressIndexing::abortRequest(void)
{
  return 0; 
}
uint8_t       DIA_progressIndexing::isAborted(void)
{
  return 0; 
}
