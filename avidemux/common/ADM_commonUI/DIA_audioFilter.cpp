//
// C++ Implementation: ADM_vidForcedPP
//
// Description: 
//
//	Force postprocessing assuming constant quant & image type
//	Uselefull on some badly authored DVD for example
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "ADM_default.h"
#include "audiofilter_conf.h"
#include "DIA_factory.h"
#include <cmath>
/**
    \fn DIA_getAudioFilter
    \brief Dialog to manage audio filters
*/
int DIA_getAudioFilter(ADM_AUDIOFILTER_CONFIG *config)
{
    uint32_t vChan=config->mixerConf;
    uint32_t vFilm=config->film2pal;
    uint32_t vGainMode=(uint32_t)config->gainParam.mode;
    int32_t  vShift=config->shiftInMs;
    uint32_t bShiftEnabled=config->shiftEnabled;
    ELEM_TYPE_FLOAT vGainValue=config->gainParam.gain10/10.;
    ELEM_TYPE_FLOAT vGainMaxLevel=config->gainParam.maxlevel10/10.;

    bool drcUseGain = (config->drcConf.mUseGain != 0);
    ELEM_TYPE_FLOAT drcFloorDB = 10.0*std::log10(config->drcConf.mFloor);     // convert to DB; factor of 10 because it's power
    ELEM_TYPE_FLOAT drcAttackTime = config->drcConf.mAttackTime;
    ELEM_TYPE_FLOAT drcDecayTime = config->drcConf.mDecayTime;
    ELEM_TYPE_FLOAT drcRatio = config->drcConf.mRatio;
    ELEM_TYPE_FLOAT drcThresholdDB = config->drcConf.mThresholdDB;

    ELEM_TYPE_FLOAT chGainDB[ADM_CH_LAST];
    int             chDelayMS[ADM_CH_LAST];
    for (int i=0; i<ADM_CH_LAST; i++)
    {
        chGainDB[i] = config->chansConf.chGainDB[i];
        chDelayMS[i] = config->chansConf.chDelayMS[i];
    }

    bool bChRemapEnabled = config->chansConf.enableRemap;
    uint32_t vChRemap[9];
    for (int i=0; i<9; i++)
        vChRemap[i] = config->chansConf.remap[i];

    bool vEqEnable = config->eqConf.enable;
    ELEM_TYPE_FLOAT eqLowDB = config->eqConf.lowDB;
    ELEM_TYPE_FLOAT eqMidDB = config->eqConf.midDB;
    ELEM_TYPE_FLOAT eqHighDB = config->eqConf.highDB;
    ELEM_TYPE_FLOAT eqCutLM = config->eqConf.cutOffLM;
    ELEM_TYPE_FLOAT eqCutMH = config->eqConf.cutOffMH;


    #define PX(x) (&(config->x))
    diaElemToggleUint eResample(PX(resamplerEnabled),QT_TRANSLATE_NOOP("adm","R_esampling (Hz):"),PX(resamplerFrequency),QT_TRANSLATE_NOOP("adm","Resampling frequency (Hz)"),6000,64000);

    //**********************************
    diaMenuEntry menuFPS[]={
        {FILMCONV_NONE,     QT_TRANSLATE_NOOP("adm","None"), NULL},
        {FILMCONV_FILM2PAL, QT_TRANSLATE_NOOP("adm","Film to PAL"), NULL},
        {FILMCONV_PAL2FILM, QT_TRANSLATE_NOOP("adm","PAL to Film"), NULL}
    };
    #define NB_ITEMS(x) sizeof(x)/sizeof(diaMenuEntry)
    diaElemMenu      eFPS(&vFilm,QT_TRANSLATE_NOOP("adm","_Frame rate change:"),NB_ITEMS(menuFPS),menuFPS);

    //**********************************
    diaMenuEntry menuMixer[]={
        {CHANNEL_INVALID,     QT_TRANSLATE_NOOP("adm","No change"), NULL},
        {CHANNEL_MONO,        QT_TRANSLATE_NOOP("adm","Mono"), NULL},
        {CHANNEL_STEREO,      QT_TRANSLATE_NOOP("adm","Stereo"), NULL},
        {CHANNEL_STEREO_HEADPHONES, QT_TRANSLATE_NOOP("adm","Stereo headphones"), NULL},
        {CHANNEL_2F_1R,       QT_TRANSLATE_NOOP("adm","Stereo+surround"), NULL},
        {CHANNEL_3F,          QT_TRANSLATE_NOOP("adm","Stereo+center"), NULL},
        {CHANNEL_3F_1R,           QT_TRANSLATE_NOOP("adm","Stereo+center+surround"), NULL},
        {CHANNEL_2F_2R,           QT_TRANSLATE_NOOP("adm","Stereo front+stereo rear"), NULL},
        {CHANNEL_3F_2R,           QT_TRANSLATE_NOOP("adm","5 channels"), NULL},
        {CHANNEL_3F_2R_LFE,       QT_TRANSLATE_NOOP("adm","5.1"), NULL},
        {CHANNEL_DOLBY_PROLOGIC,  QT_TRANSLATE_NOOP("adm","Dolby Pro Logic"), NULL},
        {CHANNEL_DOLBY_PROLOGIC2, QT_TRANSLATE_NOOP("adm","Dolby Pro Logic II"), NULL},
        {CHANNEL_SURROUND_HEADPHONES, QT_TRANSLATE_NOOP("adm","Surround headphones"), NULL}
    };
    //*************************
    diaMenuEntry menuGain[]={
        {ADM_NO_GAIN,       QT_TRANSLATE_NOOP("adm","None"), NULL},
        {ADM_GAIN_AUTOMATIC,QT_TRANSLATE_NOOP("adm","Automatic"), NULL},
        {ADM_GAIN_MANUAL,   QT_TRANSLATE_NOOP("adm","Manual (dB)"), NULL}
    };

    diaElemFrame  frameGain(QT_TRANSLATE_NOOP("adm","Gain"));
    diaElemMenu   eGain(&vGainMode,QT_TRANSLATE_NOOP("adm","_Gain mode:"),NB_ITEMS(menuGain),menuGain);
    diaElemFloat  eGainValue(&vGainValue,QT_TRANSLATE_NOOP("adm","G_ain value:"),-10,40);
    diaElemFloat  eGainMaxLevel(&vGainMaxLevel,QT_TRANSLATE_NOOP("adm","_Maximum value:"),-10,0);

    eGain.link(&(menuGain[2]),1,&eGainValue);
    eGain.link(&(menuGain[1]),1,&eGainMaxLevel);
    frameGain.swallow(&eGain);
    frameGain.swallow(&eGainValue);
    frameGain.swallow(&eGainMaxLevel);
    //****************************

    diaElemMenu      eMixer(&vChan,QT_TRANSLATE_NOOP("adm","_Mixer:"),NB_ITEMS(menuMixer),menuMixer);
    bool bMixer=config->mixerEnabled;
    diaElemToggle    tMixer(&bMixer,QT_TRANSLATE_NOOP("adm","Remix"));
    tMixer.link(1,&eMixer);

    diaElemFrame frameMixer(QT_TRANSLATE_NOOP("adm","Mixer"));
    frameMixer.swallow(&tMixer);
    frameMixer.swallow(&eMixer);

    //****************************
    diaElemToggleInt eShift(&bShiftEnabled,QT_TRANSLATE_NOOP("adm","Shift audio (ms):"),&vShift, QT_TRANSLATE_NOOP("adm","Shift Value (ms):"),-30000,30000);
    /************************************/
    #define NB_ELEM(x) sizeof(x)/sizeof(diaElem *)
    diaElem *mainElems[]={&eFPS, &eResample,&eShift,&frameMixer,&frameGain};
    diaElemTabs tabMain(QT_TRANSLATE_NOOP("adm","Main"),NB_ELEM(mainElems),mainElems);

    //*** DRC tab *****
    diaElemReadOnlyText dirtyHack1(NULL," ",NULL);
    diaElemToggle    tDRC(PX(drcEnabled),QT_TRANSLATE_NOOP("adm","Enable Compressor"));
    diaElemToggle    eDrcNorm(&drcUseGain,QT_TRANSLATE_NOOP("adm","Normalize"));
    diaElemFloatResettable  eDrcThres(&drcThresholdDB,QT_TRANSLATE_NOOP("adm","Threshold (dB):"),-60,-1,-12);
    diaElemFloatResettable  eDrcNFloor(&drcFloorDB,QT_TRANSLATE_NOOP("adm","Noise floor (dB):"),-80,-20,-30);
    diaElemFloatResettable  eDrcRatio(&drcRatio,QT_TRANSLATE_NOOP("adm","Ratio:"),1.1,10,2);
    diaElemFloatResettable  eDrcAttack(&drcAttackTime,QT_TRANSLATE_NOOP("adm","Attack time (sec):"),0.1,5,0.2);
    diaElemFloatResettable  eDrcDecay(&drcDecayTime,QT_TRANSLATE_NOOP("adm","Release time (sec):"),1,30,1);
    tDRC.link(1, &eDrcNorm);
    tDRC.link(1, &eDrcThres);
    tDRC.link(1, &eDrcNFloor);
    tDRC.link(1, &eDrcRatio);
    tDRC.link(1, &eDrcAttack);
    tDRC.link(1, &eDrcDecay);
    diaElem *drcElems[]={&dirtyHack1, &tDRC, &eDrcNorm, &eDrcThres, &eDrcNFloor, &eDrcRatio, &eDrcAttack, &eDrcDecay};
    diaElemTabs tabDRC(QT_TRANSLATE_NOOP("adm","DRC"),NB_ELEM(drcElems),drcElems);

    //*** Equalizer tab ******
    diaElemReadOnlyText dirtyHack2(NULL," ",NULL);
    diaElemToggle    tEQ(&vEqEnable,QT_TRANSLATE_NOOP("adm","Enable Equalizer"));
    diaElemFloatResettable  eEqLow(&eqLowDB,QT_TRANSLATE_NOOP("adm","Bass (dB):"),-30,+30,0);
    diaElemFloatResettable  eEqCutLM(&eqCutLM,QT_TRANSLATE_NOOP("adm","Bass/Mid cut-off (Hz):"),100,1000,880);
    diaElemFloatResettable  eEqMid(&eqMidDB,QT_TRANSLATE_NOOP("adm","Mid (dB):"),-30,+30,0);
    diaElemFloatResettable  eEqCutMH(&eqCutMH,QT_TRANSLATE_NOOP("adm","Mid/Treble cut-off (Hz):"),2000,10000,5000);
    diaElemFloatResettable  eEqHigh(&eqHighDB,QT_TRANSLATE_NOOP("adm","Treble (dB):"),-30,+30,0);
    tEQ.link(1, &eEqLow);
    tEQ.link(1, &eEqCutLM);
    tEQ.link(1, &eEqMid);
    tEQ.link(1, &eEqCutMH);
    tEQ.link(1, &eEqHigh);

    diaElemReadOnlyText noteEq(NULL,QT_TRANSLATE_NOOP("adm","Note:\n - it is highly recommended to enable normalization when positive gain values are used"),NULL);

    diaElem *eqElems[]={&dirtyHack2, &tEQ, &eEqLow, &eEqMid, &eEqHigh, &eEqCutLM, &eEqCutMH, &noteEq};
    diaElemTabs tabEq(QT_TRANSLATE_NOOP("adm","Equalizer"),NB_ELEM(eqElems),eqElems);

    //*** Channel gains tab ******
    diaElemFloat  eChGainFLValue(chGainDB+ADM_CH_FRONT_LEFT,QT_TRANSLATE_NOOP("adm","Front left (dB):"),-30,+30);
    diaElemFloat  eChGainFRValue(chGainDB+ADM_CH_FRONT_RIGHT,QT_TRANSLATE_NOOP("adm","Front right (dB):"),-30,+30);
    diaElemFloat  eChGainFCValue(chGainDB+ADM_CH_FRONT_CENTER,QT_TRANSLATE_NOOP("adm","Front center (dB):"),-30,+30);
    diaElemFloat  eChGainRLValue(chGainDB+ADM_CH_REAR_LEFT,QT_TRANSLATE_NOOP("adm","Rear left (dB):"),-30,+30);
    diaElemFloat  eChGainRRValue(chGainDB+ADM_CH_REAR_RIGHT,QT_TRANSLATE_NOOP("adm","Rear right (dB):"),-30,+30);
    diaElemFloat  eChGainRCValue(chGainDB+ADM_CH_REAR_CENTER,QT_TRANSLATE_NOOP("adm","Rear center (dB):"),-30,+30);
    diaElemFloat  eChGainSLValue(chGainDB+ADM_CH_SIDE_LEFT,QT_TRANSLATE_NOOP("adm","Side left (dB):"),-30,+30);
    diaElemFloat  eChGainSRValue(chGainDB+ADM_CH_SIDE_RIGHT,QT_TRANSLATE_NOOP("adm","Side right (dB):"),-30,+30);
    diaElemFloat  eChGainLFEValue(chGainDB+ADM_CH_LFE,QT_TRANSLATE_NOOP("adm","Low-frequency effects (LFE) (dB):"),-30,+30);

    diaElemReadOnlyText noteChGain(NULL,QT_TRANSLATE_NOOP("adm","Note:\n - it is highly recommended to enable normalization when positive gain values are used"),NULL);

    diaElem *chanGainsElems[]={&eChGainFLValue, &eChGainFRValue, &eChGainFCValue, &eChGainSLValue, &eChGainSRValue, &eChGainRLValue, &eChGainRRValue, &eChGainRCValue, &eChGainLFEValue, &noteChGain};
    diaElemTabs tabChanGains(QT_TRANSLATE_NOOP("adm","Channel gains"),NB_ELEM(chanGainsElems),chanGainsElems);

    //*** Channel delays tab ******
    diaElemInteger  eChDelayFLValue(chDelayMS+ADM_CH_FRONT_LEFT,QT_TRANSLATE_NOOP("adm","Front left (ms):"),0,10000);
    diaElemInteger  eChDelayFRValue(chDelayMS+ADM_CH_FRONT_RIGHT,QT_TRANSLATE_NOOP("adm","Front right (ms):"),0,10000);
    diaElemInteger  eChDelayFCValue(chDelayMS+ADM_CH_FRONT_CENTER,QT_TRANSLATE_NOOP("adm","Front center (ms):"),0,10000);
    diaElemInteger  eChDelayRLValue(chDelayMS+ADM_CH_REAR_LEFT,QT_TRANSLATE_NOOP("adm","Rear left (ms):"),0,10000);
    diaElemInteger  eChDelayRRValue(chDelayMS+ADM_CH_REAR_RIGHT,QT_TRANSLATE_NOOP("adm","Rear right (ms):"),0,10000);
    diaElemInteger  eChDelayRCValue(chDelayMS+ADM_CH_REAR_CENTER,QT_TRANSLATE_NOOP("adm","Rear center (ms):"),0,10000);
    diaElemInteger  eChDelaySLValue(chDelayMS+ADM_CH_SIDE_LEFT,QT_TRANSLATE_NOOP("adm","Side left (ms):"),0,10000);
    diaElemInteger  eChDelaySRValue(chDelayMS+ADM_CH_SIDE_RIGHT,QT_TRANSLATE_NOOP("adm","Side right (ms):"),0,10000);
    diaElemInteger  eChDelayLFEValue(chDelayMS+ADM_CH_LFE,QT_TRANSLATE_NOOP("adm","Low-frequency effects (LFE) (ms):"),0,10000);

    diaElemReadOnlyText noteChDelay(NULL,QT_TRANSLATE_NOOP("adm","Note:\n - the final delay will be the sum of a value above and the \"Shift audio\" value provided on the Main tab"),NULL);

    diaElem *chanDelaysElems[]={&eChDelayFLValue, &eChDelayFRValue, &eChDelayFCValue, &eChDelaySLValue, &eChDelaySRValue, &eChDelayRLValue, &eChDelayRRValue, &eChDelayRCValue, &eChDelayLFEValue, &noteChDelay};
    diaElemTabs tabChanDelays(QT_TRANSLATE_NOOP("adm","Channel delays"),NB_ELEM(chanDelaysElems),chanDelaysElems);


    //*** Channel remap tab ******
    diaElemToggle    eChRemap(&bChRemapEnabled,QT_TRANSLATE_NOOP("adm","Enable Remap"));
    diaMenuEntry menuRemap[]={
        {0,     QT_TRANSLATE_NOOP("adm","Front left"), NULL},
        {1,     QT_TRANSLATE_NOOP("adm","Front right"), NULL},
        {2,     QT_TRANSLATE_NOOP("adm","Front center"), NULL},
        {3,     QT_TRANSLATE_NOOP("adm","Side left"), NULL},
        {4,     QT_TRANSLATE_NOOP("adm","Side right"), NULL},
        {5,     QT_TRANSLATE_NOOP("adm","Rear left"), NULL},
        {6,     QT_TRANSLATE_NOOP("adm","Rear right"), NULL},
        {7,     QT_TRANSLATE_NOOP("adm","Rear center"), NULL},
        {8,     QT_TRANSLATE_NOOP("adm","Low-frequency effects (LFE)"), NULL}
    };
    diaElemMenu   eRemapFL(vChRemap+0,QT_TRANSLATE_NOOP("adm","Front left to:"),NB_ITEMS(menuRemap),menuRemap);  
    diaElemMenu   eRemapFR(vChRemap+1,QT_TRANSLATE_NOOP("adm","Front right to:"),NB_ITEMS(menuRemap),menuRemap);  
    diaElemMenu   eRemapFC(vChRemap+2,QT_TRANSLATE_NOOP("adm","Front center to:"),NB_ITEMS(menuRemap),menuRemap);  
    diaElemMenu   eRemapSL(vChRemap+3,QT_TRANSLATE_NOOP("adm","Side left to:"),NB_ITEMS(menuRemap),menuRemap);  
    diaElemMenu   eRemapSR(vChRemap+4,QT_TRANSLATE_NOOP("adm","Side right to:"),NB_ITEMS(menuRemap),menuRemap);  
    diaElemMenu   eRemapRL(vChRemap+5,QT_TRANSLATE_NOOP("adm","Rear left to:"),NB_ITEMS(menuRemap),menuRemap);  
    diaElemMenu   eRemapRR(vChRemap+6,QT_TRANSLATE_NOOP("adm","Rear right to:"),NB_ITEMS(menuRemap),menuRemap);  
    diaElemMenu   eRemapRC(vChRemap+7,QT_TRANSLATE_NOOP("adm","Rear center to:"),NB_ITEMS(menuRemap),menuRemap);  
    diaElemMenu   eRemapLFE(vChRemap+8,QT_TRANSLATE_NOOP("adm","Low-frequency effects (LFE) to:"),NB_ITEMS(menuRemap),menuRemap); 
    eChRemap.link(1, &eRemapFL);
    eChRemap.link(1, &eRemapFR);
    eChRemap.link(1, &eRemapFC);
    eChRemap.link(1, &eRemapSL);
    eChRemap.link(1, &eRemapSR);
    eChRemap.link(1, &eRemapRL);
    eChRemap.link(1, &eRemapRR);
    eChRemap.link(1, &eRemapRC);
    eChRemap.link(1, &eRemapLFE);
    
    diaElemReadOnlyText noteRemap(NULL,QT_TRANSLATE_NOOP("adm","Note:"
                                                               "\n - remap will not change the channel layout, therefore:"
                                                               "\n - mapping a channel to a non-existent will result loss"
                                                               "\n - mapping a non-existent channel will result silence"
                                                                  ),NULL);

    diaElem *remapElems[]={&eChRemap, &eRemapFL, &eRemapFR, &eRemapFC, &eRemapSL, &eRemapSR, &eRemapRL, &eRemapRR, &eRemapRC, &eRemapLFE, &noteRemap};
    diaElemTabs tabRemap(QT_TRANSLATE_NOOP("adm","Channel remap"),NB_ELEM(remapElems),remapElems);

    //*** ALL TABS *************************
    diaElemTabs *tabs[] = {
        &tabMain,
        &tabDRC,
        &tabEq,
        &tabChanGains,
        &tabChanDelays,
        &tabRemap
    };

    //**************************************
    #undef NB_ELEM
    #define NB_ELEM(x) sizeof(x)/sizeof(diaElemTabs *)
    if( diaFactoryRunTabs(QT_TRANSLATE_NOOP("adm","Audio Filters"),NB_ELEM(tabs),tabs))
    {
        config->mixerConf=(CHANNEL_CONF)vChan;
        config->film2pal=(FILMCONV)vFilm;
        config->gainParam.mode=(ADM_GAINMode)vGainMode;
        config->gainParam.gain10=vGainValue*10;
        config->gainParam.maxlevel10=vGainMaxLevel*10;
        config->mixerEnabled=bMixer;
        config->shiftInMs=vShift;
        config->shiftEnabled=bShiftEnabled;

        config->drcConf.mUseGain = (drcUseGain ? 1:0);
        config->drcConf.mFloor = pow(10.0, drcFloorDB/10.0);     // convert to linear; factor of 10 because it's power
        config->drcConf.mAttackTime = drcAttackTime;
        config->drcConf.mDecayTime = drcDecayTime;
        config->drcConf.mRatio = drcRatio;
        config->drcConf.mThresholdDB = drcThresholdDB;

        for (int i=0; i<ADM_CH_LAST; i++)
        {
            if (i < ADM_CH_FRONT_LEFT) continue;
            if (i > ADM_CH_LFE) continue;
            config->chansConf.chGainDB[i] = chGainDB[i];
            config->chansConf.chDelayMS[i] = chDelayMS[i];
        }

        config->chansConf.enableRemap = bChRemapEnabled;
        for (int i=0; i<9; i++)
            config->chansConf.remap[i] = vChRemap[i];

        config->eqConf.enable = vEqEnable;
        config->eqConf.lowDB = eqLowDB;
        config->eqConf.midDB = eqMidDB;
        config->eqConf.highDB = eqHighDB;        
        config->eqConf.cutOffLM = eqCutLM;
        config->eqConf.cutOffMH = eqCutMH;        
        return true;
    }

    return false;
}



// EOF
