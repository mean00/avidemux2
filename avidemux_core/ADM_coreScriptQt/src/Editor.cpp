#include "ADM_inttype.h"
#include "DIA_coreToolkit.h"

#include "Editor.h"
#include "FrameProperties.h"
#include "SegmentProperties.h"
#include "VideoDecoder.h"
#include "VideoFileProperties.h"
#include "AudioOutputCollectionPrototype.h"
#include "VideoFilterCollection.h"
#include "VideoFilterCollectionPrototype.h"

#define VALIDATE_OPEN_VIDEO() \
    if (!this->_editor->getVideoCount()) \
    { \
        return this->throwError(QString(QT_TR_NOOP("A video must be open to perform this operation."))); \
    }

namespace ADM_qtScript
{
    Editor::Editor(
        QScriptEngine *engine, IEditor * editor, std::map<ADM_dynMuxer*, Muxer*>* muxers,
        std::map<ADM_videoEncoder6*, VideoEncoder*>* videoEncoders) : QtScriptObject(editor)
    {
        this->_engine = engine;
        this->_muxers = muxers;
        this->_videoEncoders = videoEncoders;
    }

    QScriptValue Editor::addSegment(
        QScriptValue startTime, QScriptValue duration, QScriptValue videoIndex)
    {
        VALIDATE_OPEN_VIDEO()

        QScriptValue result;

        while (true)
        {
            result = this->validateNumber("videoIndex", videoIndex, 0, this->_editor->getVideoCount());

            if (!result.isUndefined())
            {
                break;
            }

            this->_editor->addSegment(videoIndex.toNumber(), startTime.toNumber(), duration.toNumber());

            result = this->getSegmentProperties(this->_editor->getNbSegment() - 1);

            break;
        }

        return result;
    }

    QScriptValue Editor::appendVideo(const QString& path)
    {
        if (!this->_editor->appendFile(path.toUtf8().constData()))
        {
            return this->throwError(QString(QT_TR_NOOP("Unable to append %1")).arg(path));
        }

        return getVideoFileProperties(this->_editor->getVideoCount() - 1);
    }

    void Editor::clearMarkers()
    {
        this->_editor->setMarkerAPts(0);
        this->_editor->setMarkerBPts(this->_editor->getVideoDuration());
    }

    void Editor::closeVideo()
    {
        this->_editor->closeFile();
    }

    QScriptValue Editor::getSegmentProperties(int segmentIndex)
    {
        _SEGMENT *segment = this->_editor->getSegment(segmentIndex);

        return this->engine()->newQObject(
                   new SegmentProperties(this->_editor, segment), QScriptEngine::ScriptOwnership);
    }

    QScriptValue Editor::getSegmentProperties()
    {
        int count = this->_editor->getNbSegment();

        if (count == 0)
        {
            return QScriptValue(QScriptValue::NullValue);
        }
        else
        {
            QScriptValue segments = this->engine()->newArray(count);

            for (int segmentIndex = 0; segmentIndex < count; segmentIndex++)
            {
                segments.setProperty(
                    segmentIndex, this->getSegmentProperties(segmentIndex));
            }

            return segments;
        }
    }

    QScriptValue Editor::getVideoFileProperties(int videoIndex)
    {
        _VIDEOS *video = this->_editor->getRefVideo(videoIndex);

        return this->engine()->newQObject(
                   new VideoFileProperties(this->_editor, video), QScriptEngine::ScriptOwnership);
    }

    QScriptValue Editor::getVideoFileProperties()
    {
        int count = this->_editor->getVideoCount();

        if (count == 0)
        {
            return QScriptValue(QScriptValue::NullValue);
        }
        else
        {
            QScriptValue videos = this->engine()->newArray(count);

            for (int videoIndex = 0; videoIndex < count; videoIndex++)
            {
                videos.setProperty(
                    videoIndex, this->getVideoFileProperties(videoIndex));
            }

            return videos;
        }
    }

    QScriptValue Editor::isVideoOpen(void)
    {
        return this->engine()->toScriptValue((bool)this->_editor->isFileOpen());
    }

    QScriptValue Editor::openVideo(const QString& path)
    {
        if (!this->_editor->openFile(path.toUtf8().constData()))
        {
            return this->throwError(QString(QT_TR_NOOP("Unable to open %1")).arg(path));
        }

        return getVideoFileProperties(0);
    }

    QScriptValue Editor::saveAudio(const QString& path, int audioIndex)
    {
        VALIDATE_OPEN_VIDEO()

        if (!this->_editor->getRefVideo(0)->audioTracks.size())
        {
            return this->throwError(QString(QT_TR_NOOP("Video must contain an audio track to perform this operation.")));
        }

        this->_editor->saveAudio(audioIndex, path.toUtf8().constData());

        return QScriptValue(QScriptValue::UndefinedValue);
    }

    QScriptValue Editor::saveImage(const QString& path, ImageType imageType)
    {
        VALIDATE_OPEN_VIDEO()

        switch (imageType)
        {
            case Editor::Bitmap:
                _editor->saveImageBmp(path.toUtf8().constData());
                break;

            case Editor::Jpeg:
                _editor->saveImageJpg(path.toUtf8().constData());
                break;
        }

        return QScriptValue(QScriptValue::UndefinedValue);
    }

    QScriptValue Editor::saveVideo(const QString& path)
    {
        VALIDATE_OPEN_VIDEO()

        this->_editor->saveFile(path.toUtf8().constData());

        return QScriptValue(QScriptValue::UndefinedValue);
    }

    void Editor::seekFrame(int frameCount, SeekFrameType seekFrameType)
    {
        switch (seekFrameType)
        {
            case Editor::Intra:
                this->_editor->seekKeyFrame(frameCount);
                break;

            case Editor::Black:
                this->_editor->seekBlackFrame(frameCount);
                break;

            default:
                this->_editor->seekFrame(frameCount);
                break;
        }
    }

    QScriptValue Editor::seekNextFrame(QScriptValue frameCount, SeekFrameType seekFrameType)
    {
        QScriptValue result = this->validateNumber("frameCount", frameCount, 1, INT_MAX);

        if (result.isUndefined())
        {
            this->seekFrame(frameCount.toNumber(), seekFrameType);
        }

        return result;
    }

    QScriptValue Editor::seekPreviousFrame(QScriptValue frameCount, SeekFrameType seekFrameType)
    {
        QScriptValue result = this->validateNumber("frameCount", frameCount, 1, INT_MAX);

        if (result.isUndefined())
        {
            this->seekFrame(-frameCount.toNumber(), seekFrameType);
        }

        return result;
    }

    QScriptValue Editor::getAppliedVideoFilters(void)
    {
        if (this->_editor->isFileOpen())
        {
            return this->engine()->newObject(
                       new VideoFilterCollection(
                           this->engine(), this->_editor), QScriptEngine::ScriptOwnership);
        }
        else
        {
            return QScriptValue(QScriptValue::NullValue);
        }
    }

    QScriptValue Editor::getAudioOutputs(void)
    {
        if (this->_editor->isFileOpen())
        {
            return this->engine()->newObject(
                       new AudioOutputCollection(
                           this->engine(), this->_editor), QScriptEngine::ScriptOwnership);
        }
        else
        {
            return QScriptValue(QScriptValue::NullValue);
        }
    }

    QScriptValue Editor::getCurrentFrameProperties(void)
    {
        if (this->_editor->isFileOpen())
        {
            return this->engine()->newQObject(
                       new FrameProperties(
                           this->_editor, this->_editor->getCurrentFramePts()), QScriptEngine::ScriptOwnership);
        }
        else
        {
            return QScriptValue(QScriptValue::NullValue);
        }
    }

    QScriptValue Editor::getCurrentTime(void)
    {
        return (double)this->_editor->getCurrentFramePts();
    }

    QScriptValue Editor::getMarkerA(void)
    {
        return (double)this->_editor->getMarkerAPts();
    }

    QScriptValue Editor::getMarkerB(void)
    {
        return (double)this->_editor->getMarkerBPts();
    }

    QScriptValue Editor::getPosition(void)
    {
        return (double)this->_editor->getCurrentFramePts();
    }

    QScriptValue Editor::getTotalSegmentDuration(void)
    {
        return (double)this->_editor->getVideoDuration();
    }

    QScriptValue Editor::getVideoCount(void)
    {
        return this->_editor->getVideoCount();
    }

    QScriptValue Editor::getVideoDecoders(void)
    {
        int count = this->_editor->getVideoCount();

        if (count == 0)
        {
            return QScriptValue(QScriptValue::NullValue);
        }
        else
        {
            QScriptValue videos = this->engine()->newArray(count);

            for (int videoIndex = 0; videoIndex < count; videoIndex++)
            {
                videos.setProperty(
                    videoIndex, this->engine()->newQObject(
                        new VideoDecoder(this->_engine, this->_editor, this->_editor->getRefVideo(videoIndex)),
                        QScriptEngine::ScriptOwnership));
            }

            return videos;
        }
    }

    QScriptValue Editor::setCurrentTime(QScriptValue time)
    {
        QScriptValue result = this->validateMarker("time", time);

        if (result.isUndefined())
        {
            this->_editor->setCurrentFramePts(time.toNumber());
            result = time;
        }

        return result;
    }

    QScriptValue Editor::setMarkerA(QScriptValue time)
    {
        QScriptValue result = this->validateMarker("time", time);

        if (result.isUndefined())
        {
            this->_editor->setMarkerAPts(time.toNumber());
            result = time;
        }

        return result;
    }

    QScriptValue Editor::setMarkerB(QScriptValue time)
    {
        QScriptValue result = this->validateMarker("time", time);

        if (result.isUndefined())
        {
            this->_editor->setMarkerBPts(time.toNumber());
            result = time;
        }

        return result;
    }

    QScriptValue Editor::setMarkers(QScriptValue markerA, QScriptValue markerB)
    {
        QScriptValue result;

        result = this->validateMarker("markerA", markerA);

        if (result.isUndefined())
        {
            result = this->validateMarker("markerB", markerB);

            if (result.isUndefined())
            {
                this->_editor->setMarkerAPts(markerA.toNumber());
                this->_editor->setMarkerBPts(markerB.toNumber());
            }
        }

        return result;
    }

    QScriptValue Editor::setPosition(QScriptValue position)
    {
        QScriptValue result = this->validateMarker("position", position);

        if (result.isUndefined())
        {
            this->_editor->setCurrentFramePts(position.toNumber());
            result = QScriptValue((double)this->_editor->getCurrentFramePts());
        }

        return result;
    }

    QScriptValue Editor::validateMarker(const QString& parameterName, QScriptValue time)
    {
        return this->validateNumber(parameterName, time, 0, this->_editor->getVideoDuration());
    }

    QScriptValue Editor::getMuxer(void)
    {
        ADM_dynMuxer *muxerPlugin = this->_editor->getCurrentMuxer();

        return this->engine()->newQObject(
                   this->_muxers->find(muxerPlugin)->second, QScriptEngine::ScriptOwnership);
    }

    QScriptValue Editor::setMuxer(QScriptValue muxer)
    {
        Muxer *muxerObject = qobject_cast<Muxer*>(muxer.toQObject());

        if (muxerObject != NULL)
        {
            this->_editor->setContainer(muxerObject->muxerPlugin->name, NULL);

            return muxer;
        }

        return this->engine()->undefinedValue();
    }

    QScriptValue Editor::getVideoEncoder(void)
    {
        ADM_videoEncoder6 *encoderPlugin = this->_editor->getCurrentVideoEncoder();

        return this->engine()->newQObject(
                   this->_videoEncoders->find(encoderPlugin)->second, QScriptEngine::ScriptOwnership);
    }

    QScriptValue Editor::setVideoEncoder(QScriptValue encoder)
    {
        VideoEncoder *encoderObject = qobject_cast<VideoEncoder*>(encoder.toQObject());

        if (encoderObject != NULL)
        {
            this->_editor->setVideoCodec(encoderObject->encoderPlugin->desc->encoderName, NULL);
            return encoder;
        }

        return this->engine()->undefinedValue();
    }
}
