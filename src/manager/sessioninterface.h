#pragma once

#include <QObject>

#include "connector.h"
#include "readbuffer.h"
#include "deviceinterface.h"

class SessionInterface : public Connector<DeviceInterface *>
{
    Q_OBJECT

    ReadBuffer * _buffer;
    bool _reserved;

public:
    SessionInterface ()
        : Connector<DeviceInterface *>()
    {
        _buffer = new ReadBuffer();
        _reserved = false;
    }

    ~SessionInterface()
    {
        delete _buffer;
    }

    bool clear()
    {
        if (!isActive()) return false;
        _buffer->clear();
        Connector<DeviceInterface *>::clear();
        return true;
    }

    void append(QByteArray ba)
    {
        _buffer->append(ba);
    }

    QByteArray read(qint64 maxSize)
    {
        return _buffer->read(maxSize);
    }

    QByteArray readAll()
    {
        return _buffer->readAll();
    }

    qint64 bytesAvailable()
    {
        return _buffer->bytesAvailable();
    }

    void setReserved(bool reserved)
    {
        _reserved = reserved;
    }

    bool isReserved()
    {
        return _reserved;
    }

protected:
    void attachSignals()
    {
        connect(_target,    SIGNAL(sendError(const QString &)), this,   SIGNAL(sendError(const QString &)));
        connect(_target,    SIGNAL(bytesWritten(qint64)),       this,   SIGNAL(bytesWritten(qint64)));
        connect(_target,    SIGNAL(baudRateChanged(qint32)),    this,   SIGNAL(baudRateChanged(qint32)));

        connect(_buffer,    SIGNAL(readyRead()),                this,   SIGNAL(readyRead()));
    }

    void detachSignals()
    {
        disconnect(_target,    SIGNAL(sendError(const QString &)), this,   SIGNAL(sendError(const QString &)));
        disconnect(_target,    SIGNAL(bytesWritten(qint64)),       this,   SIGNAL(bytesWritten(qint64)));
        disconnect(_target,    SIGNAL(baudRateChanged(qint32)),    this,   SIGNAL(baudRateChanged(qint32)));

        disconnect(_buffer,    SIGNAL(readyRead()),                this,   SIGNAL(readyRead()));
    }
};

