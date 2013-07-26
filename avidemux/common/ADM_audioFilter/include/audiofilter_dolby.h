//
// C++ Interface: audiofilter_dolby
//
// Description: 
//
//
// Author: Mihail Zenkov <kreator@tut.by>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef AUDM_DOLBY_H
#define AUDM_DOLBY_H


#define NZEROS 500
#define GAIN 1.571116176e+00

/**
 * \class  ADMDolbyContext
 */
class ADMDolbyContext
{
public:    
        ADMDolbyContext()
        {
            for(int i=0;i<=NZEROS;i++)
            {
                xv_left[i]=xv_right[i]=0;
            }
            posLeft=posRight=0;
        }
        float xv_left[NZEROS+1];
        float xv_right[NZEROS+1];
        static bool  skip;
        int    posLeft;
        int    posRight;
        
        float DolbyShiftLeft(float isamp);
        float DolbyShiftRight(float isamp);
        void  DolbyInit();
        static void  DolbySkip(bool on);
};

#endif
