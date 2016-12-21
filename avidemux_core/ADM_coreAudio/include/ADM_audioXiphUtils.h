/**
    \file  ADM_audioXiphUtils
    \brief Xiph lacing utilities

    \author copyright            : (C) 2016 by mean
 ***************************************************************************/
#pragma once


namespace ADMXiph
{

/**
 *  \fn extraData2packets
 *  \brief converts adm extradata to 3 packets. Packs and PackLen are3 elements arrays
 */    
ADM_COREAUDIO6_EXPORT bool  admExtraData2packets(uint8_t *extraData, int extraLen,uint8_t **packs,int *packLen);
/**
 * \fn admExtraData2xiph
 * @brief converts adm extra data style to xyph extra data style
 */
ADM_COREAUDIO6_EXPORT int   admExtraData2xiph(int l, uint8_t *src, uint8_t *dstOrg);

/**
 * \fn xiphExtraData2Adm
 * \brief converts xiph style extra data to adm style
 */
ADM_COREAUDIO6_EXPORT bool xiphExtraData2Adm(uint8_t *extraData, int extraLen,uint8_t **newExtra,int *newExtraLen)  ;  

}