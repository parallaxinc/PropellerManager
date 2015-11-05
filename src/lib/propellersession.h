#pragma once

#include "propellermanager.h"

#include <QTimer>
#include <QStringList>

/**
    @class PropellerSession
      
    @brief The PropellerSession class provides a persistent, always-on interface to Propeller hardware via PropellerManager.
    
    PropellerSession provides a persistent environment from which to
    interact with the Propeller.
    
    Sessions can communicate with any available device without needing to 
    manually open and close ports. Many sessions can be connected to a single
    device at a time. However, sessions may also request exclusive access to
    a device to complete a long-running operation.
    
    This ensures that the program always behaves as expected and makes it
    possible to debug the Propeller in RAM on non-Windows platforms.
    */

class PropellerManager;

class PropellerSession : public QObject
{
    Q_OBJECT

private:
    QString port;
    PropellerManager * manager;

public:
    PropellerSession(
            PropellerManager * manager,
            const QString & portname = QString(), 
            QObject * parent = 0);
    ~PropellerSession();

    const QString & portName();
    void            setPortName(const QString & name);

public:
    bool        isOpen();

    bool        reserve();
    bool        isReserved();
    void        release();

    void        pause();
    bool        isPaused();
    void        unpause();

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
    void        deviceFree();
    void        deviceBusy();

    void        timeover();
    void        allBytesWritten();

public slots:
    void        timeOver();
    void        writeBufferEmpty();

};

