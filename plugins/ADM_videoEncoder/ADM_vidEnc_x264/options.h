/***************************************************************************
                                x264Options.h

    begin                : Mon Apr 21 2008
    copyright            : (C) 2008 by gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef options_h
#define options_h

#include <vector>
#include <libxml/tree.h>

#include "zoneOptions.h"

extern "C"
{
#include "x264.h"
#include "ADM_vidEnc_plugin.h"
}

#define DEFAULT_ENCODE_MODE ADM_VIDENC_MODE_AQP
#define DEFAULT_ENCODE_MODE_PARAMETER 26

typedef enum
{
	CONFIG_CUSTOM,
	CONFIG_DEFAULT,
	CONFIG_USER,
	CONFIG_SYSTEM
} configType;

class x264Options
{
protected:
	x264_param_t _param;
	std::vector<x264ZoneOptions*> _zoneOptions;

	char* _configurationName;
	configType _configurationType;

	bool _sarAsInput;

	void cleanUp(void);
	xmlChar* number2String(xmlChar *buffer, size_t size, int number);
	xmlChar* number2String(xmlChar *buffer, size_t size, unsigned int number);
	xmlChar* number2String(xmlChar *buffer, size_t size, float number);
	xmlChar* boolean2String(xmlChar *buffer, size_t size, bool boolean);
	bool string2Boolean(char *buffer);

	void addX264OptionsToXml(xmlNodePtr xmlNodeRoot);
	char* dumpXmlDocToMemory(xmlDocPtr xmlDoc);
	bool validateXml(xmlDocPtr doc);
	void parsePresetConfiguration(xmlNode *node);
	void parseX264Options(xmlNode *node);

private:
	void parseVuiOptions(xmlNode *node);
	void parseCqmOption(xmlNode *node, uint8_t cqm[]);
	void parseAnalyseOptions(xmlNode *node);
	void parseRateControlOptions(xmlNode *node);
	void parseZoneOptions(xmlNode *zoneNode);

public:
	x264Options(void);
	~x264Options(void);

	void reset(void);
	x264_param_t* getParameters(void);

	void getPresetConfiguration(char** configurationName, configType *configurationType);
	void setPresetConfiguration(const char* configurationName, configType configurationType);
	void clearPresetConfiguration(void);

	int getThreads(void);
	void setThreads(int threads);

	bool getDeterministic(void);
	void setDeterministic(bool deterministic);

	unsigned int getIdcLevel(void);
	void setIdcLevel(unsigned int idcLevel);

	bool getSarAsInput(void);
	void setSarAsInput(bool sarAsInput);

	unsigned int getSarHeight(void);
	void setSarHeight(unsigned int height);

	unsigned int getSarWidth(void);
	void setSarWidth(unsigned int width);

	unsigned int getOverscan(void);
	void setOverscan(unsigned int overscan);

	unsigned int getVideoFormat(void);
	void setVideoFormat(unsigned int videoFormat);

	bool getFullRangeSamples(void);
	void setFullRangeSamples(bool fullRangeSamples);

	unsigned int getColorPrimaries(void);
	void setColorPrimaries(unsigned int colorPrimaries);

	unsigned int getTransfer(void);
	void setTransfer(unsigned int transfer);

	unsigned int getColorMatrix(void);
	void setColorMatrix(unsigned int transfer);

	unsigned int getChromaSampleLocation(void);
	void setChromaSampleLocation(unsigned int chromaSampleLocation);

	unsigned int getReferenceFrames(void);
	void setReferenceFrames(unsigned int referenceFrames);

	unsigned int getGopMinimumSize(void);
	void setGopMinimumSize(unsigned int gopSize);

	unsigned int getGopMaximumSize(void);
	void setGopMaximumSize(unsigned int gopSize);

	unsigned int getScenecutThreshold(void);
	void setScenecutThreshold(unsigned int scenecutThreshold);

	bool getPreScenecutDetection(void);
	void setPreScenecutDetection(bool preScenecutDetection);

	unsigned int getBFrames(void);
	void setBFrames(unsigned int bFrames);

	unsigned int getAdaptiveBFrameDecision(void);
	void setAdaptiveBFrameDecision(unsigned int adaptiveBframeDecision);

	int getBFrameBias(void);
	void setBFrameBias(int bFrameBias);

	bool getBFrameReferences(void);
	void setBFrameReferences(bool bFrameReferences);

	bool getLoopFilter(void);
	void setLoopFilter(bool loopFilter);

	int getLoopFilterAlphaC0(void);
	void setLoopFilterAlphaC0(int loopFilterAlphaC0);

	int getLoopFilterBeta(void);
	void setLoopFilterBeta(int loopFilterBeta);

	bool getCabac(void);
	void setCabac(bool cabac);

	bool getInterlaced(void);
	void setInterlaced(bool interlaced);

	unsigned int getCqmPreset(void);
	void setCqmPreset(unsigned int cqmPreset);

	uint8_t* getIntra4x4Luma(void);
	void setIntra4x4Luma(uint8_t intra4x4Luma[]);

	uint8_t* getIntraChroma(void);
	void setIntraChroma(uint8_t intraChroma[]);

	uint8_t* getInter4x4Luma(void);
	void setInter4x4Luma(uint8_t inter4x4Luma[]);

	uint8_t* getInterChroma(void);
	void setInterChroma(uint8_t interChroma[]);

	uint8_t* getIntra8x8Luma(void);
	void setIntra8x8Luma(uint8_t intra8x8Luma[]);

	uint8_t* getInter8x8Luma(void);
	void setInter8x8Luma(uint8_t inter8x8Luma[]);

	bool getPartitionI4x4(void);
	void setPartitionI4x4(bool partitionI4x4);

	bool getPartitionI8x8(void);
	void setPartitionI8x8(bool partitionI8x8);

	bool getPartitionP8x8(void);
	void setPartitionP8x8(bool partitionP8x8);

	bool getPartitionP4x4(void);
	void setPartitionP4x4(bool partitionP4x4);

	bool getPartitionB8x8(void);
	void setPartitionB8x8(bool partitionB8x8);

	bool getDct8x8(void);
	void setDct8x8(bool dct8x8);

	bool getWeightedPrediction(void);
	void setWeightedPrediction(bool weightedPrediction);

	unsigned int getDirectPredictionMode(void);
	void setDirectPredictionMode(unsigned int directPredictionMode);

#if X264_BUILD < 66
	int getDirectPredictionSize(void);
	void setDirectPredictionSize(int directPredictionSize);
#endif

	int getChromaLumaQuantiserDifference(void);
	void setChromaLumaQuantiserDifference(int chromaLumaQuantiserDifference);

	unsigned int getMotionEstimationMethod(void);
	void setMotionEstimationMethod(unsigned int motionEstimationMethod);

	unsigned int getMotionVectorSearchRange(void);
	void setMotionVectorSearchRange(unsigned int motionVectorSearchRange);

	int getMotionVectorLength(void);
	void setMotionVectorLength(int motionVectorLength);

	int getMotionVectorThreadBuffer(void);
	void setMotionVectorThreadBuffer(int motionVectorThreadBuffer);

	unsigned int getSubpixelRefinement(void);
	void setSubpixelRefinement(unsigned int subpixelRefinement);

#if X264_BUILD < 65
	bool getBidirectionalMotionEstimation(void);
	void setBidirectionalMotionEstimation(bool bidirectionalMotionEstimation);
#endif

	bool getChromaMotionEstimation(void);
	void setChromaMotionEstimation(bool chromaMotionEstimation);

#if X264_BUILD < 65
	bool getBFrameRdo(void);
	void setBFrameRdo(bool bFrameRdo);
#endif

	bool getMixedReferences(void);
	void setMixedReferences(bool mixedReferences);

	unsigned int getTrellis(void);
	void setTrellis(unsigned int trellis);

	bool getFastPSkip(void);
	void setFastPSkip(bool fastPSkip);

	bool getDctDecimate(void);
	void setDctDecimate(bool dctDecimate);

	unsigned int getNoiseReduction(void);
	void setNoiseReduction(unsigned int noiseReduction);

	unsigned int getInterLumaDeadzone(void);
	void setInterLumaDeadzone(unsigned int interLumaDeadzone);

	unsigned int getIntraLumaDeadzone(void);
	void setIntraLumaDeadzone(unsigned int intraLumaDeadzone);

	bool getComputePsnr(void);
	void setComputePsnr(bool computePsnr);

	bool getComputeSsim(void);
	void setComputeSsim(bool computeSsim);

	unsigned int getQuantiserMinimum(void);
	void setQuantiserMinimum(unsigned int quantiserMinimum);

	unsigned int getQuantiserMaximum(void);
	void setQuantiserMaximum(unsigned int quantiserMaximum);

	unsigned int getQuantiserStep(void);
	void setQuantiserStep(unsigned int quantiserStep);

	float getAverageBitrateTolerance(void);
	void setAverageBitrateTolerance(float averageBitrateTolerance);

	unsigned int getVbvMaximumBitrate(void);
	void setVbvMaximumBitrate(unsigned int vbvMaximumBitrate);

	unsigned int getVbvBufferSize(void);
	void setVbvBufferSize(unsigned int vbvBufferSize);

	float getVbvInitialOccupancy(void);
	void setVbvInitialOccupancy(float vbvInitialOccupancy);

	float getIpFrameQuantiser(void);
	void setIpFrameQuantiser(float ipFrameQuantiser);

	float getPbFrameQuantiser(void);
	void setPbFrameQuantiser(float pbFrameQuantiser);

#if X264_BUILD >= 62
	unsigned int getAdaptiveQuantiserMode(void);
	void setAdaptiveQuantiserMode(unsigned int adaptiveQuantiserMode);

	float getAdaptiveQuantiserStrength(void);
	void setAdaptiveQuantiserStrength(float adaptiveQuantiserStrength);
#endif

	float getQuantiserCurveCompression(void);
	void setQuantiserCurveCompression(float quantiserCurveCompression);

	float getReduceFluxBeforeCurveCompression(void);
	void setReduceFluxBeforeCurveCompression(float reduceFluxBeforeCurveCompression);

	float getReduceFluxAfterCurveCompression(void);
	void setReduceFluxAfterCurveCompression(float reduceFluxAfterCurveCompression);

	unsigned int getZoneCount(void);
	x264ZoneOptions** getZones(void);
	void clearZones(void);
	void addZone(x264ZoneOptions *zoneOptions);

	bool getAccessUnitDelimiters(void);
	void setAccessUnitDelimiters(bool accessUnitDelimiters);

	unsigned int getSpsIdentifier(void);
	void setSpsIdentifier(unsigned int spsIdentifier);

	virtual char* toXml(void);
	virtual int fromXml(const char *xml);
};

#endif	// options_h
