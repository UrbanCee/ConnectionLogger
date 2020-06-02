#include "mainwindow.h"
#include "errorhandler.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QTimer>
#include <QtDebug>
#include <QSpinBox>
#include <QDateTime>
#include <QElapsedTimer>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),pingProcess(nullptr),pingLog(nullptr),pingTimer(nullptr),pingLine(new QLineSeries),timeAxis(new QDateTimeAxis),pingAxis(new QValueAxis),axisDurationS(1000)
{
    ui->setupUi(this);
    ui->textEditPing->setHtml("");
    ui->textEditLog->setHtml("");
    setWindowIcon(QIcon(":/res/wifi_icon.png"));

    initializeChart();

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

    pingDurationTimer = new QElapsedTimer();
    ping();
}

void MainWindow::initializeChart()
{
    ui->chartViewPing->chart()->addSeries(pingLine);
    QPen pingLinePen = pingLine->pen();
    pingLinePen.setWidth(1);
    pingLinePen.setColor(Qt::blue);
    pingLine->setPen(pingLinePen);

    ui->chartViewPing->chart()->addAxis(timeAxis, Qt::AlignBottom);
    ui->chartViewPing->setRenderHint(QPainter::Antialiasing);
    pingLine->attachAxis(timeAxis);

    ui->chartViewPing->chart()->addAxis(pingAxis,Qt::AlignLeft);
    pingAxis->setRange(0,100);
    pingAxis->setLabelFormat("%d");
    pingAxis->setTickCount(6);
    pingLine->attachAxis(pingAxis);
    ui->chartViewPing->chart()->legend()->hide();
    ui->chartViewPing->chart()->layout()->setContentsMargins(0,0,0,0);
    ui->chartViewPing->chart()->setBackgroundRoundness(0);

    ui->comboBoxGraphTime->addItems({"min","10 min","hour","day","week","month"});
    ui->comboBoxGraphTime->setCurrentText("hour");
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
    pingDurationTimer->start();
}

void MainWindow::pingFinished()
{
    int iInternalPing = pingDurationTimer->elapsed();
    pingTimeOutWarning->stop();
    PingResult result(QString(pingProcess->readAll()));
    switch(pingProcess->exitCode())
    {
    case 0: // all ok
        break;
    case 1: // no reply
        result.addError(PingResult::PING_NO_REPLY_ERROR);
        break;
    case 2: // error
        result.addError(PingResult::PING_OTHER_ERROR);
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

    pingTimer->setInterval(ui->spinBoxPingDelay->value());
    pingTimer->start();
    pingLine->append(result.time.toMSecsSinceEpoch(),result.ping);
    timeAxis->setRange(QDateTime::currentDateTime().addSecs(-axisDurationS),QDateTime::currentDateTime());
}


void MainWindow::on_comboBoxGraphTime_currentTextChanged(const QString &arg1)
{
    if (arg1=="min"){
        timeAxis->setFormat("h:mm:ss");
        axisDurationS=60;
    }else if (arg1=="10 min"){
        timeAxis->setFormat("h:mm:ss");
        axisDurationS=600;
    }else if (arg1=="hour"){
        timeAxis->setFormat("h:mm");
        axisDurationS=3600;
    }else if (arg1=="day"){
        timeAxis->setFormat("h:mm");
        axisDurationS=3600*24;
    }else if (arg1=="week"){
        timeAxis->setFormat("dd.MM.");
        axisDurationS=3600*24*7;
    }else if (arg1=="month"){
        timeAxis->setFormat("dd.MM.");
        axisDurationS=3600*24*30;
    }
    timeAxis->setRange(QDateTime::currentDateTime().addSecs(-axisDurationS),QDateTime::currentDateTime());
}
