#include "propellermanager.h"

#include "../util/logging.h"

PropellerManager::PropellerManager(QObject * parent)
    : QObject(parent)
{
    sessions = new SessionManager();
    devices = new DeviceManager();

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

    SessionInterface * interface = sessions->interface(session);
    session->attach(interface);

    return true;
}

void PropellerManager::endSession(PropellerSession * session)
{
    if (!session) return;

    session->detach();
    sessions->remove(session);
}

void PropellerManager::setPortName(PropellerSession * session, const QString & name)
{
    if (name.isEmpty()) return;

    SessionInterface * sessionInterface = sessions->interface(session);

    DeviceInterface * deviceInterface = devices->interface(name);
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

