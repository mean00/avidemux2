#ifndef ADM_qtScript_ComboBoxItemCollection
#define ADM_qtScript_ComboBoxItemCollection

#include "VectorBasedCollection.h"
#include "ComboBoxItem.h"
#include "ComboBoxItemCollectionPrototype.h"

namespace ADM_qtScript
{
	class ComboBoxItemCollection : public VectorBasedCollection<ComboBoxItem>
	{
	public:
		ComboBoxItemCollection(QScriptEngine *engine, std::vector<ComboBoxItem*>& items) :
			VectorBasedCollection<ComboBoxItem>(engine, items, new ComboBoxItemCollectionPrototype(items)) { }

		QString name() const { return QLatin1String("ComboBoxItemCollection"); }
	};
}
#endif
