/**
    \file ADM_update.h
    \brief Check for update
    \author mean (c) 2016
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once
#include "ADM_update.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QThread>

#if 0
#define ADM_UPDATE_SERVER "http://192.168.0.9/"
#else
#define ADM_UPDATE_SERVER "http://www.avidemux.org/update/"
#endif

// Helper defines to construct URL
#ifdef __MINGW32__
    #ifdef _WIN64
            #define ADM_UPDATE_MACHINE "Win64"
    #else  // _WIN64
            #define ADM_UPDATE_MACHINE "Win32"
    #endif // _WIN64
#else //__MINGW32__
    #ifdef __APPLE__
            #define ADM_UPDATE_MACHINE "OsX"
    #else
        #ifdef  __linux__
            #define ADM_UPDATE_MACHINE "Linux"
        #else
            #define ADM_UPDATE_MACHINE "???"
        #endif
    #endif  // apple

#endif // mingw

#define ADM_UPDATE_TARGET "update_for_" ADM_UPDATE_MACHINE ".html"

/**
 */
class ADMCheckUpdate: public QObject
{
  Q_OBJECT
public:
                    ADMCheckUpdate(ADM_updateComplete *up);
        virtual     ~ADMCheckUpdate();
protected:
        QNetworkAccessManager manager;
        ADM_updateComplete    *_updateCallback;

public slots:
        void execute();
        void downloadFinished(QNetworkReply *reply);
};
