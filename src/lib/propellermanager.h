#pragma once

#include <QTimer>
#include <QStringList>
#include <QHash>

#include "propellerdevice.h"
#include "propellersession.h"

/**
@class PropellerManager

@brief The PropellerManager class provides an abstraction layer between PropellerSession and PropellerDevice instances.

PropellerManager mediates access to hardware and allows it to be shared between applications seamlessly.

PropellerManager works by acting as a signal router between PropellerSession and
PropellerDevice instances, preventing them from interacting with each other directly.
*/

class PropellerSession;

class PropellerManager : public QObject
{
    Q_OBJECT

private:
    QTimer portMonitor;
    QStringList _ports;
    QHash<QString, PropellerDevice *> _devices;
    QHash<PropellerDevice *, quint32> _active_sessions;
    QHash<QString, PropellerSession *> _busy;
    QHash<PropellerSession *, PropellerSession *> _sessions;
    QHash<PropellerSession *, PropellerDevice *> _connections;
    
    void attach(PropellerSession * session, PropellerDevice * device);
    void attachByName(PropellerSession * session, const QString & port);
    void detach(PropellerSession * session, PropellerDevice * device);
    bool portIsBusy(PropellerSession * session, const QString & name);

    PropellerDevice * getDevice(const QString & port);
    void deleteDevice(const QString & port);

public:
    PropellerManager(QObject *parent = 0);
    ~PropellerManager();

    PropellerSession * session(const QString & port);
    void endSession(PropellerSession * session);

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
    void message(const QString & message, const QString & port);

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
    bool        isOpen(PropellerSession * session, const QString & port);
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
