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

PingResult::PingResult(short ping)
    :time(QDateTime::currentDateTime()),warnings(NO_WARNING),errors(NO_ERROR),ping(ping),output()
{}

bool PingResult::hasErrors() const
{
    return warnings!=PingResult::NO_WARNING || errors!=PingResult::NO_ERROR;
}

bool PingResult::containsWarning(errorType warning) const
{
    return (warning & warnings)==warning;
}

bool PingResult::containsError(errorType error) const
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

QString PingResult::errorString(errorType errors)
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

QString PingResult::warningString(errorType warnings)
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

bool PingResult::containsWarning(errorType warnings, PingResult::Warnings warning)
{
    return (warning & warnings)==warning;
}

bool PingResult::containsError(errorType errors, PingResult::Errors error)
{
    return (error & errors)==error;
}

PingResult &PingResult::operator|=(const PingResult &other)
{
    this->ping=-1;
    this->errors|=other.errors;
    this->warnings|=other.warnings;
    this->output.append(other.output);
    return *this;
}


PingEvent::PingEvent() : startIndex(-1), endIndex(-1)
{}

PingEvent::PingEvent(int start, int end) :startIndex(start),endIndex(end)
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
                labelStatus->setText(QString("Last Ping: %1 ms (%2 pings, recovering from <i>%3</i>) %4")
                                     .arg(currentPing.ping)
                                     .arg(iCurrentID-problemPings.lastKey())
                                     .arg(problemPings.last().warningString()+problemPings.last().errorString())
                                     .arg(visualize(40,PingEvent(pingEvents.last().startIndex,iCurrentID))));
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
                        QDateTime endTime;
                        for (int i=event.startIndex;i<=event.endIndex;i++)
                        {
                            if (problemPings.contains(i)){
                                ++iProblemPings;
                                const PingResult &currentEvent=problemPings.value(i);
                                warnings|=currentEvent.warnings;
                                errors|=currentEvent.errors;
                                endTime=currentEvent.time;
                            }else {
                                ++iCleanPings;
                            }

                        }
                        QDateTime startTime=problemPings.value(event.startIndex).time;
                        if (iProblemPings==1)
                            textEditLog->append(QString("<b>%1</b>: ( %2%3)")
                                                .arg(startTime.toString())
                                                .arg(PingResult::warningString(warnings))
                                                .arg(PingResult::errorString(errors))
                                                );
                        else
                            textEditLog->append(QString("<b>%1</b> for %2: ( %3%4)<br>%5 clean pings; %6 problem pings;<br>%7")
                                            .arg(startTime.toString())
                                            .arg(sec2String(startTime.secsTo(endTime)))
                                            .arg(PingResult::warningString(warnings))
                                            .arg(PingResult::errorString(errors))
                                            .arg(iCleanPings)
                                            .arg(iProblemPings)
                                            .arg(visualize(80,event)));
                        pingEvents.append(PingEvent());
                    }
            }
    }
    if (iCurrentID%100==0)
        additionalTimeStamps.insert(iCurrentID,QDateTime::currentDateTime());
}

PingResult PingLog::getPing(int id)
{
    if (problemPings.contains(id))
        return problemPings.value(id);
    else
        return PingResult(cleanPings.value(id,-1));
}

QString PingLog::visualize(int iLineLength, const PingEvent &event)
{
    QString pingVis;
    auto getVisChar = [](const PingResult &pingResult){
        if (!pingResult.hasErrors())
            return ".";
        if (pingResult.errors==PingResult::NO_ERROR){
            return "W";
        }else if (pingResult.warnings==PingResult::NO_WARNING){
            return "E";
        }
        return "X";
    };
    int iEventLength=event.endIndex-event.startIndex+1;
    if (iEventLength<iLineLength){
        for (int i=0;i<iEventLength;++i)
        {
            pingVis.append(getVisChar(getPing(i+event.startIndex)));
        }
    }else {
        int iPingOffset=0;
        for (int i=0;i<iLineLength;++i)
        {
            PingResult result;
            while(iPingOffset <= event.endIndex && (iPingOffset*iLineLength)/iEventLength <= i){
                if (iPingOffset+event.startIndex>event.endIndex)
                    qDebug() << __FILE__ << __LINE__ << "IndexExceeded!";
                result|=getPing(iPingOffset+event.startIndex);
                ++iPingOffset;
            }
            pingVis.append(getVisChar(result));
        }
    }
    return pingVis;
}



QString sec2String(int s)
{
    if (s<60)
        return QString("%1s").arg(s);
    else if (s<3600)
        return QString("%1m %2s").arg(s/60).arg(s%60);
    else return QString("%1h %2m %3s").arg(s/3600).arg((s%3600)/60).arg(s%60);
}
