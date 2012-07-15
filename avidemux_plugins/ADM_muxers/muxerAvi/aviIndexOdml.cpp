                
/** *************************************************************************
        \file op_aviwrite.cpp
        \brief low level avi muxer

		etc...


    copyright            : (C) 2002-2012 by mean
                           (C) Feb 2005 by GMV: ODML write support
    GPL V2.0
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "vector"
#include "aviIndexOdml.h"
#include "op_aviwrite.hxx"
#include "fourcc.h"
#define aprintf(...) {}

/**
    \fn
*/
aviIndexOdml::aviIndexOdml(aviWrite *father ): aviIndexBase(father)  
{
  
}
/**
    \fn
*/

aviIndexOdml::~aviIndexOdml() 
{

}
/**
    \fn addVideoFrame
*/
bool  aviIndexOdml::addVideoFrame(int len,uint32_t flags,const uint8_t *data)
{
#if 0
      IdxEntry entry;
      uint64_t offset=_father->LMovie->Tell () - 8 - _father->LMovie->TellBegin ();

	// write initial index chunks
	if(vframe==2 && doODML!=NO)
    {	// apparently some players require a video frame at first in the movi list, so we put the initial index dummys behind it (bye bye index before data)
		odml_write_dummy_chunk(LMovie, &(odml_indexes[0].odml_index[0].fpos), 24+8*odml_index_size);
        for(int i=1;i<odml_nbrof_streams;i++)
			odml_write_dummy_chunk(LMovie, &(odml_indexes[i].odml_index[0].fpos), 24+8*odml_index_size);
	}
	// test for new riff
	odml_riff_break(len+8); // data size + fcc + size info (padding is handled in odml_riff_break)
	// index frame
	if(!odml_index_frame(0, len,flags&AVI_KEY_FRAME))
    {
		aprintf("\ncan not index video frame %u\n",vframe);
	}

      entry.fcc = fourccs[0];
      entry.len = len;
      entry.flags = flags;
      entry.offset = offset;
      myIndex.push_back(entry);
    
      _father->LMovie->WriteChunk (entry.fcc, len, data);
#endif
      return true;
}
/**
    \fn addAudioFrame
*/

bool  aviIndexOdml::addAudioFrame(int trackNo,int len,uint32_t flags,const uint8_t *data)
{
#if 0
 IdxEntry entry;
    uint64_t offset=_father->LMovie->Tell () - 8 - _father->LMovie->TellBegin ();
      entry.fcc = entry.fcc = fourccs[trackNo+1];
      entry.len = len;
      entry.flags = flags;
      entry.offset = offset;
      myIndex.push_back(entry);
      _father->LMovie->WriteChunk (entry.fcc, len, data);
      return true;
#endif
}
/**
    \fn writeIndex
*/

bool  aviIndexOdml::writeIndex()
{
#if 0
        #define ix32(a)  memIo.write32(myIndex[i].a)
            ADM_info("Writing type 1 Avi index\n");
            // Write index
            int curindex=myIndex.size();
            _father->LAll->Write32 ("idx1");
            _father->LAll->Write32 (curindex * 16);
            ADMMemio memIo(4*4);
            for (uint32_t i = 0; i < curindex; i++)
            {
                memIo.reset();
                ix32(fcc);ix32(flags);ix32(offset);ix32(len);
                _father->LAll->WriteMem (memIo);
            }
            ADM_info("Done writing type 1 Avi index\n");
            return true;
#endif
}
#if 0
/**
        \fn odml_destroy_index
*/
void aviWrite::odml_destroy_index(void){
	// destroy odml index data structure
	if(doODML!=NO){
		if(odml_indexes){
			for(int a=0;a<odml_nbrof_streams;++a){
				if(odml_indexes[a].odml_index){
					for(int b=0;b<odml_indexes[a].odml_nbrof_index;++b)
                                        {
						if(odml_indexes[a].odml_index[b].index)
							ADM_dealloc (odml_indexes[a].odml_index[b].index);
					}
					ADM_dealloc (odml_indexes[a].odml_index);
				}
			}
			ADM_dealloc (odml_indexes);
		}
		odml_indexes=NULL;
	}
}
/**
        \fn odml_write_dummy_chunk
*/
void aviWrite::odml_write_dummy_chunk(AviList* alist, uint64_t* fpos, uint32_t size){
	if(doODML!=NO){
		// save file position
		*fpos=alist->Tell();
		aprintf("[ODML]write dummy chunk at file position %"LLU" with data size %"LU"\n",*fpos, size);
		// generate dummy data
		uint8_t* dummy=(uint8_t*)ADM_alloc (size);
		memset(dummy,0,size);
		// write dummy chunk
		alist->WriteChunk ((uint8_t *) "JUNK", size, dummy);
		// clean up
		ADM_dealloc (dummy);
	}
}
/**
        \fn reallocIndeces
*/
void aviWrite::reallocIndeces( odml_super_index_t *idx)
{
    uint32_t nw,old;
    odml_index_t   *newindex;
    odml_index_t *oldindex;
            old=idx->odml_nbrof_index;
            nw=old*2;
            printf("Increasing # of indeces from %d to %d\n",old,nw);
            oldindex=idx->odml_index;
            newindex=(odml_index_t *)ADM_alloc(sizeof(odml_index_t)*nw);
            memset(newindex,0,sizeof(odml_index_t)*nw);
            memcpy(newindex,oldindex,old*sizeof(odml_index_t));
            idx->odml_index=newindex;
            ADM_dealloc(oldindex);
            idx->odml_nbrof_index=nw;
            // Now fill in the new
            uint32_t lineSize=sizeof(odml_index_data_t) * odml_index_size;
             for(int b=old;b<nw;++b)
                        {	// for each index, alloc
                            newindex[b].index=(odml_index_data_t*) ADM_alloc (lineSize);
                            memset(newindex[b].index,0,lineSize);
                            newindex[b].nEntriesInUse=0;	// (redundant)
                        }

}


/**
        \fn odml_index_frame
*/
bool aviWrite::odml_index_frame(int stream_nbr, uint32_t data_size, bool keyFrame)
#if 0
{

	if(doODML!=NO){
//		ADM_assert(!stream_nbr<odml_nbrof_streams);
		odml_super_index_t* sidx=odml_indexes+stream_nbr;	// access to super index
		if(sidx->odml_index[sidx->index_count].nEntriesInUse==odml_index_size)
                {	// new index needed?
			if(sidx->index_count>=sidx->odml_nbrof_index-1)	// can index counter be increased?
                                reallocIndeces(sidx);
                        ++(sidx->index_count);	// increment index counter

			// handle possible riff break
			odml_riff_break(data_size+8); // data size + fcc + size info (padding is handled in odml_riff_break)
			// write placeholder
			odml_write_dummy_chunk(LMovie, &(sidx->odml_index[sidx->index_count].fpos), 24+8*odml_index_size);
			sidx->odml_index[sidx->index_count].nEntriesInUse=0;
		}
		odml_index_t* idx=sidx->odml_index+(sidx->index_count);		// access to index
		odml_index_data_t* idxd=idx->index+(idx->nEntriesInUse);	// access to unused index data

		uint64_t pos=LMovie->Tell()+8;	// preview position of data
		idxd->fpos=pos;	// store file position of data

		if(keyFrame)
			idxd->size=data_size; //store data size
		else	// if no key frame
			idxd->size=data_size|0x80000000; //store data size with bit 31 set

		++(idx->nEntriesInUse);	// advance to next free index data entry
	}
	return true;
}
#else
{
    return true;
}
#endif
/**
        \fn odml_write_sindex
*/

void aviWrite::odml_write_sindex(int stream_nbr, const char* stream_fcc)
{

	// Warning: This changes the file position
	if(doODML==NORMAL)
                {
                    uint32_t startAt=odml_indexes[stream_nbr].fpos;
                    uint32_t pad=odml_indexes[stream_nbr].pad;
                    uint32_t endAt=startAt+pad+8;
#ifndef MOVINDEX
		_file->seek(startAt);
#endif
		aprintf("[AVI]writing super index at file pos %"LLU", total available size %"LU"\n",
                            odml_indexes[stream_nbr].fpos,pad);
		AviList* LIndex =  new AviList("JUNK", _file);	// abused writing aid (don't call Begin or End; the fcc is unused until 'Begin')
                uint32_t nbEntries=odml_indexes[stream_nbr].index_count+1;
		LIndex->Write32("indx");			// 4cc
		LIndex->Write32(24+nbEntries*16);	// size
		LIndex->Write16(4);				// wLongsPerEntry
		LIndex->Write8(0);				// bIndexSubType
		LIndex->Write8(0);				// bIndexType (AVI_INDEX_OF_INDEXES)
		LIndex->Write32(nbEntries);// nEntriesInUse
		LIndex->Write32(stream_fcc);			// dwChunkId;
		LIndex->Write32((uint32_t)0);
                LIndex->Write32((uint32_t)0);
                LIndex->Write32((uint32_t)0);// reserved
		for(uint32_t a=0;a<nbEntries;++a)
                {	// for each chunk index
                        uint64_t pos;
                        pos=odml_indexes[stream_nbr].odml_index[a].fpos;
                        LIndex->Write64(pos);	//absolute file position
                        LIndex->Write32(32 + 8 * odml_index_size);	// complete index chunk size
                        LIndex->Write32(odml_indexes[stream_nbr].odml_index[a].nEntriesInUse);	// duration
                        aprintf("[AVI]stream %d, index %"LU" Position: %"LLU"  EntriesInUse:%"LU"\n",stream_nbr, a ,pos,
                        odml_indexes[stream_nbr].odml_index[a].nEntriesInUse);
		}
                uint32_t at=LIndex->Tell();

                int32_t junkLen=endAt-at-8;
                ADM_assert(junkLen>=9);
                printf("[AVI]Padding ODML index with junk of size %"LD", total padding %u\n",junkLen, odml_indexes[stream_nbr].pad);
		delete LIndex;
// Now create out junk chunk if needed, to padd the odml
                AviList *Junk=new AviList("JUNK",_file);
                uint8_t *zz=new uint8_t[junkLen];
                if(junkLen>9) strcpy((char *)zz,"Avidemux");
                Junk->WriteChunk ((uint8_t *)"JUNK", junkLen, zz);
                delete [] zz;
                ADM_assert(endAt==Junk->Tell());
                delete Junk;


	}
}
/**
        \fn odml_write_index
*/

bool aviWrite::odml_write_index(int stream_nbr, const char* stream_fcc, const char* index_fcc){	// write index
	// Warning: This changes the file position
	if(doODML==NORMAL){
		aprintf ("\n writing %d interleaved ODML indexes for %u frames in stream %s", odml_indexes[stream_nbr].index_count+1, vframe, stream_fcc);
		AviList* LIndex =  new AviList("JUNK", _file);	// abused writing aid (don't call Begin or End; the fcc is unused until 'Begin')
		for(int a=0;a<=odml_indexes[stream_nbr].index_count;++a){	// for each index
			odml_index_t* idx=odml_indexes[stream_nbr].odml_index+a;		// access to index
			_file->seek(idx->fpos);					// shift file pointer
			LIndex->Write32(index_fcc);			// 4cc
			LIndex->Write32(24+odml_index_size*8);		// data size
			LIndex->Write16(2);				// wLongsPerEntry
			LIndex->Write8(0);				// bIndexSubType
			LIndex->Write8(1);				// bIndexType (AVI_INDEX_OF_CHUNKS)
			LIndex->Write32(idx->nEntriesInUse);		// nEntriesInUse
			LIndex->Write32(stream_fcc);			// dwChunkId;
			uint64_t base_off=idx->index[0].fpos-8;		// lets take the position of the first frame in the index as base
			uint64_t rel_pos;
			LIndex->Write64(base_off);			// qwBaseOffset
			LIndex->Write32((uint32_t)0);			// reserved
			for(int b=0;b<idx->nEntriesInUse;++b){		// for each frame in the current index
				odml_index_data_t* idxd=idx->index+b;	// access to index data
				rel_pos=idxd->fpos-base_off;	// get relative file position
				if(rel_pos>(uint64_t)4*1024*1024*1024){	// index chunks have a maximum offset of 4GB
					printf("[AVI]Data rate too high for index size. Decrease index duration.\n"); // decrease the multiplicator in saveBegin that calculates odml_index_size
					printf("[AVI]base:%"LLU" abs:%"LLU" rel:%"LLU" stream:%d index:%d entry:%d",base_off,idxd->fpos,rel_pos,stream_nbr,a,b);
					delete LIndex;
					return false;
				}
				LIndex->Write32(rel_pos);	// relative file position
				LIndex->Write32(idxd->size);		// data size
			}
		}
		delete LIndex;
	}
	return true;
}
/**
        \fn odml_riff_break
*/
void aviWrite::odml_riff_break(uint32_t len){	// advance to the next riff if required
#if 0
	if(doODML!=NO){
		// get padded size
		uint64_t len2=len;
		if(len & 1)++len2;
		// preview file position
		len2+=LMovie->Tell();
		// will we get over the next GB border?
		if( len2>((uint64_t)1024*1024*1024*(odml_riff_count+1)) ){
			if(doODML==HIDDEN){
				aprintf("[AVI]starting new (hidden) RIFF at %"LLU"\n",LMovie->Tell());
				if(odml_riff_count<4)	// we have only 4 buffers but this has to be enough
					odml_write_dummy_chunk(LMovie, odml_riff_fpos+odml_riff_count, 16);	// write dummy
				if(odml_riff_count==0) odml_frames_inAVI=vframe-1;	// rescue number of frames in first AVI (-1 since there may be no audio for the last video frame)
			}else{	// restart riff and movie
				aprintf("[AVI]starting new RIFF at %"LLU"\n",LMovie->Tell());
				// restart lists
				LMovie->End();
				LAll->End();
				LAll->Begin ("AVIX");
				LMovie->Begin ("movi");
			}
			++odml_riff_count;
		}
		// ODML required for movie?
		if(doODML==HIDDEN){
			if( ((uint64_t)getPos()+len+17) >= ((uint64_t)4*1024*1024*1024) ){	//if (written data + new chunk + index (old type) for new chunk + possible padding) does not fit into 4GB
				printf("[AVI]switching to ODML mode at %"LLU"\n",LMovie->Tell());
				uint64_t last_pos=LMovie->Tell();	// rescue current file position
				// close First RIFF
				for(int a=0;a<4;++a){	// for each hidden riff
					if(odml_riff_fpos[a]!=0){
						_file->seek(odml_riff_fpos[a]);	// set file pointer to start of next riff
						LMovie->End();
						LAll->End();
						LAll->Begin("AVIX");
						LMovie->Begin("movi");
					}
				}
				// goto end of file
				_file->seek(last_pos);
				// following riffs can start directly
				doODML=NORMAL;	// write RIFF breaks directly
			}
		}
	}
#endif
}

#endif

// EOF