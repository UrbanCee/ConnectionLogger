#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QDateTime>
#include <QObject>
#include <QMap>

QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QTextEdit)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

struct PingResult
{
    enum Errors{NO_ERROR = 0 , PING_RETURN_ERROR = 1<<0, PING_NO_REPLY_ERROR = 1<<1, NO_TIME_ERROR = 1<<2, EMPTY_OUTPUT_ERROR = 1 << 3, UNKNOWN_ERROR = 1<<15};
    enum Warnings{NO_WARNING = 0, TIME_MISMATCH_WARNING = 1<<0, HIGH_PING_WARNING = 1<<2, RESPONSE_TIME_TOO_HIGH_WARNING = 1<<3, UNKNOWN_WARNING= 1<<15};
    PingResult();
    PingResult(QString output);
    QDateTime time;
    unsigned short warnings;
    unsigned short errors;
    short ping;
    QString output;
    bool hasErrors() const;
    void addWarning(unsigned short warning){warnings|=warning;}
    void addError(unsigned short error){errors|=error;}
    bool containsWarning(unsigned short warning) const;
    bool containsError(unsigned short error) const;
    QString errorString() const;
    QString warningString() const;

    static QString errorString(unsigned short errors);
    static QString warningString(unsigned short warnings);
    static bool containsWarning(unsigned short warnings, Warnings warning);
    static bool containsError(unsigned short errors,Errors error);
};

struct PingEvent
{
    PingEvent();
    int startIndex;
    int endIndex;
};


class PingLog : public QObject
{
    Q_OBJECT
public:
    explicit PingLog(QLabel *labelStatus,QTextEdit *textEditLog,QTextEdit *textEditPing, QSpinBox *spinBoxTimeToNormal, QWidget *parent = nullptr);
    void update(const PingResult &currentPing);
private:
    QWidget *parentWidget;
    QLabel *labelStatus;
    QTextEdit *textEditLog;
    QTextEdit *textEditPing;
    QSpinBox *spinBoxTimeToNormal;
    QMap<int,short> cleanPings;
    QMap<int,PingResult> problemPings;
    QMap<int,QDateTime> additionalTimeStamps;
    QList<PingEvent> pingEvents;
    int iCurrentID;
};

#endif // ERRORHANDLER_H
