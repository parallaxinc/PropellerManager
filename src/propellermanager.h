#pragma once

#include <QObject>
#include "portmonitor.h"
#include "devicemanager.h"
#include "sessionmanager.h"
#include "propellersession.h"

namespace PM
{
    class SessionManager;
}

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

class PropellerManager : public QObject
{
    Q_OBJECT

    PM::PortMonitor monitor;
    PM::DeviceManager * devices;
    PM::SessionManager * sessions;

    PM::PropellerDevice * getDevice(const QString & name);

private slots:
    void openNewPorts();

public:
    PropellerManager(QObject *parent = 0);
    ~PropellerManager();

    void closePort(const QString & name);
    void openPort(const QString & name);
    QStringList listPorts();
    QStringList latestPorts();
    void enablePortMonitor(bool enabled, int timeout = 200);

/// @cond

    bool beginSession(PropellerSession * session);
    void endSession(PropellerSession * session);

    bool reserve(PropellerSession * session);
    bool isReserved(PropellerSession * session);
    void release(PropellerSession * session);

    void setPortName(PropellerSession * session, const QString & name);

/// @endcond

signals:
    void portListChanged();
};
