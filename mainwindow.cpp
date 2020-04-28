#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QTimer>
#include <QtDebug>
#include <QSpinBox>

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
//    pingTimer->start();
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
}

void MainWindow::pingTakingLong()
{

}

void MainWindow::pingFinished()
{
    QString returnString=pingProcess->readAll();
    ui->textEditPing->append(returnString);
    QRegularExpression re("time=(?<time>\\d+\\.\\d+)\\sms");
    QRegularExpressionMatch match = re.match(returnString);
    int pingTime=static_cast<int> (match.captured("time").toDouble());
    qDebug() << pingTime;
    ui->labelStatus->setText(QString("Last Ping: %1 ms").arg(pingTime));
    pingTimer->setInterval(ui->spinBoxPingDelay->value());
    pingTimer->start();
}

