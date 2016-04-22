/***************************************************************************
    \file   ADM_audioAccessfileAACADTS_indexer
    \brief  build a map to seek into the file
    \author mean (c) 2016 fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define AAC_SEEK_PERIOD (10000000LL) // 10 sec

/**
 * \class adtsIndexer
 * @param fd
 */
class adtsIndexer
{
public:
    
            adtsIndexer(FILE *fd,int fq,int chan)
            {
                f=fd;
                this->fq=fq;
                this->channels=chan;
                payload=0;
                nbPackets=0;
            }
           
    virtual ~adtsIndexer()
            {
                
            }
    int     getPayloadSize() {return payload;}    
    bool    index(std::vector<aacAdtsSeek> &seekPoints);    
    int     getNbPackets() {return nbPackets;}
    
protected:
    FILE *f;
    int  fq;
    int  channels;
    int  payload;
    int  nbPackets;
    
};
/**
 * \fn index
 * @param seekPoints
 * @return 
 */
bool adtsIndexer::index(std::vector<aacAdtsSeek>& seekPoints)
{
#define CHUNK_SIZE (5*1024)
   uint8_t  buffer[CHUNK_SIZE];
   int      fileOffset=0;;
   uint64_t lastPoint=0;
   int      len;
   
   audioClock   clk(fq);
   ADM_adts2aac aac;
   aacAdtsSeek  start;
   int offset;

   int      beginningOfBuffer=0;
   int      endOfBuffer=0;
   
   start.dts=0;
   start.position=0;
   seekPoints.push_back(start); // initial start
   
   while(1)
   {
       ADM_adts2aac::ADTS_STATE s=aac.getAACFrame(&len,buffer,&offset);
       switch(s)
       {
           case ADM_adts2aac::ADTS_OK:
            {
               uint64_t currentPoint=clk.getTimeUs();
               if( (currentPoint-lastPoint)>AAC_SEEK_PERIOD) // one seek point every 10 s
               {
                   start.dts=currentPoint;
                   start.position=offset;
                   seekPoints.push_back(start);
                   lastPoint=currentPoint;   
               }
                payload+=len;
                clk.advanceBySample(1024);
                nbPackets++;
                //printf("Found %d packet, len is %d, offset is %d, time is %s\n",nbPackets,len,offset,ADM_us2plain(clk.getTimeUs()));
                continue;
                break;
            }
           case ADM_adts2aac::ADTS_ERROR:
           {
               return true;
               break;
           }
           case ADM_adts2aac::ADTS_MORE_DATA_NEEDED:
           {
              
               int n=fread(buffer,1,CHUNK_SIZE,f);
               if(n<=0)
                   return true;
               beginningOfBuffer=endOfBuffer;
               endOfBuffer+=n;
               aac.addData(n,buffer);
               break;
           }
           default:
               ADM_assert(0);
               break;
       }
    
   }
   return true; 
}
// EOF

