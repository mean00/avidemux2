/***************************************************************************
                          DIA_factory.cpp
  This file contains only redirect functions.
  Check the GTK/QT/.. functions to see the intersting parts
  (C) Mean 2008 fixounet@free.fr

/!\ Big Warning, each time there is a cast, it is a gross hack and needs to be fixed
 later by using proper inheritance. It is very dangerous as it is.


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
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"
#include "DIA_factoryStubs.h"

static FactoryDescriptor *Factory=NULL;
/**
 * 	\fn FactoryDescriptor
 */
uint8_t  DIA_factoryInit(FactoryDescriptor *d)
{
	uint32_t major,minor,patch;
	Factory=d;
	d->FactoryGetVersion(&major,&minor,&patch);
	printf("[COREUI] Compiled with %02d.%02d.%02d\n",ADM_COREUI_MAJOR,ADM_COREUI_MINOR,ADM_COREUI_PATCH);
	printf("[COREUI] Linked with   %02d.%02d.%02d\n",major,minor,patch);
	if(major!=ADM_COREUI_MAJOR)
	{
			printf("[CoreUI]Incompatible COREUI Major version, compiled with %d, using %d\n",ADM_COREUI_MAJOR,major);
			ADM_assert(0);
	}
	if(minor!=ADM_COREUI_MINOR)
	{
		printf("[CoreUI] Maybe Incompatible COREUI Minor version, compiled with %d, using %d\n",ADM_COREUI_MINOR,minor);
	}
	printf("[CoreUI] Compiled with patch version %d, using %d\n",ADM_COREUI_PATCH,patch);
	return 1;
}
// ****************** All ************************
uint8_t diaFactoryRun(const char *title,uint32_t nb,diaElem **elems)
{
	ADM_assert(Factory); 
	return Factory->FactoryRun(title,nb,elems);
}
uint8_t diaFactoryRunTabs(const char *title,uint32_t nb,diaElemTabs **tabs)
{
	ADM_assert(Factory); 
	return Factory->FactoryRunTab(title,nb,tabs);
}

// ****************** Buttons ********************
diaElemButton ::diaElemButton(const char *toggleTitle, ADM_FAC_CALLBACK *cb,void *cookie,const char *tip) :diaElem(ELEM_BUTTON)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreareButton(toggleTitle, cb,cookie,tip);
}
diaElemButton ::~diaElemButton()
{
	ADM_assert(Factory); 
	Factory->DestroyButton(internalPointer);
	internalPointer=NULL;
}
void      diaElemButton::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
DIA_MKSTUBS(diaElemButton)
// ****************** Bar ********************
diaElemBar ::diaElemBar(uint32_t percent,const char *toggleTitle) :diaElem(ELEM_BAR)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateBar(percent,toggleTitle);
}
diaElemBar ::~diaElemBar()
{
	ADM_assert(Factory); 
	Factory->DestroyBar(internalPointer);
	internalPointer=NULL;
}
DIA_MKSTUBS(diaElemBar)

// ****************** Buttons ********************
diaElemFloat ::diaElemFloat(ELEM_TYPE_FLOAT *intValue,const char *toggleTitle, ELEM_TYPE_FLOAT min,
        ELEM_TYPE_FLOAT max,const char *tip, int decimals) :diaElem(ELEM_FLOAT)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateFloat(intValue,toggleTitle, min,max,tip, decimals);
}
diaElemFloat ::~diaElemFloat()
{
	ADM_assert(Factory); 
	Factory->DestroyFloat(internalPointer);
	internalPointer=NULL;
}
void      diaElemFloat::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
DIA_MKSTUBS(diaElemFloat)
// ****************** Integer ********************
diaElemInteger ::diaElemInteger(int32_t *intValue,const char *toggleTitle, int32_t min, int32_t max,const char *tip) :
	diaElem(ELEM_INTEGER)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateInteger(intValue,toggleTitle, min,max,tip);
}
diaElemInteger ::~diaElemInteger()
{
	ADM_assert(Factory); 
	Factory->DestroyInteger(internalPointer);
	internalPointer=NULL;
}
void      diaElemInteger::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
DIA_MKSTUBS(diaElemInteger)
// ****************** UInteger ********************
diaElemUInteger ::diaElemUInteger(uint32_t *intValue,const char *toggleTitle, uint32_t min, uint32_t max,const char *tip) :
	diaElem(ELEM_INTEGER)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateUInteger(intValue,toggleTitle, min,max,tip);
}
diaElemUInteger ::~diaElemUInteger()
{
	ADM_assert(Factory); 
	Factory->DestroyUInteger(internalPointer);
	internalPointer=NULL;
}
void      diaElemUInteger::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
DIA_MKSTUBS(diaElemUInteger)
// ****************** diaElemNotch ********************
diaElemNotch ::diaElemNotch(uint32_t yes,const char *toggleTitle, const char *tip):
	diaElem(ELEM_NOTCH)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateNotch(yes,toggleTitle,tip);
}
diaElemNotch ::~diaElemNotch()
{
	ADM_assert(Factory); 
	Factory->DestroyNotch(internalPointer);
	internalPointer=NULL;
}

DIA_MKSTUBS(diaElemNotch)
// ****************** diaReadonlyText ********************
diaElemReadOnlyText ::diaElemReadOnlyText(const char *readyOnly,const char *toggleTitle,const char *tip):
	diaElem(ELEM_ROTEXT)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateReadonlyText(readyOnly,toggleTitle,tip);
}
diaElemReadOnlyText ::~diaElemReadOnlyText()
{
	ADM_assert(Factory); 
	Factory->DestroyReadonlyText(internalPointer);
	internalPointer=NULL;
}

DIA_MKSTUBS(diaElemReadOnlyText)
// ****************** diaText ********************
diaElemText ::diaElemText(char **readyOnly,const char *toggleTitle,const char *tip):
	diaElem(ELEM_TEXT)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateText(readyOnly,toggleTitle,tip);
}
diaElemText ::~diaElemText()
{
	ADM_assert(Factory); 
	Factory->DestroyText(internalPointer);
	internalPointer=NULL;
}
void      diaElemText::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
DIA_MKSTUBS(diaElemText)
// ******************************************	

diaElemHex ::diaElemHex(const char *toggleTitle, uint32_t dataSize,uint8_t *data):
	diaElem(ELEM_HEXDUMP)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateHex(toggleTitle,dataSize,data);
}
diaElemHex ::~diaElemHex()
{
	ADM_assert(Factory); 
	Factory->DestroyHex(internalPointer);
	internalPointer=NULL;
}
void      diaElemHex::finalize(void)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->finalize(); 
	}
DIA_MKSTUBS(diaElemHex)
// ****************** diaElemMatrix ********************
diaElemMatrix ::diaElemMatrix(uint8_t *trix,const char *toggleTitle, uint32_t trixSize,const char *tip):
	diaElem(ELEM_MATRIX)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateMatrix(trix,toggleTitle,trixSize,tip);
}
diaElemMatrix ::~diaElemMatrix()
{
	ADM_assert(Factory); 
	Factory->DestroyMatrix(internalPointer);
	internalPointer=NULL;
}
void      diaElemMatrix::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
DIA_MKSTUBS(diaElemMatrix)
// ****************** diaElemMenu ********************
diaElemMenu ::diaElemMenu(uint32_t *intValue,const char *itle, uint32_t nb, 
        const diaMenuEntry *menu,const char *tip):
	diaElemMenuBase()
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateMenu(intValue, itle,  nb,  menu,tip);
}
diaElemMenu ::~diaElemMenu()
{
	ADM_assert(Factory); 
	Factory->DestroyMenu(internalPointer);
	internalPointer=NULL;
}
void      diaElemMenu::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
uint8_t   diaElemMenu::link(diaMenuEntry *entry,uint32_t onoff,diaElem *w)
{
	diaElemMenuBase *cast=(diaElemMenuBase *)internalPointer;
	cast->link(entry,onoff,w);
}
void      diaElemMenu::updateMe(void)
{
	

}
void   diaElemMenu::finalize(void)
{
	internalPointer->finalize();
}
DIA_MKSTUBS(diaElemMenu)
// ****************** diaElemMenuDynamic ********************
diaElemMenuDynamic ::diaElemMenuDynamic(uint32_t *intValue,const char *itle, uint32_t nb, 
         diaMenuEntryDynamic **menu,const char *tip):
        	 diaElemMenuDynamicBase()
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateMenuDynamic(intValue, itle,  nb,  menu,tip);
}
diaElemMenuDynamic ::~diaElemMenuDynamic()
{
	ADM_assert(Factory); 
	Factory->DestroyMenuDynamic(internalPointer);
	internalPointer=NULL;
}
void      diaElemMenuDynamic::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
uint8_t   diaElemMenuDynamic::link(diaMenuEntryDynamic *entry,uint32_t onoff,diaElem *w)
{
	diaElemMenuDynamicBase *cast=(diaElemMenuDynamicBase *)internalPointer;
	cast->link(entry,onoff,w);
}
void   diaElemMenuDynamic::finalize(void)
{
	internalPointer->finalize();
}
void      diaElemMenuDynamic::updateMe(void)
{
	

}

DIA_MKSTUBS(diaElemMenuDynamic)
// ****************** diaElemMatrix ********************
diaElemThreadCount ::diaElemThreadCount(uint32_t *value, const char *title, const char *tip):
	diaElem(ELEM_THREAD_COUNT)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateThreadCount(value,title,tip);
}
diaElemThreadCount ::~diaElemThreadCount()
{
	ADM_assert(Factory); 
	Factory->DestroyThreadCount(internalPointer);
	internalPointer=NULL;
}
DIA_MKSTUBS(diaElemThreadCount)

// ****************** diaElemBitrate ********************
diaElemBitrate ::diaElemBitrate(COMPRES_PARAMS *p,const char *toggleTitle,const char *tip):
	diaElemBitrateBase()
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateBitrate(p,toggleTitle,tip);
}
diaElemBitrate ::~diaElemBitrate()
{
	ADM_assert(Factory); 
	Factory->DestroyBitrate(internalPointer);
	internalPointer=NULL;
}
void diaElemBitrate::updateMe()
{
	
}
void diaElemBitrate::setMaxQz(uint32_t qz)
{
	diaElemBitrateBase *cast=(diaElemBitrateBase *)internalPointer;
		cast->setMaxQz(qz);
	
}
DIA_MKSTUBS(diaElemBitrate)
// ****************** diaElemFile ********************
diaElemFile ::diaElemFile(uint32_t writeMode,char **filename,const char *toggleTitle,
        const char *defaultSuffix ,const char *tip)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateFile(writeMode,filename,toggleTitle,defaultSuffix,tip);
}
diaElemFile ::~diaElemFile()
{
	ADM_assert(Factory); 
	Factory->DestroyBitrate(internalPointer);
	internalPointer=NULL;
}
void      diaElemFile::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
void      diaElemFile::changeFile(void)
	{ 
	diaElemFileBase *cast=(diaElemFileBase *)internalPointer;
	cast->changeFile();
	}
DIA_MKSTUBS(diaElemFile)

// ****************** diaElemDirSelect ********************
diaElemDirSelect ::diaElemDirSelect(char **filename,const char *toggleTitle,const char *tip):
	diaElemDirSelectBase()
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateDir( filename, toggleTitle, tip);
}
diaElemDirSelect ::~diaElemDirSelect()
{
	ADM_assert(Factory); 
	Factory->DestroyBitrate(internalPointer);
	internalPointer=NULL;
}
void      diaElemDirSelect::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
void      diaElemDirSelect::changeFile(void)
	{ 
	diaElemDirSelectBase *cast=(diaElemDirSelectBase *)internalPointer;
	cast->changeFile();
	}
DIA_MKSTUBS(diaElemDirSelect)
// ****************** diaElemFrame ********************
diaElemFrame ::diaElemFrame(const char *toggleTitle, const char *tip):
	diaElemFrameBase()
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateFrame( toggleTitle, tip);
}
diaElemFrame ::~diaElemFrame()
{
	ADM_assert(Factory); 
	Factory->DestroyFrame(internalPointer);
	internalPointer=NULL;
}
void      diaElemFrame::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
void diaElemFrame::swallow(diaElem *widget)
{
	diaElemFrameBase *cast=(diaElemFrameBase *)internalPointer;
		cast->swallow(widget);	
}
 
void diaElemFrame::finalize(void)
{
	internalPointer->finalize();
		
}
 
DIA_MKSTUBS(diaElemFrame)
// ****************** diaElemToggleUint ********************
diaElemToggleUint ::diaElemToggleUint(uint32_t *toggleValue,const char *toggleTitle, uint32_t *uintval, const char *name,uint32_t min,uint32_t max,const char *tip):
	diaElem(ELEM_TOGGLE_UINT)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateToggleUint(toggleValue,toggleTitle,uintval,name,min,max,tip);
}
diaElemToggleUint ::~diaElemToggleUint()
	{
	
	ADM_assert(Factory); 
	Factory->DestroyToggleUint(internalPointer);
	internalPointer=NULL;
}
void      diaElemToggleUint::finalize(void)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->finalize(); 
	}
void      diaElemToggleUint::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
DIA_MKSTUBS(diaElemToggleUint)
// ****************** diaElemToggleInt ********************
diaElemToggleInt ::diaElemToggleInt(uint32_t *toggleValue,const char *toggleTitle, int32_t *uintval, 
									const char *name,int32_t min,int32_t max,const char *tip):
	diaElem(ELEM_TOGGLE_INT)
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateToggleInt(toggleValue,toggleTitle,uintval,name,min,max,tip);
}
diaElemToggleInt ::~diaElemToggleInt()
	{
	
	ADM_assert(Factory); 
	Factory->DestroyToggleInt(internalPointer);
	internalPointer=NULL;
}
void      diaElemToggleInt::finalize(void)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->finalize(); 
	}
void      diaElemToggleInt::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
DIA_MKSTUBS(diaElemToggleInt)
// ****************** diaElemToggle ********************
diaElemToggle ::diaElemToggle(uint32_t *toggleValue,const char *toggleTitle, const char *tip):
	diaElemToggleBase()
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateToggle(toggleValue,toggleTitle,tip);
}
diaElemToggle ::~diaElemToggle()
	{
	
	ADM_assert(Factory); 
	Factory->DestroyToggle(internalPointer);
	internalPointer=NULL;
}
void      diaElemToggle::finalize(void)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->finalize(); 
	}
void      diaElemToggle::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
uint8_t   diaElemToggle::link(uint32_t onoff,diaElem *w)
{
	diaElemToggleBase *cast=(diaElemToggleBase *)internalPointer;
			cast->link(onoff,w);	
}
DIA_MKSTUBS(diaElemToggle)
//
// ****************** diaElemUSlider ********************
diaElemUSlider ::diaElemUSlider(uint32_t *value,const char *toggleTitle, uint32_t min,uint32_t max,uint32_t incr , const char *tip):
	diaElemSliderBase()
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateUSlider(value,toggleTitle,min,max,incr,tip);
}
diaElemUSlider ::~diaElemUSlider()
	{
	
	ADM_assert(Factory); 
	Factory->DestroyUSlider(internalPointer);
	internalPointer=NULL;
}
void      diaElemUSlider::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
uint8_t   diaElemUSlider::setDigits(uint32_t digits)
{
	diaElemSliderBase *cast=(diaElemSliderBase *)internalPointer;
			cast->setDigits(digits);	
}
DIA_MKSTUBS(diaElemUSlider)
// ****************** diaElemSlider ********************
diaElemSlider ::diaElemSlider(int32_t *value,const char *toggleTitle, int32_t min,int32_t max,int32_t incr , const char *tip):
	diaElemSliderBase()
{
	ADM_assert(Factory); 
	internalPointer=Factory->CreateSlider(value,toggleTitle,min,max,incr,tip);
}
diaElemSlider ::~diaElemSlider()
	{
	
	ADM_assert(Factory); 
	Factory->DestroySlider(internalPointer);
	internalPointer=NULL;
}
void      diaElemSlider::enable(uint32_t onoff)
	{ 
		ADM_assert(internalPointer); 
		internalPointer->enable(onoff); 
	}
uint8_t   diaElemSlider::setDigits(uint32_t digits)
{
	diaElemSliderBase *cast=(diaElemSliderBase *)internalPointer;
			cast->setDigits(digits);	
}
DIA_MKSTUBS(diaElemSlider)
//
// EOF
