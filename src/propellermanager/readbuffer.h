#pragma once

#include <QByteArray>
#include <QObject>

class ReadBuffer : public QObject
{
    Q_OBJECT

    QByteArray array;

public:
    ReadBuffer(QObject * parent = 0);
    ~ReadBuffer();
    QByteArray & append(QByteArray ba);
    QByteArray read(qint64 maxSize);
    QByteArray readAll();
    void clear();
    quint64 bytesAvailable();

signals:
    void readyRead();
};

