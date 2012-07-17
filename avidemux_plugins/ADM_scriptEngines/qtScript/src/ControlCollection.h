#ifndef ADM_qtScript_ControlCollection
#define ADM_qtScript_ControlCollection

#include "VectorBasedCollection.h"
#include "Control.h"
#include "ControlCollectionPrototype.h"

namespace ADM_qtScript
{
	class ControlCollection : public VectorBasedCollection<Control>
	{
	public:
		ControlCollection(QScriptEngine *engine, std::vector<Control*>& controls) :
			VectorBasedCollection<Control>(engine, controls, new ControlCollectionPrototype(controls)) { }

		QString name() const { return QLatin1String("ControlCollection"); }
	};
}
#endif
