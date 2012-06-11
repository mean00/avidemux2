#ifndef ADM_qtScript_AudioOutputCollectionPrototype
#define ADM_qtScript_AudioOutputCollectionPrototype

#include "QtScriptObject.h"

namespace ADM_qtScript
{
	/** \brief The AudioOutputCollection %class represents a collection of AudioOutput objects
	 * that can be individually accessed by index.
	 */
	class AudioOutputCollectionPrototype /*% AudioOutputCollection %*/: public QtScriptObject
	{
		Q_OBJECT

	private:
		ActiveAudioTracks* _tracks;

		QScriptValue getLength();

	public:
		/** \cond */
		AudioOutputCollectionPrototype(QObject* parent, IEditor* editor);
		/** \endcond */

		/** \brief Returns the number of AudioOutput objects in the collection.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ length READ getLength());

		/** \brief Adds a new audio output to the collection to encode an audio track from the open video.
		 * \return Returns the index of the newly added AudioOutput in the collection.
		 */
		Q_INVOKABLE QScriptValue /*% Number %*/ add(int /*% Number %*/ inputTrackIndex, QScriptValue /*% AudioEncoder %*/ encoder = QScriptValue::NullValue /*% new CopyAudioEncoder() %*/);

		/** \brief Adds a new audio output to the collection to encode an external audio file.
		 * \return Returns the index of the newly added AudioOutput in the collection.
		 */
		Q_INVOKABLE QScriptValue /*% Number %*/ add(QString /*% String %*/ externalAudioFile, QScriptValue /*% AudioEncoder %*/ encoder = QScriptValue::NullValue /*% new CopyAudioEncoder() %*/);

		/** \brief Removes all AudioOutput objects from the collection.
		 */
		Q_INVOKABLE void clear();

		/** \brief Inserts a new audio output in to the collection at the specified index to encode an audio track from the open video.
		 */
		Q_INVOKABLE void insert(int /*% Number %*/ index, int /*% Number %*/ inputTrackIndex, QScriptValue /*% AudioEncoder %*/ encoder = QScriptValue::NullValue /*% new CopyAudioEncoder() %*/);

		/** \brief Inserts a new audio output in to the collection at the specified index to encode an external audio file.
		 */
		Q_INVOKABLE void insert(int /*% Number %*/ index, QString /*% String %*/ externalAudioFile, QScriptValue /*% AudioEncoder %*/ encoder = QScriptValue::NullValue /*% new CopyAudioEncoder() %*/);

		/** \brief Removes the AudioOutput object at the specified index.
		 */
		Q_INVOKABLE void removeAt(int /*% Number %*/ index);

		/** \brief Gets the AudioOutput object at the specified index.
		 */
		; /*% AudioOutput operator[](Number i); %*/
	};
}
#endif
