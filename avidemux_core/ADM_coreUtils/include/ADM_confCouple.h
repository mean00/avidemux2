/** *************************************************************************
    \file ADM_confCouple.h
    \brief Handle Key/value pair
    copyright            : (C) 2002/2009 by mean
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
            

	public:
			int32_t lookupName(const char *myname);

			CONFcouple(uint32_t nb);
			~CONFcouple();

			bool writeAsUint32(const char *name,uint32_t value);
			bool writeAsInt32(const char *name,int32_t value);
			bool writeAsString(const char *name,const char *value);
			bool writeAsFloat(const char *name,float value);
            bool writeAsBool(const char *name,bool value);
			


            bool readAsUint32(const char *name,uint32_t *value);    
			bool readAsInt32(const char *name,int32_t *value);
			bool readAsString(const char *name,char **value);
			bool readAsFloat(const char *name,float *value);
            bool readAsBool(const char *name,bool *value);

            bool exist(const char *name);

			uint32_t getSize(void) { return nb;};

            bool     setInternalName(const char *name, const char *key);
			bool     getInternalName(uint32_t n, char **nm, char **val);
			void dump(void );

};
#endif
