#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QDebug>


class ConnectorSignals : public QObject
{
    Q_OBJECT

public:
    ConnectorSignals(QObject * parent = 0)
        : QObject(parent)
    {
    }

signals:
    void bytesWritten(qint64 bytes);
    void readyRead();
    void baudRateChanged(qint32 baudRate);
    void sendError(const QString & message);
    void deviceFree();
    void deviceBusy();
};

template <class Target>
class Connector : public ConnectorSignals
{
    bool        _attached;
    bool        _paused;

protected:
    Target      _target;

    void attachSignals()
    {
        connect(_target,    SIGNAL(sendError(const QString &)), this,   SIGNAL(sendError(const QString &)));
        connect(_target,    SIGNAL(bytesWritten()),             this,   SIGNAL(bytesWritten()));
        connect(_target,    SIGNAL(baudRateChanged()),          this,   SIGNAL(baudRateChanged()));
        connect(_target,    SIGNAL(readyRead()),                this,   SIGNAL(readyRead()));
    }

    void detachSignals()
    {
        disconnect(_target,    SIGNAL(sendError(const QString &)), this,   SIGNAL(sendError(const QString &)));
        disconnect(_target,    SIGNAL(bytesWritten()),             this,   SIGNAL(bytesWritten()));
        disconnect(_target,    SIGNAL(baudRateChanged()),          this,   SIGNAL(baudRateChanged()));
        disconnect(_target,    SIGNAL(readyRead()),                this,   SIGNAL(readyRead()));
    }

public:
    Connector() : ConnectorSignals()
    {
    }

    Target target()
    {
        return _target;
    }

    bool isPaused()
    {
        return _paused;
    }

    void setPaused(bool paused)
    {
        _paused = paused;
    }

    bool isAttached()
    {
        return _attached;
    }

    bool isActive()
    {
        return (!_paused && _attached);
    }

    bool attach(Target target)
    {
        if (isAttached()) return false;
    
        _target = target;
    
        qDebug() << "connecting" << this << "to" << _target;

        attachSignals();
    
        _attached = true;

        return true;
    }

    void detach()
    {
        if (!isAttached()) return;
    
        qDebug() << "removing" << this << "from" << _target;

        detachSignals();
    
        _attached = false;
    }

    bool isOpen()
    {
        if (!isActive()) return false;
        return _target->isOpen();
    }

    bool clear()
    {
        if (!isActive()) return false;
        return _target->clear();
    }

    void setPortName(const QString & name)
    {
        _target->setPortName(name);
    }

    bool setBaudRate(quint32 baudRate)
    {
        if (!isActive()) return false;
        return _target->setBaudRate(baudRate);
    }

    quint32 baudRate()
    {
        return _target->baudRate();
    }

    qint64 bytesToWrite()
    {
        if (!isActive()) return 0;
        return _target->bytesToWrite();
    }

    qint64 bytesAvailable()
    {
        if (!isActive()) return 0;
        return _target->bytesAvailable();
    }

    QByteArray read(qint64 maxSize)
    {
        if (!isActive()) return QByteArray();
        return _target->read(maxSize);
    }

    QByteArray readAll()
    {
        if (!isActive()) return QByteArray();
        return _target->readAll();
    }

    bool putChar(char c)
    {
        if (!isActive()) return false;
        return _target->putChar(c);
    }

    qint64 write(QByteArray ba)
    {
        if (!isActive()) return -1;
        return _target->write(ba);
    }

    quint32 minimumTimeout()
    {
        if (!isActive()) return 0;
        return _target->minimumTimeout();
    }

    void setMinimumTimeout(quint32 milliseconds)
    {
        if (!isActive()) return;
        _target->setMinimumTimeout(milliseconds);
    }

    quint32 calculateTimeout(quint32 bytes)
    {
        if (!isActive()) return 0;
        return _target->calculateTimeout(bytes);
    }

    void useReset(QString name, int pin)
    {
        if (!isActive()) return;
        _target->useReset(name, pin);
    }

    void useDefaultReset()
    {
        if (!isActive()) return;
        return _target->useDefaultReset();
    }

    bool reset()
    {
        if (!isActive()) return false;
        return _target->reset();
    }

    quint32 resetPeriod()
    {
        if (!isActive()) return 0;
        return _target->resetPeriod();
    }

    int error()
    {
        if (!isActive()) return 0;
        return _target->error();
    }

    QString errorString()
    {
        if (!isActive()) return QString();
        return _target->errorString();
    }

};

