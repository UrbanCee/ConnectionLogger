#include "errorhandler.h"

#include <QTimer>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QtDebug>

PingResult::PingResult(QString output)
    :time(QDateTime::currentDateTime()),warnings(NO_WARNING),errors(NO_ERROR),ping(-1),output(output)
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
            ping=static_cast<int> (match.captured("time").toDouble());
        }
    }

}

bool PingResult::hasErrors() const
{
    return warnings!=PingResult::NO_WARNING || errors!=PingResult::NO_ERROR;
}

bool PingResult::containsWarning(int warning) const
{
    return (warning & warnings)==warning;
}

bool PingResult::containsError(int error) const
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


Error::Error(const PingResult &result)
    :goodPings(0)
{
    start=QDateTime::currentDateTime();
    addPing(result);
}

void Error::addPing(const PingResult &result)
{
    if (result.hasErrors())
        goodPings=0;
    else
        ++goodPings;
    warnings|=result.warnings;
    errors|=result.errors;
    pings.append(result);
}

void Error::finalize()
{
    end = QDateTime::currentDateTime();
}

QString Error::warningErrorString()
{
    return PingResult::errorString(errors) + PingResult::warningString(warnings);
}


ErrorHandler::ErrorHandler(QLabel *labelStatus, QTextEdit *textEditLog, QWidget *parent)
    : QObject(parent), parentWidget(parent),labelStatus(labelStatus),textEditLog(textEditLog),timer(new QTimer(this)), currErr(nullptr)
{
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this,SLOT(returnToNormal()));
}

void ErrorHandler::update(PingResult currentPing)
{
    if (currErr==nullptr)
    {
        if (!currentPing.hasErrors()){
            labelStatus->setText(QString("Last Ping: %1 ms").arg(currentPing.ping));
            labelStatus->setStyleSheet("QLabel {color  : rgb(0,180,0);}");
            return;
        }
        errors.append(Error(currentPing));
        currErr=&errors.last();
    }else{
        currErr->addPing(currentPing);
    }
    if (currentPing.hasErrors()){
        timer->start();
        labelStatus->setText(QString("Problem: %1%2").arg(currentPing.ping>=0?QString("(Ping %1ms) ").arg(currentPing.ping):"").arg(currentPing.warningString()+currentPing.errorString()));
        labelStatus->setStyleSheet("QLabel {color  : rgb(180,0,0);}");
    }else {
        labelStatus->setText(QString("Last Ping: %1 ms (%2 pings, recovering from(%3)")
                             .arg(currentPing.ping)
                             .arg(currErr->goodPings)
                             .arg(currErr->warningErrorString()));
        labelStatus->setStyleSheet("QLabel {color  : rgb(180,180,0);}");
    }
}

void ErrorHandler::changeTimeout(int iNewTimeToNormal)
{
    timer->setInterval(iNewTimeToNormal*1000);
}

void ErrorHandler::returnToNormal()
{
    if (currErr==nullptr){
        qDebug() << __FILE__ << __LINE__ << "unexpected call of returnToNormal()";
        return;
    }
    timer->stop();
    currErr->finalize();
    currErr=nullptr;
}


