#pragma once

#include "propellermanager.h"

#include <QTimer>
#include <QStringList>

class PropellerManager;

class PropellerSession : public QObject
{
    Q_OBJECT

private:
    QString port;
    QString session;
    PropellerManager * manager;

public:
    PropellerSession(
            const QString & portname, 
            PropellerManager * manager,
            QObject * parent = 0);
    ~PropellerSession();

    const QString & portName();
    void            setPortName(const QString & name);
    const QString & sessionName();
    void            setSessionName(const QString & name);

public:
    bool        clear();

    bool        setBaudRate(quint32 baudRate);

    qint64      bytesToWrite();
    qint64      bytesAvailable();

    QByteArray  read(qint64 maxSize);
    QByteArray  readAll();

    bool        putChar(char c);
    qint64      write(const QByteArray & byteArray);
    int         error();
    QString     errorString();
    quint32     minimumTimeout();
    void        setMinimumTimeout(quint32 milliseconds);
    quint32     calculateTimeout(quint32 bytes);
    void        useReset(const QString & name, int pin);
    void        useDefaultReset();
    bool        reset();

signals:
    void        bytesWritten(qint64 bytes);
    void        readyRead();
    void        baudRateChanged(qint32 baudRate);
    void        sendError(const QString & message);
    void        finished();

    void        timeover();
    void        _write_buffer_empty();

public slots:
    void        timeOver();
    void        writeBufferEmpty();
};

