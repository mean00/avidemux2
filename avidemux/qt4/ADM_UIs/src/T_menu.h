#ifndef T_menu_h
#define T_menu_h

#include <QComboBox>

#include "ADM_inttype.h"
#include "DIA_factory.h"

namespace ADM_qt4Factory
{
	class diaElemMenuDynamic : public diaElemMenuDynamicBase
	{
	public:
		diaElemMenuDynamic(uint32_t *intValue, const char *itle, uint32_t nb, diaMenuEntryDynamic **menu, const char *tip = NULL);

		virtual   ~diaElemMenuDynamic() ;
		void      setMe(void *dialog, void *opaque,uint32_t line);
		void      getMe(void);
		virtual uint8_t   link(diaMenuEntryDynamic *entry,uint32_t onoff,diaElem *w);
		virtual void      updateMe(void);
		virtual void      enable(uint32_t onoff) ;
		virtual void      finalize(void);
		int getRequiredLayout(void);
	};

	class ADM_QComboBox : public QComboBox
	{
		Q_OBJECT

	public slots:
		void changed(int i);

	protected:
		diaElemMenuDynamic *_menu;

	public:
		ADM_QComboBox(diaElemMenuDynamic *menu);
		void connectMe(void);
	};
}
#endif	// T_menu_h
