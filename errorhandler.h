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
    enum Errors{NO_ERROR = 0 , PING_RETURN_ERROR = 1<<0, PING_NO_REPLY_ERROR = 1<<1, NO_TIME_ERROR = 1<<2, EMPTY_OUTPUT_ERROR = 1 << 3, UNKNOWN_ERROR = 1<<7};
    enum Warnings{NO_WARNING = 0, TIME_MISMATCH_WARNING = 1<<0, HIGH_PING_WARNING = 1<<2, RESPONSE_TIME_TOO_HIGH_WARNING = 1<<3, UNKNOWN_WARNING= 1<<7};
    PingResult(QString output);
    QDateTime time;
    unsigned char warnings;
    unsigned char errors;
    short ping;
    QString output;
    bool hasErrors() const;
    void addWarning(unsigned char warning){warnings|=warning;}
    void addError(unsigned char error){errors|=error;}
    bool containsWarning(unsigned char warning) const;
    bool containsError(unsigned char error) const;
    QString errorString() const;
    QString warningString() const;

    static QString errorString(int errors);
    static QString warningString(int warnings);
    static bool containsWarning(int warnings,Warnings warning);
    static bool containsError(int errors,Errors error);
};




class PingLog : public QObject
{
    Q_OBJECT
public:
    explicit PingLog(QLabel *labelStatus,QTextEdit *textEditLog,QSpinBox *spinBoxTimeToNormal, QWidget *parent = nullptr);
    void update(const PingResult &currentPing);
private:
    QWidget *parentWidget;
    QLabel *labelStatus;
    QTextEdit *textEditLog;
    QSpinBox *spinBoxTimeToNormal;
    QMap<int,short> cleanPings;
    QMap<int,PingResult> problemPings;
    QMap<int,QDateTime> additionalTimeStamps;
    int iCurrentID;
};

#endif // ERRORHANDLER_H
