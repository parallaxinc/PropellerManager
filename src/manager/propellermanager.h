#pragma once

#include <QTimer>
#include <QStringList>
#include <QHash>

#include "../device/propellerdevice.h"
#include "../session/propellersession.h"
#include "../session/readbuffer.h"

/**
@class PropellerManager manager/propellermanager.h PropellerManager

@brief The PropellerManager class provides an abstraction layer between PropellerSession and PropellerDevice instances.

There are several advantages to the PropellerManager approach.

- Applications are easier to write without device connectivity and error handling.
- Does not require multi-threading, which is not always available.
- Applications can immediately leverage new download strategies without changing code.
- Cleanly shares devices without ever disconnecting, side-stepping a host of platform usability issues.
- Build your applications on a battle-hardened Propeller implementation instead of rolling your own.

PropellerManager mediates access to hardware and allows it to be shared between applications seamlessly.

PropellerManager works by acting as a signal router between PropellerSession and
PropellerDevice instances, preventing them from interacting with each other directly.

@code
PropellerManager manager;
PropellerSession session(&manager);

QByteArray data;
session.write(data);
@endcode

One of the unique features of PropellerManager is that it makes opening and closing devices unnecessary. It does so
by maintaining a count of the number of sessions currently accessing any given device, and as long as this count
remains above 0, the device will be held open.

### Port Monitoring

PropellerManager enables you to monitor all available connections in the background
of your application, and provides functions to update it in real-time.

Enabling port monitoring in your application is simple. Here is an example Qt class with
monitoring configured.

\include portmonitor/portmonitor.h

And that's it! Now your application knows what Propeller devices are connected at any given time.

@cond

### PropellerSession Interface

These functions implement the interface between PropellerSession
and PropellerManager.


\warning It is not recommended to use these functions directly.
         Use PropellerSession to interface with PropellerManager.

@endcond

\see PropellerSession

*/


/**
@example portmonitor/main.cpp

Here is an example of how to enable the port monitor in an application.

First, we implement a class to listen to the port monitor.

\include portmonitor/portmonitor.h

Then initialize the class and run.
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
    QHash<PropellerDevice *, PropellerSession *> _busy;
    QHash<PropellerSession *, PropellerSession *> _paused;
    QHash<PropellerSession *, PropellerSession *> _sessions;

    QHash<PropellerSession *, PropellerDevice *> _connections;
    QHash<PropellerSession *, PropellerDevice *> _saved_connections;

    QHash<PropellerSession *, ReadBuffer *> _buffers;
    
    void attach(PropellerSession * session, PropellerDevice * device);
    void attachByName(PropellerSession * session, const QString & port);
    void detach(PropellerSession * session, PropellerDevice * device);
    bool portIsBusy(PropellerSession * session, const QString & name);

    PropellerDevice * getDevice(const QString & port, bool open = true);
    void deleteDevice(const QString & port);

private slots:
    void readyBuffer();
    void checkPorts();

public:
    PropellerManager(QObject *parent = 0);
    ~PropellerManager();

    const QStringList & listPorts();
    void enablePortMonitor(bool enabled);

/// @cond

    bool        beginSession(PropellerSession * session);
    void        endSession(PropellerSession * session);

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
    quint32     resetPeriod(PropellerSession * session, const QString & port);

    bool        reserve(PropellerSession * session, const QString & port);
    bool        isReserved(PropellerSession * session, const QString & port);
    void        release(PropellerSession * session, const QString & port);

    void        pause(PropellerSession * session);
    bool        isPaused(PropellerSession * session);
    void        unpause(PropellerSession * session);

/// @endcond

signals:
    void portListChanged();
};
