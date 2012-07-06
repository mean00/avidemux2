#ifndef ADM_qtScript_VideoFilter
#define ADM_qtScript_VideoFilter

#include "QtScriptConfigObject.h"
#include "ADM_coreVideoFilterInternal.h"
#include "VideoFilterShim.h"

namespace ADM_qtScript
{
	/** \brief The VideoFilter %class is the base class of all Avidemux video filters.
	 *
	 * Depending on the plugins that are packaged with Avidemux, the following video filter classes
	 * may be available to the ECMA scripting engine:
	 * \li AddborderVideoFilter
	 * \li AddlogoVideoFilter
	 * \li AsharpVideoFilter
	 * \li BlackenborderVideoFilter
	 * \li ChangefpsVideoFilter
	 * \li ChromashiftVideoFilter
	 * \li ColoryuvVideoFilter
	 * \li ContrastVideoFilter
	 * \li CropVideoFilter
	 * \li DecimateVideoFilter
	 * \li DgbobVideoFilter
	 * \li DummyVideoFilter
	 * \li Eq2VideoFilter
	 * \li FluxsmoothVideoFilter
	 * \li GaussianVideoFilter
	 * \li GlbenchmarkVideoFilter
	 * \li GlresizeVideoFilter
	 * \li GlrotateVideoFilter
	 * \li GlsampledistortVideoFilter
	 * \li Glsamplefragment2VideoFilter
	 * \li HueVideoFilter
	 * \li HflipVideoFilter
	 * \li HzstackfieldVideoFilter
	 * \li KerndeldeintVideoFilter
	 * \li LargemedianVideoFilter
	 * \li LavdeintVideoFilter
	 * \li LumaonlyVideoFilter
	 * \li MeanVideoFilter
	 * \li MedianVideoFilter
	 * \li MergefieldsVideoFilter
	 * \li MpdelogoVideoFilter
	 * \li Mplayerdenoise3dhqVideoFilter
	 * \li Mplayerdenoise3dVideoFilter
	 * \li MsharpenVideoFilter
	 * \li PrintinfoVideoFilter
	 * \li ResamplefpsVideoFilter
	 * \li RotateVideoFilter
	 * \li RplaneVideoFilter
	 * \li SeparatefieldsVideoFilter
	 * \li SharpenVideoFilter
	 * \li SsaVideoFilter
	 * \li StackfieldVideoFilter
	 * \li SwapuvVideoFilter
	 * \li SwscaleVideoFilter
	 * \li TelecideVideoFilter
	 * \li UnstackfieldVideoFilter
	 * \li VflipVideoFilter
	 * \li YadifVideoFilter
	 */
	class VideoFilter : public QtScriptConfigObject
	{
		Q_OBJECT

	private:
		VideoFilterShim *_videoFilterShim;
		QScriptValue _configObject;
		ADM_coreVideoFilter* _filter;
		CONFcouple *_defaultConf;
		bool _attachedToFilterChain;
		int _trackObjectId;

		QScriptValue getConfiguration();
		CONFcouple* getFilterConfiguration();
		QString getName();
		QScriptValue getVideoOutput();
		void setConfCouple(CONFcouple* conf, const QString& containerName = QString());
		void setFilterConfiguration(CONFcouple *couple);
		bool verifyFilter();

	public:
		/** \cond */
		ADM_vf_plugin *filterPlugin;
        static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);

		VideoFilter(QScriptEngine *engine, IEditor *editor, ADM_vf_plugin *plugin);
		VideoFilter(QScriptEngine *engine, IEditor *editor, ADM_VideoFilterElement *element);
		~VideoFilter();

		void getConfCouple(CONFcouple** conf, const QString& containerName = QString());
		bool isFilterUsed();
		void setFilterAsUsed(ADM_VideoFilterElement* element);
		/** \endcond */

		/** \brief Gets an object that holds parameters used to configure the video filter.
		 *
		 * \sa resetConfiguration
		 */
		Q_PROPERTY(QScriptValue /*% Object %*/ configuration READ getConfiguration);

		/** \brief Gets the user friendly name of the video filter.
		 */
		Q_PROPERTY(QString /*% String %*/ name READ getName);

        /** \brief Gets the properties of the video after the filter has been applied.
		 *
		 * \return Returns a VideoOutput object if the filter has been applied to the video to the editor; otherwise, null.
		 */
		Q_PROPERTY(QScriptValue /*% VideoOutput %*/ videoOutput READ getVideoOutput);

		/** \brief Resets the video filter back to its default configuration.
		 *
		 * \sa configuration
		 */
		Q_INVOKABLE void resetConfiguration(void);
	};
}

#endif
