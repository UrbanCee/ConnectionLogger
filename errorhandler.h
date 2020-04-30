#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QDateTime>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QTextEdit)

struct PingResult
{
    enum Errors{NO_ERROR = 0 , PING_RETURN_ERROR = 1<<0, PING_NO_REPLY_ERROR = 1<<1, NO_TIME_ERROR = 1<<2, EMPTY_OUTPUT_ERROR = 1 << 3, UNKNOWN_ERROR = 1<<16};
    enum Warnings{NO_WARNING = 0, TIME_MISMATCH_WARNING = 1<<0, HIGH_PING_WARNING = 1<<2, RESPONSE_TIME_TOO_HIGH_WARNING = 1<<3, UNKNOWN_WARNING= 1<<16};
    PingResult(QString output);
    QDateTime time;
    int warnings;
    int errors;
    int ping;
    QString output;
    bool hasErrors() const;
    void addWarning(int warning){warnings|=warning;}
    void addError(int error){errors|=error;}
    bool containsWarning(int warning) const;
    bool containsError(int error) const;
    QString errorString() const;
    QString warningString() const;

    static QString errorString(int errors);
    static QString warningString(int warnings);
    static bool containsWarning(int warnings,Warnings warning);
    static bool containsError(int errors,Errors error);
};



class Error{
public:
    Error(const PingResult &);
    void addPing(const PingResult &);
    void finalize();

    int errors;
    int warnings;
    int goodPings;
    QDateTime start;
    QDateTime end;
    QList<PingResult> pings;

    QString warningErrorString();
};


class ErrorHandler : public QObject
{
    Q_OBJECT
public:
    explicit ErrorHandler(QLabel *labelStatus,QTextEdit *textEditLog,QWidget *parent = nullptr);
    void update(PingResult currentPing);
signals:
public slots:
    void changeTimeout(int iNewTimeToNormal);
private slots:
    void returnToNormal();
private:
    QWidget *parentWidget;
    QLabel *labelStatus;
    QTextEdit *textEditLog;
    QTimer *timer;
    Error *currErr;
    QList<Error> errors;

};

#endif // ERRORHANDLER_H
