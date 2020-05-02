#include "errorhandler.h"

#include <QTimer>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QSpinBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QtDebug>

PingResult::PingResult(QString output)
    :time(QDateTime::currentDateTime()),warnings(NO_WARNING),errors(NO_ERROR),ping(-1),output()
{
    if (output.isEmpty())
        addError(EMPTY_OUTPUT_ERROR);
    else{
        QRegularExpression re("time=(?<time>\\d+\\.*\\d*)\\sms");
        QRegularExpressionMatch match = re.match(output);
        if (!match.hasMatch())
        {
            addError(NO_TIME_ERROR);
        }else {
            ping=static_cast<short> (match.captured("time").toDouble());
        }
    }
    if (ping<=0)
        this->output=output;
}

bool PingResult::hasErrors() const
{
    return warnings!=PingResult::NO_WARNING || errors!=PingResult::NO_ERROR;
}

bool PingResult::containsWarning(unsigned char warning) const
{
    return (warning & warnings)==warning;
}

bool PingResult::containsError(unsigned char error) const
{
    return (error & errors)==error;
}

QString PingResult::errorString() const
{
    return errorString(this->errors);
}

QString PingResult::warningString() const
{
    QString warningString=PingResult::warningString(warnings);
    if (containsWarning(HIGH_PING_WARNING)){
        warningString.remove("High ping! ");
        warningString.append(QString( "High ping! (%1 ms)").arg(ping));
    }
    return warningString;
}

QString PingResult::errorString(int errors)
{
    QString errorString;
    if (containsError(errors,PING_RETURN_ERROR))
        errorString.append("Ping abnormal execution! ");
    if (containsError(errors,PING_NO_REPLY_ERROR))
        errorString.append("Ping return error! ");
    if (containsError(errors,NO_TIME_ERROR))
        errorString.append("No ping time! ");
    if (containsError(errors,EMPTY_OUTPUT_ERROR))
        errorString.append("Ping result empty! ");
    if (containsError(errors,UNKNOWN_ERROR))
        errorString.append("Unknown Error! ");
    return errorString;
}

QString PingResult::warningString(int warnings)
{
    QString warningString;
    if (containsWarning(warnings,TIME_MISMATCH_WARNING))
        warningString.append("Time mismatch warning! ");
    if (containsWarning(warnings,HIGH_PING_WARNING))
        warningString.append(QString( "High ping! "));
    if (containsWarning(warnings,RESPONSE_TIME_TOO_HIGH_WARNING))
        warningString.append("High response time! ");
    if (containsWarning(warnings,UNKNOWN_WARNING))
        warningString.append("Unknown Warning! ");
    return warningString;
}

bool PingResult::containsWarning(int warnings, PingResult::Warnings warning)
{
    return (warning & warnings)==warning;
}

bool PingResult::containsError(int errors, PingResult::Errors error)
{
    return (error & errors)==error;
}

PingLog::PingLog(QLabel *labelStatus, QTextEdit *textEditLog, QSpinBox *spinBoxTimeToNormal, QWidget *parent)
    :QObject(parent),labelStatus(labelStatus),textEditLog(textEditLog),spinBoxTimeToNormal(spinBoxTimeToNormal),iCurrentID(0)
{}

void PingLog::update(const PingResult &currentPing)
{
    ++iCurrentID;
    if (currentPing.hasErrors()){
        problemPings.insert(iCurrentID,currentPing);
    }else{
        cleanPings.insert(iCurrentID,currentPing.ping);
    }
    if (iCurrentID%100==0)
        additionalTimeStamps.insert(iCurrentID,QDateTime::currentDateTime());
    if (currentPing.hasErrors()){
        labelStatus->setText(QString("Problem: %1%2").arg(currentPing.ping>=0?QString("(Ping %1ms) ").arg(currentPing.ping):"").arg(currentPing.warningString()+currentPing.errorString()));
        labelStatus->setStyleSheet("QLabel {color  : rgb(180,0,0);}");
    } else if (!problemPings.isEmpty() && problemPings.last().time.msecsTo(QDateTime::currentDateTime()) < spinBoxTimeToNormal->value()*1000){
        labelStatus->setText(QString("Last Ping: %1 ms (%2 pings, recovering from(%3)")
                             .arg(currentPing.ping)
                             .arg(iCurrentID-problemPings.lastKey())
                             .arg(problemPings.last().warningString()+problemPings.last().errorString()));
        labelStatus->setStyleSheet("QLabel {color  : rgb(180,180,0);}");
    }else{
            labelStatus->setText(QString("Last Ping: %1 ms").arg(currentPing.ping));
            labelStatus->setStyleSheet("QLabel {color  : rgb(0,180,0);}");
    }
}

