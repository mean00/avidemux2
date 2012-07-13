#ifndef ADM_qtScript_Editor
#define ADM_qtScript_Editor

#include <map>

#include "QtScriptObject.h"
#include "AudioOutputCollection.h"
#include "Muxer.h"
#include "VideoEncoder.h"

namespace ADM_qtScript
{
	/** \brief The Editor object exposes most of the video and audio editing functionality available in
	 * Avidemux's graphical user interface to the ECMA scripting engine.
	 *
	 * It can only be accessed using static properties and methods as it doesn't have a constructor.
	 * Editor is usually the first object that an individual should become familiar with
	 * when starting to write Avidemux ECMA scripts.
	 */
	class Editor : public QtScriptObject
	{
		Q_OBJECT
		Q_ENUMS(ImageType SeekFrameType)

	public:
		/** \brief Specifies a type of image file.
		 */
		enum ImageType
		{
			Bitmap = 1, /**< Bitmap file (.bmp) */
			Jpeg = 2, /**< JPEG file (.jpg) */
		};

		/** \brief Specifies a type of frame to seek.
		 */
		enum SeekFrameType
		{
			Any = 0, /**< Seek for any type of frame */
			Intra = 1, /**< Seek for an intra frame */
			Black = 2 /**< Seek for a black frame */
		};

	private:
		std::map<ADM_dynMuxer*, Muxer*>* _muxers;
		std::map<ADM_videoEncoder6*, VideoEncoder*>* _videoEncoders;
		QScriptEngine *_engine;

        QScriptValue getAppliedVideoFilters(void);
		QScriptValue getAudioOutputs(void);
		QScriptValue getCurrentFrameProperties(void);
		QScriptValue getCurrentTime(void);
		QScriptValue getMarkerA(void);
		QScriptValue getMarkerB(void);
		QScriptValue getMuxer(void);
		QScriptValue getPosition(void);
		QScriptValue getSegments(void);
		QScriptValue getVideoCount(void);
		QScriptValue getVideoDecoders(void);
		QScriptValue getVideoEncoder(void);
		QScriptValue getVideoFileProperties(void);
		QScriptValue getVideoFileProperties(int videoIndex);

		void seekFrame(int frameCount, SeekFrameType seekFrameType);
		QScriptValue setCurrentTime(QScriptValue time);
		QScriptValue setMarkerA(QScriptValue time);
		QScriptValue setMarkerB(QScriptValue time);
		QScriptValue setMuxer(QScriptValue muxer);
		QScriptValue setPosition(QScriptValue position);
		QScriptValue setVideoEncoder(QScriptValue encoder);
		QScriptValue validateMarker(const QString& parameterName, QScriptValue time);

	public:
		/** \cond */
		Editor(
			QScriptEngine *engine, IEditor *editor, std::map<ADM_dynMuxer*, Muxer*>* muxers,
			std::map<ADM_videoEncoder6*, VideoEncoder*>* videoEncoders);
		/** \endcond */

		/** \brief Gets the video decoders currently being used to decode the video files that are open in the editor.
		 *
		 * \return Returns an array of VideoDecoder objects if a video is open in the editor; otherwise, null.
		 * \sa openVideo and appendVideo
		 */
		Q_PROPERTY(QScriptValue /*% Array %*/ activeVideoDecoders READ getVideoDecoders);

        /** \brief Gets the filters that will be applied to the encoded video.
		 *
		 * \return Returns an array of VideoFilter objects if a video is open in the editor; otherwise, null.
		 */
        Q_PROPERTY(QScriptValue /*% VideoFilterCollection %*/ appliedVideoFilters READ getAppliedVideoFilters);

		/** \brief Gets the intended audio outputs for the encoded video.
		 */
		Q_PROPERTY(QScriptValue /*% AudioOutputCollection %*/ audioOutputs READ getAudioOutputs);

		/** \brief Gets extended information about the editor's currently selected frame.
		 *
		 * \return Returns a FrameProperties object if a video is open in the editor; otherwise, null.
		 */
		Q_PROPERTY(QScriptValue /*% FrameProperties %*/ currentFrameProperties READ getCurrentFrameProperties);

		/** \brief Gets or sets the time (in milliseconds) of the editor's navigation bar.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ currentFrameTime READ getCurrentTime WRITE setCurrentTime);

		/** \brief Gets or sets the current muxer.
		 *
		 * Example usage:
		 * \code
		 * Editor.currentMuxer = AviMuxer;
		 * \endcode
		 */
		Q_PROPERTY(QScriptValue /*% Muxer %*/ currentMuxer READ getMuxer WRITE setMuxer);

		/** \brief Gets or sets the current video encoder.
		 *
		 * Example usage:
		 * \code
		 * Editor.currentVideoEncoder = Xvid4VideoEncoder;
		 * \endcode
		 */
		Q_PROPERTY(QScriptValue /*% VideoEncoder %*/ currentVideoEncoder READ getVideoEncoder WRITE setVideoEncoder);

		/** \brief Gets or sets the time (in milliseconds) of marker A (selection start).
		 *
		 * \sa markerB, setMarkers and clearMarkers
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ markerA READ getMarkerA WRITE setMarkerA);

		/** \brief Gets or sets the time (in milliseconds) of marker B (selection end).
		 *
		 * \sa markerA, setMarkers and clearMarkers
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ markerB READ getMarkerB WRITE setMarkerB);

		/** \brief Gets or sets the time (in milliseconds) of the editor's position.
		 *
		 * \sa seekNextFrame and seekPreviousFrame
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ position READ getPosition WRITE setPosition);

		/** \brief Gets extended information about the video segments that have been added to the editor.
		 *
		 * \return Returns an array of Segment objects if a video is open in the editor; otherwise, null.
		 */
		Q_PROPERTY(QScriptValue /*% SegmentCollection %*/ segments READ getSegments);

		/** \brief Returns the number of video files currently open in the editor.
		 *
		 * \sa openVideo and appendVideo
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ videoCount READ getVideoCount);

		/** \brief Gets extended information about the video files that have been added to the editor.
		 *
		 * \return Returns an array of VideoFileProperties objects if a video is open in the editor; otherwise, null.
		 * \sa openVideo and appendVideo
		 */
		Q_PROPERTY(QScriptValue /*% Array %*/ videoFileProperties READ getVideoFileProperties);

		/** \brief Appends an additional video file to the end of the currently open video.
		 *
		 * \return Returns a VideoFileProperties object that contains extended information about the video file that has been appended to the editor.
		 * \sa isVideoOpen, openVideo and VideoFileProperties
		 */
		Q_INVOKABLE QScriptValue /*% VideoFileProperties %*/ appendVideo(const QString& /*% String %*/ path);

		/** \brief Sets marker A to the start of the video and marker B to the end of the video.
		 *
		 * \sa markerA, markerB and setMarkers
		 */
		Q_INVOKABLE void clearMarkers(void);

		/** \brief Closes the video in the editor.
		 *
		 * \sa isVideoOpen and openVideo
		 */
		Q_INVOKABLE void closeVideo();

		/** \brief Indicates whether a video file is currently open in the editor.
		 *
		 * \return Returns true if a video is open in the editor; otherwise, false.
		 * \sa openVideo
		 */
		Q_INVOKABLE QScriptValue /*% Boolean %*/ isVideoOpen(void);

		/** \brief Opens a video file at the given path in the editor.
		 *
		 * \return Returns a VideoFileProperties object that contains extended information about the video file that has been opened in the editor.
		 * \sa closeVideo, isVideoOpen and VideoFileProperties
		 */
		Q_INVOKABLE QScriptValue /*% VideoFileProperties %*/ openVideo(const QString& /*% String %*/ path);

		/** \brief Saves the specified audio track of the currently open video to a file.
		 *
		 * \sa VideoFileProperties.audioProperties
		 */
		Q_INVOKABLE QScriptValue /*% void %*/ saveAudio(const QString& /*% String %*/ path, int /*% Number %*/ audioIndex = 0);

		/** \brief Save the current frame as an image file.
		 *
		 * Example usage:
		 * \code
		 * Editor.saveImage("C:\\Test\\Test.bmp", Editor.ImageType.Bitmap);
		 * \endcode
		 *
		 * \sa currentFrameTime
		 */
		Q_INVOKABLE QScriptValue /*% void %*/ saveImage(const QString& /*% String %*/ path, ImageType imageType);

		/** \brief Encodes or copies the video to the specified file.
		 */
		Q_INVOKABLE QScriptValue /*% void %*/ saveVideo(const QString& /*% String %*/ path);

		/** \brief Seeks forward to the next frame(s) of the specified type.
		 *
		 * \sa seekPreviousFrame and position
		 */
		Q_INVOKABLE QScriptValue /*% void %*/ seekNextFrame(QScriptValue /*% Number %*/ frameCount = 1, SeekFrameType seekFrameType = Editor::Any);

		/** \brief Seeks backward to the next frame(s) of the specified type.
		 *
		 * \sa seekNextFrame and position
		 */
		Q_INVOKABLE QScriptValue /*% void %*/ seekPreviousFrame(QScriptValue /*% Number %*/ frameCount = 1, SeekFrameType seekFrameType = Editor::Any);

		/** \brief Sets the time (in milliseconds) of marker A (selection start) and marker B (selection end).
		 *
		 * \sa markerA, markerB and setMarkers
		 */
		Q_INVOKABLE QScriptValue /*% void %*/ setMarkers(QScriptValue /*% Number %*/ markerA, QScriptValue /*% Number %*/ markerB);
	};
}
#endif
