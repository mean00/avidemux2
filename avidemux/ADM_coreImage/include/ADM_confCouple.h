/***************************************************************************
                          CONFcouple.cpp  -  description
                             -------------------
	Used to set a pair name/value
	--> filters

    begin                : Sun Apr 14 2002
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
 #ifndef __CONFCOUPLE__
 #define __CONFCOUPLE__

class CONFcouple
{
	private:
			uint32_t    nb;
			char 		**name;
			char 		**value;
			uint8_t	cur;

			int32_t lookupName(const char *myname);
	public:
			CONFcouple(uint32_t nb);
			~CONFcouple();
			uint8_t setCouple(const char *name,uint32_t value);
			uint8_t setCouple(const char *name,int32_t value);
			uint8_t setCouple(const char *name,const char *value);
			uint8_t setCouple(const char *name,const ADM_filename *value);
			uint8_t setCouple(const char *name,double value);
			uint8_t setCouple(const char *name,float value);

			uint8_t getCouple(const char *name,int32_t *value);
			uint8_t getCouple(const char *name,uint32_t *value);
			uint8_t getCouple(const char *name,char **value);
			uint8_t getCouple(const char *name,ADM_filename **value);
			uint8_t getCouple(const char *name,double *value);
			uint8_t getCouple(const char *name,float *value);
			uint32_t getNumber(void) { return nb;};
			uint8_t getEntry(uint32_t n, char **nm, char **val)
				 { assert(n<nb); *nm=name[n];*val=value[n];return 1;};
			void dump(void );

};
#endif
