
#include "DIA_flyDialog.h"
#include "ADM_image.h"
#include "DIA_flyLogo.h"

//************************
/**
    \fn upload
*/
uint8_t flyLogo::upload(void)
{

        return 1;
}
/**
        \fn download
*/
uint8_t flyLogo::download(void)
{

       
        return true;
}
/**
 * 
 * @param param
 * @param in
 * @return 
 */
bool DIA_getLogo(logo *param, ADM_coreVideoFilter *in)
{
    return true;
}

/**
    \fn process
*/
uint8_t    flyLogo::processYuv(ADMImage* in, ADMImage *out)
{
    return true;
}
