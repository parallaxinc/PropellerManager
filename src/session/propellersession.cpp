#include "propellersession.h"

#include <QDebug>

#include "../device/propellerdevice.h"

class PropellerManager;

PropellerSession::PropellerSession(
        PropellerManager * manager,
        const QString & portname, 
        QObject * parent)
    : Connector<SessionInterface *>()
{
//    attach(manager);
    setPortName(portname);

//    manager->beginSession(this);
}

PropellerSession::~PropellerSession()
{
//    manager->endSession(this);
}

/**
    Returns the name of the current device.
  */
const QString & PropellerSession::portName()
{
    return port;
}

/**
    Sets the name of the current device.
  */
void PropellerSession::setPortName(const QString & name)
{
    port = name;
//    target()->setPortName(this);
}

///**
//    Reserve the device for exclusive access.
//
//    \returns true if successfully reserved, otherwise false.
//  */
//bool PropellerSession::reserve()
//{
//    return manager->reserve(this);
//}
//
///**
//    Returns true if this session has reserved the device, otherwise false.
//  */
//bool PropellerSession::isReserved()
//{
//    return manager->isReserved(this);
//}
//
///**
//    Releases the device so that other sessions can use it.
//  */
//void PropellerSession::release()
//{
//    manager->release(this);
//}
