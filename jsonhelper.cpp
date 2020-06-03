#include "jsonhelper.h"
#include <QtDebug>
#include <QFile>
#include <QJsonParseError>
#include <QJsonArray>

QJsonObject graphData;
QJsonObject graphTimeData;


QJsonObject readJson(const QString &filename, QString &errorString)
{
    QFile inFile( filename );
    if ( !inFile.open(QIODevice::ReadOnly) )
    {
        errorString=QString("Fehler beim Lesen der Datei %1. Datei konnte nicht geöffnet werden.").arg(filename);
        return QJsonObject();
    }
    QJsonParseError error;
    QJsonDocument doc=QJsonDocument::fromJson(inFile.readAll(),&error);
    if (error.error==QJsonParseError::NoError){
        return QJsonObject(doc.object());
    }
    errorString=QString("Fehler beim parsen von %1:%2").arg(filename).arg(error.errorString());
    return QJsonObject();
}


QJsonObject readJson(const QString &filename)
{
    QString error;
    QJsonObject obj=readJson(filename,error);
    if (!error.isEmpty())
    {
        qDebug() << error;
    }
    return obj;
}


QStringList toStringList(const QJsonArray &array)
{
    QStringList list;
    foreach(QJsonValue val,array)
    {
        list.append(val.toString());
    }
    return list;
}


QStringList toStringList(const QJsonValue &val)
{
    return toStringList(val.toArray());
}

