#pragma once

#include <QByteArray>
#include <QFile>
#include <QString>

class Utility : public QObject
{
    Q_OBJECT
public:
    static QByteArray readFile(QString filename);
public slots:
    static void print(const QString & text);
    static void print_task(const QString & text);
    static void print_status(const QString & text);
    static void print_error(int code, const QString & message);
};
