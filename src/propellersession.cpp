#include "propellersession.h"

PropellerSession::PropellerSession(
        PropellerManager * manager,
        const QString & portname)
    : Connector<PM::SessionInterface *>()
{
    this->manager = manager;

    manager->beginSession(this);
    setPortName(portname);
}

PropellerSession::~PropellerSession()
{
    manager->endSession(this);
}

void PropellerSession::setPortName(const QString & name)
{
    manager->setPortName(this, name);
}

bool PropellerSession::reserve()
{
    return manager->reserve(this);
}

bool PropellerSession::isReserved()
{
    return manager->isReserved(this);
}

void PropellerSession::release()
{
    manager->release(this);
}
