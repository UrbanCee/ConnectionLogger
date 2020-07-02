#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include "QtCharts"


QT_FORWARD_DECLARE_CLASS(QProcess)
QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QElapsedTimer)
QT_FORWARD_DECLARE_CLASS(PingLog)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void pingFinished();
    void pingTakingLong();
    void pingStarted();
    void ping();
    void on_comboBoxGraphTime_currentTextChanged(const QString &arg1);
    void on_spinBoxMaxPingOnGraph_valueChanged(int arg1);
    void on_horizontalScrollBarGraph_valueChanged(int value);
    void updatePingAliveMarker();

    void on_checkBoxShowGraph_toggled(bool checked);

protected:
    void resizeEvent(QResizeEvent *event);
private:
    void initializeChart();
    void updateSlider();

    Ui::MainWindow *ui;
    QProcess *pingProcess;
    PingLog *pingLog;
    QTimer *pingTimer;
    QTimer *pingTimeOutWarning;
    QElapsedTimer *pingDurationTimer;
    QLineSeries* pingLine;
    QDateTimeAxis* timeAxis;
    QValueAxis *pingAxis;
    qint64 axisDurationS;
    QElapsedTimer *pingAlive;
    QDateTime timeOfFirstDataset;
};

#endif // MAINWINDOW_H
