#ifndef ADM_QLineEditPTS_h
#define ADM_QLineEditPTS_h

#include <QLineEdit>

bool text2time(const char *text, uint32_t *hh, uint32_t *mm, uint32_t *ss, uint32_t *ms, uint32_t *us);
uint64_t time2pts(const uint32_t *hh, const uint32_t *mm, const uint32_t *ss, const uint32_t *ms, const uint32_t *us);
QString pts2text(const uint64_t &pts);
QString &appendPTS(QString &text, const uint64_t &pts);
QString &appendPTS(QString &text, const QString &pts);
QString &truncatePTS(QString &text);

/**
 * \brief Add properties and methods to give timing precision.
 */
class ADM_QLineEditPTS : public QLineEdit
{
    Q_OBJECT

public:
    ADM_QLineEditPTS(const uint64_t &pts, QWidget *parent = nullptr);
    ADM_QLineEditPTS(QWidget *parent = nullptr);
    virtual ~ADM_QLineEditPTS();

    void clear(void);
    void resetEdit(void);
    void setPTS(const uint64_t &pts);
    uint64_t PTS();
    uint64_t PTS(uint32_t *hh, uint32_t *mm, uint32_t *ss, uint32_t *ms, uint32_t *us);
    uint64_t uneditedPTS(void);
    uint64_t uneditedPTS(uint32_t *hh, uint32_t *mm, uint32_t *ss, uint32_t *ms, uint32_t *us);
    QString text(void);
    QString uneditedText(void);
    bool isTextEdited(void);
    void appendPTSToolTip(void);
    void removePTSToolTip(void);

private:
    uint64_t _pts;
    QString _uneditedText;

    // Do not set the text directly, use setPTS(), unless setText() is
    // designed to hold the us too (i.e. not cutting the real time).
    void setText(const QString &text);

    // These use hardcoded values, don't move them from here.
    void setValidator(const QValidator *validator);
    void setInputMask(const QString &mask);
};

#endif // ADM_QLineEditPTS_h
