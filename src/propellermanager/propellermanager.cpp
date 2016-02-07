#include "propellermanager.h"

#include "../common/logging.h"

PropellerManager::PropellerManager(QObject * parent)
    : QObject(parent)
{
    sessions = new PM::SessionManager();
    devices = new PM::DeviceManager();

    connect(&monitor,   SIGNAL(listChanged()),
            this,       SIGNAL(portListChanged()));
}

PropellerManager::~PropellerManager()
{
    delete sessions;
    delete devices;
}

bool PropellerManager::beginSession(PropellerSession * session)
{
    if (!session) return false;
//    qCDebug(pmanager) << "beginning" << session;

    PM::SessionInterface * interface = sessions->interface(session);
    session->attach(interface);

    return true;
}

void PropellerManager::endSession(PropellerSession * session)
{
    if (!session) return;
//    qCDebug(pmanager) << "ending" << session;

    session->detach();
    sessions->remove(session);
}

bool PropellerManager::reserve(PropellerSession * session)
{
    PM::SessionInterface * interface = sessions->interface(session);

    if (interface->isPaused()) return false;
    if (interface->isReserved()) return true;

//    qCDebug(pmanager) << "reserving" << session->portName() << "for" << session;

    foreach (PM::SessionInterface * interface, sessions->list())
    {
        if (interface->portName() == session->portName())
            interface->setPaused(true);
    }

    interface->setPaused(false);
    interface->setReserved(true);

    return true;
}

bool PropellerManager::isReserved(PropellerSession * session)
{
    PM::SessionInterface * interface = sessions->interface(session);
    return interface->isReserved();
}

void PropellerManager::release(PropellerSession * session)
{
    PM::SessionInterface * interface = sessions->interface(session);

    if (interface->isPaused()) return;
    if (!interface->isReserved()) return;

//    qCDebug(pmanager) << "releasing" << session->portName() << "from" << session;

    foreach (PM::SessionInterface * interface, sessions->list())
    {
        if (interface->portName() == session->portName())
            interface->setPaused(false);
    }

    interface->setReserved(false);
}

void PropellerManager::setPortName(PropellerSession * session, const QString & name)
{
    PM::SessionInterface * sessionInterface = sessions->interface(session);


    bool exists = devices->exists(name);
    PM::DeviceInterface * deviceInterface = devices->interface(name);

    if(!exists)
        connect(deviceInterface,    SIGNAL(readyRead()),    sessions,   SLOT(readyBuffer()));

    sessionInterface->detach();
    sessionInterface->attach(deviceInterface);
}

QStringList PropellerManager::listPorts()
{
    return monitor.list();
}

void PropellerManager::enablePortMonitor(bool enabled, int timeout)
{
    monitor.toggle(enabled, timeout);
}

