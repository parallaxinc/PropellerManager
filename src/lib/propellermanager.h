#pragma once

#include <QTimer>
#include <QStringList>
#include <QMap>

#include "propellerdevice.h"
#include "propellersession.h"

class PropellerSession;

class PropellerManager : public QObject
{
    Q_OBJECT

private:
    QTimer portMonitor;
    QStringList _ports;
    QMap<QString, PropellerDevice *> _devices;
    QMap<QString, PropellerSession *> _busy;
    QMap<PropellerSession *, PropellerSession *> _sessions;
    
    void attach(PropellerSession * session, PropellerDevice * device);
    void attachByName(PropellerSession * session, const QString & port);
    void detach(PropellerSession * session, PropellerDevice * device);
    bool portIsBusy(PropellerSession * session, const QString & name);

public:
    PropellerManager(QObject *parent = 0);
    ~PropellerManager();

    PropellerSession * newSession(const QString & port);
    PropellerDevice * getDevice(const QString & port);
    void deleteSession(PropellerSession * session);
    void deleteDevice(const QString & port);

/**
    @name Port Monitoring

    PropellerManager enables you to monitor all available
    connections in the background of your application, and
    provides functions to update it in real-time.
  */

/**@{*/
public:
    const QStringList & listPorts();
    void enablePortMonitor(bool enabled);

public slots:
    void checkPorts();

signals:
    void portListChanged();
/**@}*/

/**
    @name I/O Device Functions

    Provide serial device functions that can be sent
    to any available device.
  */

/**@{*/
public:
    bool        open(PropellerSession * session, const QString & port);
    bool        isOpen(PropellerSession * session, const QString & port);
    void        close(PropellerSession * session, const QString & port);
    bool        clear(PropellerSession * session, const QString & port);

    bool        setBaudRate(PropellerSession * session, const QString & port, quint32 baudRate);

    qint64      bytesToWrite(PropellerSession * session, const QString & port);
    qint64      bytesAvailable(PropellerSession * session, const QString & port);

    QByteArray  read(PropellerSession * session, const QString & port, qint64 maxSize);
    QByteArray  readAll(PropellerSession * session, const QString & port);

    bool        putChar(PropellerSession * session, const QString & port, char c);
    qint64      write(PropellerSession * session, const QString & port, const QByteArray & byteArray);
    QString     errorString(PropellerSession * session, const QString & port);
    int         error(PropellerSession * session, const QString & port);

    quint32     minimumTimeout(PropellerSession * session, const QString & port);
    void        setMinimumTimeout(PropellerSession * session, const QString & port, quint32 milliseconds);
    quint32     calculateTimeout(PropellerSession * session, const QString & port, quint32 bytes);
    void        useReset(PropellerSession * session, const QString & port, const QString & name, int pin);
    void        useDefaultReset(PropellerSession * session, const QString & port);
    bool        reset(PropellerSession * session, const QString & port);
/**@}*/

};
