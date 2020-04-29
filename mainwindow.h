#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>

QT_FORWARD_DECLARE_CLASS(QProcess)
QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(ErrorHandler)

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
private:
    Ui::MainWindow *ui;
    ErrorHandler *errorHandler;
    QProcess *pingProcess;
    QTimer *pingTimer;
    QTimer *pingTimeOutWarning;
    QTime timePingStarted;
};

#endif // MAINWINDOW_H
