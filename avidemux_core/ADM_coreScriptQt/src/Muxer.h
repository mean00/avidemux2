#ifndef ADM_qtScript_Muxer
#define ADM_qtScript_Muxer

#include "QtScriptConfigObject.h"
#include "ADM_muxerInternal.h"

namespace ADM_qtScript
{
	/** \brief The Muxer %class is the base class of all Avidemux muxers.
	 *
	 * Depending on the plugins that are packaged with Avidemux, the following child muxer objects
	 * may be available to the ECMA scripting engine:
	 * \li AviMuxer
	 * \li DummyMuxer
	 * \li FfpsMuxer
	 * \li FftsMuxer
	 * \li FlvMuxer
	 * \li MkvMuxer
	 * \li Mp4Muxer
	 * \li Mp4v2Muxer
	 * \li RawMuxer
	 *
	 * Example usage:
	 * \code
	 * AviMuxer.resetConfiguration();
	 * AviMuxer.configuration.odmlType = 2;
	 * \endcode
	 */
	class Muxer : public QtScriptConfigObject
	{
		Q_OBJECT

	private:
		QScriptValue _configObject;

		void getConfCouple(CONFcouple **conf, const QString& containerName = QString());
		QScriptValue getConfiguration(void);
		QScriptValue getDefaultExtension(void);
		QScriptValue getName(void);
		void setConfCouple(CONFcouple *conf, const QString& containerName = QString());

	public:
		/** \cond */
		ADM_dynMuxer *muxerPlugin;

		Muxer(QScriptEngine *engine, IEditor *editor, ADM_dynMuxer *muxer);
		/** \endcond */

		/** \brief Gets an object that holds parameters used to configure the muxer.
		 *
		 * \sa resetConfiguration
		 */
		Q_PROPERTY(QScriptValue /*% Object %*/ configuration READ getConfiguration);

		/** \brief Gets the default file extension of the muxer.
		 */
		Q_PROPERTY(QScriptValue /*% String %*/ defaultFileExtension READ getDefaultExtension);

		/** \brief Gets the user friendly name of the muxer.
		 */
		Q_PROPERTY(QScriptValue /*% String %*/ name READ getName);

		/** \brief Resets the muxer back to its default configuration.
		 *
		 * \sa configuration
		 */
		Q_INVOKABLE void resetConfiguration(void);
	};
}

#endif
