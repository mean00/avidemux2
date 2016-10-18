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
ADM_COREAUDIO6_EXPORT bool  extraData2packets(uint8_t *extraData, int extraLen,uint8_t **packs,int *packLen);

}