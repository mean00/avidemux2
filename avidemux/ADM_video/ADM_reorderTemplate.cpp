/*
 * INDEX_TMPL is the type of each index
 * INDEX_ARRAY_TMPL is the attribute holding the index
 * FRAMETYPE_TMPL is the field holding video flags
 * */
#ifndef INDEX_TMPL
#error INDEX_TMPL
#endif
#ifndef INDEX_ARRAY_TMPL
#error INDEX_ARRAY_TMPL
#endif
#ifndef FRAMETYPE_TMPL
#error FRAMETYPE_TMPL
#endif

INDEX_TMPL *index;
uint8_t ret=1;
uint32_t nbFrame= _videostream.dwLength;
	// already done..
	if( _reordered) return 1;
	ADM_assert(INDEX_ARRAY_TMPL);
	
	index=new INDEX_TMPL[nbFrame];
	// clear B frame flag for last frames
	INDEX_ARRAY_TMPL[nbFrame-1].FRAMETYPE_TMPL &=~AVI_B_FRAME;

			//__________________________________________
			// the direct index is in DTS time (i.e. decoder time)
			// we will now do the PTS index, so that frame numbering is done
			// according to the frame # as they are seen by editor / user
			// I1 P0 B0 B1 P1 B2 B3 I2 B7 B8
			// xx I1 B0 B1 P0 B2 B3 P1 B7 B8
			//__________________________________________
			uint32_t forward=0;
			uint32_t curPTS=0;
			uint32_t dropped=0;

			for(uint32_t c=1;c<nbFrame;c++)
			{
				if(!(INDEX_ARRAY_TMPL[c].FRAMETYPE_TMPL & AVI_B_FRAME))
					{
								memcpy(&index[curPTS],
										&INDEX_ARRAY_TMPL[forward],
										sizeof(INDEX_TMPL));
								forward=c;
								curPTS++;
								dropped++;
					}
					else
					{// we need  at lest 2 i/P frames to start decoding B frames
						if(dropped>=1)
						{
							memcpy(&index[curPTS],
								&INDEX_ARRAY_TMPL[c],
								sizeof(INDEX_TMPL));
							curPTS++;
						}
						else
						{
						printf("We dropped a frame (%d/%d).\n",dropped,c);
						}
					}
			}

			uint32_t last;


			// put back last I/P we had in store
			memcpy(&index[curPTS],
				&INDEX_ARRAY_TMPL[forward],
				sizeof(INDEX_TMPL));
			last=curPTS;

			_videostream.dwLength= _mainaviheader.dwTotalFrames=nbFrame=last+1;
			// last frame is always I

			delete [] INDEX_ARRAY_TMPL;

			INDEX_ARRAY_TMPL=index;;
			// last frame cannot be B frame
			INDEX_ARRAY_TMPL[last].FRAMETYPE_TMPL&=~AVI_B_FRAME;
                        // And first is an intra
                        INDEX_ARRAY_TMPL[0].FRAMETYPE_TMPL=AVI_KEY_FRAME;
			 _reordered=ret;
