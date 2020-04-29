#include "errorhandler.h"

#include <QTimer>
#include <QWidget>

ErrorHandler::ErrorHandler(QWidget *parent)
    : QObject(parent), parentWidget(parent), timer(new QTimer(this)), currErr(nullptr)
{

}
