#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <QJsonObject>

QJsonObject readJson(const QString &filename, QString &error);
QJsonObject readJson(const QString &filename);

QStringList toStringList(const QJsonArray &);
QStringList toStringList(const QJsonValue &);


extern QJsonObject graphData;
extern QJsonObject graphTimeData;


#endif // JSONHELPER_H
