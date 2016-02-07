#pragma once

#include "../common/connector.h"
#include "../propellermanager/sessioninterface.h"
#include "../propellermanager/propellermanager.h"

class PropellerManager;

/**
@class PropellerSession session/propellersession.h PropellerSession
  
@brief The PropellerSession class provides a persistent interface to Propeller hardware via PropellerManager.

Sessions can communicate with any available device without needing to 
manually open and close ports. Many sessions can be connected to a single
device at a time. However, sessions may also request exclusive access to
a device to complete a long-running operation.

### Usage

Using PropellerSession is easy because it handles all of the device setup transparently. In just a few
lines of code, you'll be able to develop Propeller-based applications.

Here is a random function that writes some data to the device.

@code
void writeToDevice()
{
    PropellerManager manager;
    PropellerSession session(&manager);
    session.setPortName("ttyUSB0");
    session.write(QByteArray("some data!"))
}
@endcode

PropellerSession automatically disconnects from PropellerManager when destroyed, so manual cleanup
isn't needed, and it's easy to make sure your connected devices don't end up in an invalid state.

\see PropellerManager
    */

class PropellerSession
    : public Connector<PM::SessionInterface *>
{
    Q_OBJECT

private:
    PropellerManager * manager;

public:
    PropellerSession(
            PropellerManager * manager,
            const QString & portname = QString());
    ~PropellerSession();

    void    setPortName(const QString & name);
    bool    reserve();
    bool    isReserved();
    void    release();

};

