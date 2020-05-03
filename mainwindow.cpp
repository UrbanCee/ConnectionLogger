#include "mainwindow.h"
#include "errorhandler.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QTimer>
#include <QtDebug>
#include <QSpinBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),pingProcess(nullptr),pingLog(nullptr),pingTimer(nullptr)
{
    ui->setupUi(this);
    ui->textEditPing->setHtml("");
    ui->textEditLog->setHtml("");

    pingLog = new PingLog(ui->labelStatus,ui->textEditLog,ui->textEditPing,ui->spinBoxTimeToNorm,this);

    pingProcess = new QProcess(this);
    connect(pingProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(pingFinished()));
    connect(pingProcess,SIGNAL(started()),this,SLOT(pingStarted()));
    pingTimer = new QTimer(this);
    pingTimeOutWarning = new QTimer(this);

    connect(pingTimer,SIGNAL(timeout()),this,SLOT(ping()));
    connect(pingTimeOutWarning,SIGNAL(timeout()),this,SLOT(pingTakingLong()));
    pingTimer->setInterval(ui->spinBoxPingDelay->value());
    pingTimeOutWarning->setInterval(1000);
    ping();
}

MainWindow::~MainWindow()
{
    pingProcess->blockSignals(true);
    pingProcess->waitForFinished(2000);
    delete ui;
}


void MainWindow::ping()
{
    QString command = "ping";
    QStringList args{"-c" ,  "1" ,"8.8.8.8"};
    pingProcess->start(command, args);
    pingTimer->stop();
}

void MainWindow::pingTakingLong()
{
    ui->labelStatus->setText(QString("Ping taking long ... (>%1 ms)").arg(ui->spinBoxPingWarning->value()));
    ui->labelStatus->setStyleSheet("QLabel {color  : rgb(180,180,0);}");
}

void MainWindow::pingStarted()
{
    timePingStarted.start();
}

void MainWindow::pingFinished()
{
    int iInternalPing = timePingStarted.elapsed();
    pingTimeOutWarning->stop();
    PingResult result(pingProcess->readAll());
    switch(pingProcess->exitCode())
    {
    case 0: // all ok
        break;
    case 1: // no reply
        result.addError(PingResult::PING_NO_REPLY_ERROR);
        break;
    case 2: // error
        result.addError(PingResult::PING_RETURN_ERROR);
        break;
    default:
        break;
    }
    if (ui->checkBoxMismatch->isChecked())
        if (qAbs(iInternalPing-result.ping)/static_cast<double>(result.ping)*100>ui->spinBoxMismatchPercent->value())
            result.addWarning(PingResult::TIME_MISMATCH_WARNING);
    if (result.ping>ui->spinBoxHighPing->value())
        result.addWarning(PingResult::HIGH_PING_WARNING);
    pingLog->update(result);

/*    if (errorString.isEmpty())
    {
        int pingTime=static_cast<int> (match.captured("time").toDouble());
        ui->labelStatus->setText(QString("Last Ping: %1 ms").arg(pingTime));
        ui->labelStatus->setStyleSheet("QLabel {color  : green }");
        if (pingTime>ui->spinBoxHighPing->value())
            ui->textEditLog->append(QString("%1 %2")
                                    .arg(QString("[%1]:").arg(QDateTime::currentDateTime().toString()))
                                    .arg(QString("High Ping: %1").arg(pingTime)));
        if (ui->checkBoxMismatch->isChecked())
            if (qAbs(iInternalPing-pingTime)/static_cast<double>(pingTime)*100>ui->spinBoxMismatchPercent->value())
                ui->textEditLog->append(QString("%1 %2")
                                        .arg(QString("[%1]:").arg(QDateTime::currentDateTime().toString()))
                                        .arg(QString("Timing mismatch: ping result: %1ms   internal: %2ms").arg(pingTime).arg(iInternalPing)));

    }
    else{
        errorString.append(QString(" (duration: %1ms").arg(iInternalPing));
        ui->labelStatus->setStyleSheet("QLabel {color  : yellow }");
        ui->textEditLog->append(QString("%1 %2")
                                .arg(QString("[%1]:").arg(QDateTime::currentDateTime().toString()))
                                .arg(errorString));
        ui->textEditPing->append(QString("%1 %2")
                                 .arg(QString("[%1]:").arg(QDateTime::currentDateTime().toString()))
                                 .arg(returnString));
    }
    */
    pingTimer->setInterval(ui->spinBoxPingDelay->value());
    pingTimer->start();
}

