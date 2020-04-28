#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QTimer>
#include <QtDebug>
#include <QSpinBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),pingProcess(nullptr),pingTimer(nullptr)
{
    ui->setupUi(this);
    pingProcess = new QProcess(this);
    connect(pingProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(pingFinished()));
    pingTimer = new QTimer(this);
    pingTimeOutWarning = new QTimer(this);

    connect(pingTimer,SIGNAL(timeout()),this,SLOT(ping()));
    pingTimer->setInterval(ui->spinBoxPingDelay->value());
    pingTimeOutWarning->setInterval(1000);
    ping();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ping()
{
    QString command = "ping";
    QStringList args{"-c" ,  "1" ,"8.8.8.8"};
    pingProcess->start(command, args);
    pingTimer->stop();
    timePingStarted.start();
}

void MainWindow::pingTakingLong()
{

}

void MainWindow::pingFinished()
{
    int iInternalPing = timePingStarted.elapsed();
    QString returnString=pingProcess->readAll();
    QString errorString;
    switch(pingProcess->exitCode())
    {
    case 0: // all ok
        break;
    case 1: // no reply
        errorString.append("Ping returned no reply! ");
        break;
    case 2: // error
        errorString.append("Ping returned error! ");
        break;
    default:
        break;
    }
    QRegularExpression re("time=(?<time>\\d+\\.*\\d*)\\sms");
    QRegularExpressionMatch match = re.match(returnString);
    if (!match.hasMatch())
    {
        errorString.append("No time found! ");
    }
    if (errorString.isEmpty())
    {
        int pingTime=static_cast<int> (match.captured("time").toDouble());
        ui->labelStatus->setText(QString("Last Ping: %1 ms").arg(pingTime));
        ui->labelStatus->setStyleSheet("QLabel {color  : green }");
        if (pingTime>ui->spinBoxHighPing->value())
            ui->textEditLog->append(QString("%1 %2")
                                    .arg(QString("[%1]:").arg(QDateTime::currentDateTime().toString()))
                                    .arg(QString("High Ping: %1").arg(pingTime)));
        if (qAbs(iInternalPing-pingTime)/static_cast<double>(pingTime)>0.8)
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
    pingTimer->setInterval(ui->spinBoxPingDelay->value());
    pingTimer->start();
}

