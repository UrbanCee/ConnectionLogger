#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QDateTime>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QWidget)

struct Error{
    int warnings;
    int errors;
    QDateTime start;
    QDateTime end;
    QStringList outputs;
};

class ErrorHandler : public QObject
{
    Q_OBJECT
public:
    explicit ErrorHandler(QWidget *parent = nullptr);

    enum ErrorTypes {NO_ERROR = 0 , PING_NO_RETURN = 1<<0, PING_NO_REPLY = 1<<1, NO_TIME = 1<<2, UNKNOWN = 1<<16};
    enum WarningTypes {NO_WARNING = 0, TIME_MISMATCH = 1<<0, HIGH_PING = 1<<2};

    void update(int error, int warning, int pingTime, QString pingReturn);
signals:
public slots:
    void changeTimeout(int);
private:
    QWidget *parentWidget;
    Error *currErr;
    QList<Error> errors;
};

#endif // ERRORHANDLER_H
