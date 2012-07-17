#ifndef ADM_qtScript_Control
#define ADM_qtScript_Control

#include <QtCore/QObject>
#include <QtScript/QScriptable>

#include "DIA_factory.h"

namespace ADM_qtScript
{
	/** \brief The Control %class is the base class of all user interface controls that can be added to a Dialog.
	 */
	class Control : public QObject, protected QScriptable
	{
		Q_OBJECT

	public:
		/** \cond */
		virtual diaElem* createControl(void) = 0;
		/** \endcond */
	};
}

#endif