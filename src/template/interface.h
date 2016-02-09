#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>

class Interface : public QObject
{
    Q_OBJECT

public:
    Interface() : QObject()
    {
    }

    virtual bool        isOpen() = 0;
    virtual bool        clear() = 0;
    virtual QString     portName() = 0;
    virtual quint32     baudRate() = 0;
    virtual bool        setBaudRate(quint32 baudRate) = 0;

    virtual qint64      bytesToWrite() = 0;
    virtual qint64      bytesAvailable() = 0;
    virtual QByteArray  read(qint64 maxSize) = 0;
    virtual QByteArray  readAll() = 0;
    virtual bool        putChar(char c) = 0;
    virtual qint64      write(QByteArray ba) = 0;

    virtual quint32     minimumTimeout() = 0;
    virtual void        setMinimumTimeout(quint32 milliseconds) = 0;
    virtual quint32     calculateTimeout(quint32 bytes) = 0;

    virtual void        useReset(QString name, int pin) = 0;
    virtual void        useDefaultReset() = 0;
    virtual bool        reset() = 0;
    virtual quint32     resetPeriod() = 0;

signals:
    void sendError(const QString & message);
    void bytesWritten(qint64 bytes);
    void baudRateChanged(qint32 baudRate);
    void readyRead();
    void deviceStateChanged(bool enabled);
    void deviceAvailableChanged(bool available);
};

