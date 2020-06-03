#include "mainwindow.h"
#include <QApplication>
#include "jsonhelper.h"

int main(int argc, char *argv[])
{
    graphData=readJson(":/res/graphData.json");
    graphTimeData=readJson(":/res/graphData.json").value("timeSettings").toObject();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
