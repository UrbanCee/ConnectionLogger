#include "errorhandler.h"

#include <QTimer>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QSpinBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QtDebug>

PingResult::PingResult()
    :time(QDateTime::currentDateTime()),warnings(NO_WARNING),errors(NO_ERROR),ping(-1),output()
{}

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
            ping=static_cast<short> (match.captured("time").toDouble());
        }
    }
}

bool PingResult::hasErrors() const
{
    return warnings!=PingResult::NO_WARNING || errors!=PingResult::NO_ERROR;
}

bool PingResult::containsWarning(unsigned short warning) const
{
    return (warning & warnings)==warning;
}

bool PingResult::containsError(unsigned short error) const
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

QString PingResult::errorString(unsigned short errors)
{
    QString errorString;
    if (containsError(errors,PING_OTHER_ERROR))
        errorString.append("Ping other error! ");
    if (containsError(errors,PING_NO_REPLY_ERROR))
        errorString.append("Ping no reply! ");
    else if (containsError(errors,EMPTY_OUTPUT_ERROR))
        errorString.append("Ping result empty! ");
    else if (containsError(errors,NO_TIME_ERROR))
        errorString.append("No ping time! ");
    if (containsError(errors,UNKNOWN_ERROR))
        errorString.append("Unknown Error! ");
    return errorString;
}

QString PingResult::warningString(unsigned short warnings)
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

bool PingResult::containsWarning(unsigned short warnings, PingResult::Warnings warning)
{
    return (warning & warnings)==warning;
}

bool PingResult::containsError(unsigned short errors, PingResult::Errors error)
{
    return (error & errors)==error;
}


PingEvent::PingEvent() : startIndex(-1), endIndex(-1)
{}



PingLog::PingLog(QLabel *labelStatus, QTextEdit *textEditLog, QTextEdit *textEditPing, QSpinBox *spinBoxTimeToNormal, QWidget *parent)
    :QObject(parent),labelStatus(labelStatus),textEditLog(textEditLog),textEditPing(textEditPing),spinBoxTimeToNormal(spinBoxTimeToNormal),iCurrentID(0)
{
    pingEvents.append(PingEvent());
}

void PingLog::update(const PingResult &currentPing)
{
    ++iCurrentID;
    if (currentPing.hasErrors()){
        problemPings.insert(iCurrentID,currentPing);
        labelStatus->setText(QString("Problem: %1%2").arg(currentPing.ping>=0?QString("(Ping %1ms) ").arg(currentPing.ping):"").arg(currentPing.warningString()+currentPing.errorString()));
        labelStatus->setStyleSheet("QLabel {color  : rgb(180,0,0);}");
        textEditPing->append(QString("<b>%1</b><br>%2%3<br><br><small>%4</small>")
                             .arg(QDateTime::currentDateTime().toString())
                             .arg(currentPing.warningString())
                             .arg(currentPing.errorString())
                             .arg(QString(currentPing.output).replace("\n","<br>")));
        if (pingEvents.last().startIndex==-1)
        {
            pingEvents.last().startIndex=iCurrentID;
        }
    }else{
        cleanPings.insert(iCurrentID,currentPing.ping);
        if (!problemPings.isEmpty() && problemPings.last().time.msecsTo(QDateTime::currentDateTime()) < spinBoxTimeToNormal->value()*1000){
                labelStatus->setText(QString("Last Ping: %1 ms (%2 pings, recovering from <i>%3</i>)")
                                     .arg(currentPing.ping)
                                     .arg(iCurrentID-problemPings.lastKey())
                                     .arg(problemPings.last().warningString()+problemPings.last().errorString()));
                labelStatus->setStyleSheet("QLabel {color  : rgb(180,180,0);}");
            }else{
                    labelStatus->setText(QString("Last Ping: %1 ms").arg(currentPing.ping));
                    labelStatus->setStyleSheet("QLabel {color  : rgb(0,180,0);}");
                    if ( (pingEvents.last().startIndex!= -1) )
                    {
                        PingEvent &event = pingEvents.last();
                        event.endIndex=iCurrentID;
                        int iCleanPings = 0;
                        int iProblemPings = 0;
                        unsigned short warnings=PingResult::NO_WARNING;
                        unsigned short errors=PingResult::NO_ERROR;
                        for (int i=event.startIndex;i<=event.endIndex;i++)
                        {
                            if (problemPings.contains(i)){
                                ++iProblemPings;
                                warnings|=problemPings.value(i).warnings;
                                errors|=problemPings.value(i).errors;
                            }else {
                                ++iCleanPings;
                            }

                        }
                        QDateTime startTime=problemPings.value(event.startIndex).time;
                        textEditLog->append(QString("<b>%1</b> for %2s: (%3%4)<br>%5 clean pings; %6 problem pings;<br>%7")
                                            .arg(startTime.toString())
                                            .arg(startTime.secsTo(currentPing.time))
                                            .arg(PingResult::warningString(warnings))
                                            .arg(PingResult::errorString(errors))
                                            .arg(iCleanPings)
                                            .arg(iProblemPings)
                                            .arg(visualize()));
                        pingEvents.append(PingEvent());
                    }
            }
    }
    if (iCurrentID%100==0)
        additionalTimeStamps.insert(iCurrentID,QDateTime::currentDateTime());
}

QString PingLog::visualize()
{
    QString pingVis;
    PingEvent &event = pingEvents.last();
    auto getVisChar = [this](int iIndex){
        if (problemPings.contains(iIndex))
        {
            const PingResult &problemPing = problemPings.value(iIndex);
            if (problemPing.errors==PingResult::NO_ERROR)
                return "W";
            else if (problemPing.warnings==PingResult::NO_WARNING)
                return "E";
            return "X";
        }else{
            return ".";
        }
    };
    int iEventLength=event.endIndex-event.startIndex+1;
    int iLineLength=200;
    if (iEventLength<iLineLength){
        for (int i=0;i<iEventLength;++i)
        {
            pingVis.append(getVisChar(event.startIndex+i));
        }
    }else {
        //TODO
    }
    return pingVis;
}


