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

typedef unsigned short errorType;

QString sec2String(int s);

struct PingResult
{
    enum Errors{NO_ERROR = 0 , PING_OTHER_ERROR = 1<<0, PING_NO_REPLY_ERROR = 1<<1, NO_TIME_ERROR = 1<<2, EMPTY_OUTPUT_ERROR = 1 << 3, UNKNOWN_ERROR = 1<<15};
    enum Warnings{NO_WARNING = 0, TIME_MISMATCH_WARNING = 1<<0, HIGH_PING_WARNING = 1<<2, RESPONSE_TIME_TOO_HIGH_WARNING = 1<<3, UNKNOWN_WARNING= 1<<15};
    PingResult();
    PingResult(QString output);
    PingResult(short ping);
    PingResult(const PingResult &) = default;
    QDateTime time;
    errorType warnings;
    errorType errors;
    short ping;
    QString output;
    bool hasErrors() const;
    void addWarning(errorType warning){warnings|=warning;}
    void addError(errorType error){errors|=error;}
    bool containsWarning(errorType warning) const;
    bool containsError(errorType error) const;
    QString errorString() const;
    QString warningString() const;

    static QString errorString(errorType errors);
    static QString warningString(errorType warnings);
    static bool containsWarning(errorType warnings, Warnings warning);
    static bool containsError(errorType errors, Errors error);

    PingResult &operator= (const PingResult&) = default;
    PingResult &operator|= (const PingResult &);
};

struct PingEvent
{
    PingEvent();
    PingEvent(int start,int end);
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
    PingResult getPing(int id);
    QString visualize(int iLineLength, const PingEvent &);

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
