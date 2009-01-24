/***************************************************************************
                          ADM_vidFont.cpp  -  description
                             -------------------
    begin                : Sun Dec 15 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
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
#include "ADM_vidFont.h"

static    FT_Library   	library;   		/* handle to library     */
static    int 			initialized=0; 	// 0 No init at all, 1 engine inited

ADMfont::ADMfont ( void )
{
	_faceAllocated=0;
	_use2bytes=0;
	_hold=0;
	_value=0;

}

/**
	Deallocate font allocated stuff
*/
 ADMfont::~ADMfont( )
{

	if(_faceAllocated)
	{
		//
		FT_Done_Face(_face);
		_faceAllocated=0;

	}
}


int ADMfont::initFreeType( char *fontname )
{
int error;

	printf("\n ** Initializing FreeType **\n");
	if(initialized==0)
	{

    		error = FT_Init_FreeType( &library );
    		if ( error )
    		{
			  printf("\n Error Initializing Free Type (%d)\n",error);
			  return 0;
		  }
		initialized=1;
	}
  	error = FT_New_Face( library,
                         fontname,
                         0,
                         &_face );
    	if ( error == FT_Err_Unknown_File_Format )
    	{
   	   	printf("\n Error unknown font format (%d)\n",error);
			  return 0;

    	}
    	else if ( error )
    	{
     		printf("\n Error unknown error (font %d)\n",error);
		 return 0;
    	}
	_faceAllocated=1;

	error = FT_Set_Pixel_Sizes(
              _face,   /* handle to face object            */
              0,      /* pixel_width                      */
              16 );   /* pixel_height                     */

	printf("\n **  FreeType Initialized **\n");
	_hold=0;
   	return 1;
}
//____________________________________________________
int ADMfont::fontSetCharSet (char *charset)
{
	printf("OBSOLETE*********\n");
	return 1;
}
//---------------------------------
int ADMfont::fontSetSize ( int size)
{
int error;
	if(!_faceAllocated)
		{
				printf("\n not initialized");
				return 0;
		}
	   error = FT_Set_Pixel_Sizes(
              _face,   /* handle to face object            */
              0,      /* pixel_width                      */
              size );   /* pixel_height                     */

	return 1;
}
//____________________________________________________

int ADMfont::fontDraw(char *target, int  c, int prevchar,int stride, int size,int *ww)
{


			if(!_faceAllocated)
			{
				printf("No face!\n");
				return 0;
			}
FT_GlyphSlot  slot = _face->glyph;  // a small shortcut
int  glyph_index,glyph_prev;
int error;
FT_Vector delta;
int kern;

	//printf("FONT: rendering %d %c\n",c,c);
	*ww=0;
/* Ugly patch t avoid some display problem */
#if 0
        if(c=='\'') c='"';
        if(prevchar=='\'') prevchar='"';
#endif
/* Ugly patch t avoid some display problem */
	glyph_index = FT_Get_Char_Index( _face, c );
	if(prevchar)
		glyph_prev=FT_Get_Char_Index( _face, prevchar );

   	error = FT_Load_Glyph(
        		   _face,          /* handle to face object */
        		     glyph_index,   /* glyph index           */
        		      0 );  /* load flags, see below */
	if(error)
	{
		printf("Loadglyph error\n");
	 	return 0;
	}

	error = FT_Render_Glyph(
			slot,      /* glyph slot  */
			ft_render_mode_normal);    /* render mode */

      	if(error)
	{
		printf("RenderGlyph error");
	 	return 0;
	 }

       // now, draw to our target surface
       // inspired from MPlayer font rendering

	FT_Bitmap *bitmap=&(slot->bitmap);
	int heigh;
	int srow=0;


	heigh=bitmap->rows;
	target+=stride*(size-slot->bitmap_top);

	int correction;
// If kerning is available from freetype
#ifdef FT_FACE_FLAG_KERNING
	if(prevchar && FT_HAS_KERNING( _face ))
	{
		FT_Get_Kerning( _face,glyph_prev, glyph_index, 0 /*FT_KERNING_DEFAULT*/, &delta );
		correction=delta.x/64;
	}
	else
#endif
		correction=0;
	target+=correction;

	target+=slot->bitmap_left;
	//target+=(slot->bitmap_top)*stride;
	for (int h = heigh; h>0 ; h-- )
	{
	    for (int w =0;w< bitmap->width;  w++ )
	    {
		if(bitmap->buffer[srow+w])
			    *(target+w) = bitmap->buffer[srow+w];

	     }
		 target+=stride;
		 srow+=bitmap->pitch ;
	}

	// Now advance cursor
	int advance=0;//correction;
	*ww=bitmap->width;
	advance+=slot->advance.x/64;


// 	printf("FONT: Width %d, adance:%d\n",bitmap->width,advance);
//  	printf("FONT:cur :%c next :%c\n",c,prevchar);
//  	printf("FONT:cur :%d next :%d\n",glyph_index,glyph_prev);
//  	printf("FONT:Raw: %d kerning:%d kerning :%d \n",*ww,delta.x,delta.y);


//	FT_Done_Glyph(glyph_index); Mem leak ?
	*ww=advance;
	return 1;
}

