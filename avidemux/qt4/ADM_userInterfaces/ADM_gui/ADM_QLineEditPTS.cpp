/***************************************************************************
    \file  ADM_QLineEditPTS
    \brief Custom QLineEdit to store the time pointer (precision timing)
    \author Matthew White <mehw.is.me@inventati.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include "ADM_misc.h"
#include "ADM_QLineEditPTS.h"

#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
#include <QRegExp>
#include <QRegExpValidator>
#else
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionValidator>
#endif

ADM_CORE6_EXPORT uint8_t ms2time(uint32_t ms, uint32_t *h, uint32_t *m, uint32_t *s, uint32_t *mms);

static
#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
    QRegExp
#else
    QRegularExpression
#endif
    timeRegExp("^([0-9]{2}):([0-5][0-9]):([0-5][0-9])\\.([0-9]{3})$");

static
#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
    QRegExp
#else
    QRegularExpression
#endif
    PTSRegExp("^(.*)([0-9]{2}):([0-5][0-9]):([0-5][0-9])\\.([0-9]{3})(,([0-9]{3}))?$");

/**
 * \brief Convert a hh:mm:ss.ms,us string to integer values (",us" is optional).
 * \param[in] text string in 00:00:00.000,000 format (",000" is optional).
 * \param[out] hh set to hours.
 * \param[out] mm set to minutes.
 * \param[out] ss set to seconds.
 * \param[out] ms set to milliseconds.
 * \param[out] us set to microseconds.
 * \return true if the string has the right format, otherwise false.
 */
bool text2time(const char *text, uint32_t *hh, uint32_t *mm, uint32_t *ss, uint32_t *ms, uint32_t *us)
{
#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
    QRegExp
#else
    QRegularExpression
#endif
    rx(PTSRegExp);

#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
    if(rx.exactMatch(text))
#else
    QRegularExpressionMatch match = rx.match(text);
    if(match.hasMatch())
#endif
    {
        QStringList results =
#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
        rx.capturedTexts();
#else
        match.capturedTexts();
#endif

        *hh = results.at(2).toInt(NULL, 10);
        *mm = results.at(3).toInt(NULL, 10);
        *ss = results.at(4).toInt(NULL, 10);
        *ms = results.at(5).toInt(NULL, 10);

        if(results.size() > 6)
            *us = results.at(7).toInt(NULL, 10);
        else
            *us = 0;

        return true;
    }

    return false;
}

/**
 * \brief Convert time into PTS.
 * \param[in] hh hours.
 * \param[in] mm minutes.
 * \param[in] ss seconds.
 * \param[in] ms milliseconds.
 * \param[in] us microseconds.
 * \return pts time in microseconds (us).
 */
uint64_t time2pts(const uint32_t *hh, const uint32_t *mm, const uint32_t *ss, const uint32_t *ms, const uint32_t *us)
{
    return (((uint64_t)(*hh)*3600+(*mm)*60+(*ss))*1000+(*ms))*1000+(*us);
}

/**
 * \brief Convert PTS to hh:mm:ss.ms,us string.
 * \param[in] pts time in microseconds (us).
 * \return string in 00:00:00.000,000 format.
 */
QString pts2text(const uint64_t &pts)
{
    char text[80];
    uint32_t hh, mm, ss, ms, us;

    us = pts - (uint64_t)((uint32_t)(pts/1000))*1000;
    ms2time((uint32_t)(pts/1000), &hh, &mm, &ss, &ms);
    snprintf(text, 79, "%02" PRIu32 ":%02" PRIu32 ":%02" PRIu32 ".%03" PRIu32  ",%03" PRIu32, hh, mm, ss, ms, us);

    return text;
}

/**
 * \brief Append/Replace PTS string to/in a text.
 * \param[in,out] text string to modify.
 * \param[in] pts string in 00:00:00.000,000 format (",000" is optional).
 * \return reference to text.
 */
QString &appendPTS(QString &text, const QString &pts)
{
    QString check = pts;

    // Verify the format of the PTS string
    if(!truncatePTS(check).isEmpty())
        return text;

    // Remove any appended PTS string
    if(!truncatePTS(text).isEmpty())
        text.append(" ");

    // Append the PTS string
    return text.append(pts);
}

/**
 * \brief Append/Replace PTS string to/in a text.
 * \param[in,out] text string to modify.
 * \param[in] pts time in microseconds (us).
 * \return reference to text.
 */
QString &appendPTS(QString &text, const uint64_t &pts)
{
    return appendPTS(text, pts2text(pts));
}

/**
 * \brief Remove appended PTS string from a text.
 * \param[in,out] text string to modify.
 * \return reference to text.
 */
QString &truncatePTS(QString &text)
{
#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
    QRegExp
#else
    QRegularExpression
#endif
    rx(PTSRegExp);

#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
    if(rx.exactMatch(text))
#else
    QRegularExpressionMatch match = rx.match(text);
    if(match.hasMatch())
#endif
    {
        QStringList results =
#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
        rx.capturedTexts();
#else
        match.capturedTexts();
#endif
        text = results.at(1).trimmed();
    }

    return text;
}

/**
 * \brief Initialize PTS to the given value.
 * \param[in] pts time in microseconds (us).
 * \param[in] parent sent to the QWidget constructor.
 * \sa ADM_QLineEditPTS::setPTS(const uint64_t&)
 */
ADM_QLineEditPTS::ADM_QLineEditPTS(const uint64_t &pts, QWidget *parent /* = nullptr */) : QLineEdit(parent)
{
#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
    QRegExpValidator *timeValidator = new QRegExpValidator(timeRegExp, this);
#else
    QRegularExpressionValidator *timeValidator = new QRegularExpressionValidator(timeRegExp, this);
#endif
    setInputMask("99:99:99.999");
    setValidator(timeValidator);
    setPTS(pts);
}

/**
 * \brief Initialize PTS to 0 microseconds (us).
 * \param[in] parent sent to the QWidget constructor.
 * \sa ADM_QLineEditPTS::setPTS(const uint64_t&)
 */
ADM_QLineEditPTS::ADM_QLineEditPTS(QWidget *parent /* = nullptr */) : ADM_QLineEditPTS(0, parent)
{
}

ADM_QLineEditPTS::~ADM_QLineEditPTS()
{
}

/**
 * \brief Internally used to set text().
 * \param[in] text string to use as text().
 * \sa ADM_QLineEditPTS::setPTS(const uint64_t&)
 */
void ADM_QLineEditPTS::setText(const QString &text)
{
    QLineEdit::setText(text);
}

/**
 * \brief Internally used to set a hardcoded value.
 * \param[in] validator content validator, pass 0 to disable.
 * \sa ADM_QLineEditPTS::setInputMask(const QString&)
 */
void ADM_QLineEditPTS::setValidator(const QValidator *validator)
{
    QLineEdit::setValidator(validator);
}

/**
 * \brief Internally used to set a hardcoded value.
 * \param[in] mask input mask, pass an empty string to disable.
 * \sa ADM_QLineEditPTS::setValidator(const QValidator*)
 */
void ADM_QLineEditPTS::setInputMask(const QString &mask)
{
    QLineEdit::setInputMask(mask);
}

/**
 * \brief Set PTS to 0 microseconds (us).
 * \sa ADM_QLineEditPTS::resetEdit(void)
 */
void ADM_QLineEditPTS::clear(void)
{
    setPTS(0);
}

/**
 * \brief Reset to the original PTS.
 * \sa ADM_QLineEditPTS::isTextEdited(void)
 */
void ADM_QLineEditPTS::resetEdit(void)
{
    setText(_uneditedText);
}

/**
 * \brief Set PTS to the given value.
 * \param[in] pts time in microseconds (us).
 * \sa ADM_QLineEditPTS::PTS(void)
 * \sa ADM_QLineEditPTS::uneditedPTS(void)
 */
void ADM_QLineEditPTS::setPTS(const uint64_t &pts)
{
    _pts = pts;
    // Remove microseconds from conversion to text
    setText(pts2text(_pts).chopped(4));
    _uneditedText = text();
}

/**
 * \brief Return the current PTS.
 * \return time in microseconds (us).
 * \sa ADM_QLineEditPTS::isTextEdited(void)
 */
uint64_t ADM_QLineEditPTS::PTS(void)
{
    uint32_t hh, mm, ss, ms, us;
    return PTS(&hh, &mm, &ss, &ms, &us);
}

/**
 * \brief Return the current PTS.
 * \param[out] hh set to hours.
 * \param[out] mm set to minutes.
 * \param[out] ss set to seconds.
 * \param[out] ms set to milliseconds.
 * \param[out] us set to microseconds.
 * \return time in microseconds (us).
 * \sa ADM_QLineEditPTS::isTextEdited(void)
 */
uint64_t ADM_QLineEditPTS::PTS(uint32_t *hh, uint32_t *mm, uint32_t *ss, uint32_t *ms, uint32_t *us)
{
    // Fail if text() isn't in the right format
    assert(text2time(text().toUtf8().constData(), hh, mm, ss, ms, us));
    // Compute microseconds missing from the text
    *us = _pts - (uint64_t)((uint32_t)(_pts/1000))*1000;
    return time2pts(hh, mm, ss, ms, us);
}

/**
 * \brief Return the original PTS.
 * \return time in microseconds (us).
 * \sa ADM_QLineEditPTS::isTextEdited(void)
 */
uint64_t ADM_QLineEditPTS::uneditedPTS(void)
{
    return _pts;
}

/**
 * \brief Return the original PTS.
 * \param[out] hh set to hours.
 * \param[out] mm set to minutes.
 * \param[out] ss set to seconds.
 * \param[out] ms set to milliseconds.
 * \param[out] us set to microseconds.
 * \return time in microseconds (us).
 * \sa ADM_QLineEditPTS::isTextEdited(void)
 */
uint64_t ADM_QLineEditPTS::uneditedPTS(uint32_t *hh, uint32_t *mm, uint32_t *ss, uint32_t *ms, uint32_t *us)
{
    // Compute microseconds missing from the text
    *us = _pts - (uint64_t)((uint32_t)(_pts/1000))*1000;
    ms2time((uint32_t)(_pts/1000), hh, mm, ss, ms);
    return _pts;
}

/**
 * \brief Return the current PTS as hh:mm:ss.ms string.
 * \return string in 00:00:00.000 format.
 * \sa ADM_QLineEditPTS::PTS(void)
 */
QString ADM_QLineEditPTS::text(void)
{
    return QLineEdit::text();
}

/**
 * \brief Return the original PTS as hh:mm:ss.ms string.
 * \return string in 00:00:00.000 format.
 * \sa ADM_QLineEditPTS::uneditedPTS(void)
 */
QString ADM_QLineEditPTS::uneditedText(void)
{
    return _uneditedText;
}

/**
 * \brief Return true if the PTS has been edited by the user (via GUI).
 * \return true if text() has been modified, otherwise false.
 * \sa ADM_QLineEditPTS::resetEdit(void)
 */
bool ADM_QLineEditPTS::isTextEdited(void)
{
    return (text() != _uneditedText);
}

/**
 * \brief Append/Replace PTS string to/in tooltip.
 * \sa ADM_QLineEditPTS::setPTS(const uint64_t&)
 */
void ADM_QLineEditPTS::appendPTSToolTip(void)
{
    QString tt = toolTip();
    // Append whole PTS to tooltip
    setToolTip(appendPTS(tt, _pts));
}

/**
 * \brief Remove appended PTS string from tooltip.
 * \sa ADM_QLineEditPTS::setPTS(const uint64_t&)
 */
void ADM_QLineEditPTS::removePTSToolTip(void)
{
    QString tt = toolTip();
    // Remove any appended PTS from tooltip
    setToolTip(truncatePTS(tt));
}
