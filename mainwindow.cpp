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
}

void MainWindow::pingTakingLong()
{

}

void MainWindow::pingFinished()
{
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
    ui->textEditPing->append(returnString);
    QRegularExpression re("time=(?<time>\\d+\\.\\d+)\\sms");
    QRegularExpressionMatch match = re.match(returnString);
    if (!match.hasMatch())
    {
        errorString.append("No time found! ");
    }
    if (errorString.isEmpty())
    {
        int pingTime=static_cast<int> (match.captured("time").toDouble());
        ui->labelStatus->setText(QString("Last Ping: %1 ms").arg(pingTime));
    }
    else{
        ui->textEditLog->append(QString("%1 %2")
                                .arg(QString("[%1]:").arg(QDateTime::currentDateTime().toString()))
                                .arg(errorString));
    }
    pingTimer->setInterval(ui->spinBoxPingDelay->value());
    pingTimer->start();
}

