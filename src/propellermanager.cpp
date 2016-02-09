#include "propellermanager.h"

#include "logging.h"

PropellerManager::PropellerManager(QObject * parent)
    : QObject(parent)
{
    sessions = new PM::SessionManager();
    devices = new PM::DeviceManager();

    connect(&monitor,   SIGNAL(listChanged()),
            this,       SIGNAL(portListChanged()));

    connect(&monitor,   SIGNAL(listChanged()),
            this,       SLOT(openNewPorts()));
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
        {
            interface->setPaused(true);
        }
    }

    interface->setPaused(false);
    interface->setReserved(true);

    foreach (PM::SessionInterface * interface, sessions->list())
    {
        if (interface->portName() == session->portName())
        {
            emit interface->deviceAvailableChanged(!interface->isPaused());
        }
    }

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

    foreach (PM::SessionInterface * interface, sessions->list())
    {
        if (interface->portName() == session->portName())
        {
            emit interface->deviceAvailableChanged(!interface->isPaused());
        }
    }
}

void PropellerManager::setPortName(PropellerSession * session, const QString & name)
{
    PM::SessionInterface * sessionInterface = sessions->interface(session);
    PM::PropellerDevice * deviceInterface = getDevice(name);

    QString oldname = sessionInterface->portName();

    sessionInterface->detach();
    sessionInterface->attach(deviceInterface);

    if (name != oldname)
        emit deviceInterface->deviceStateChanged(deviceInterface->isOpen());
}

void PropellerManager::closePort(const QString & name)
{
    if (devices->exists(name))
        devices->interface(name)->setEnabled(false);
}

void PropellerManager::openPort(const QString & name)
{
    if (devices->exists(name))
        devices->interface(name)->setEnabled(true);
}

QStringList PropellerManager::listPorts()
{
    return monitor.list();
}

QStringList PropellerManager::latestPorts()
{
    return monitor.latest();
}

void PropellerManager::enablePortMonitor(bool enabled, int timeout)
{
    monitor.toggle(enabled, timeout);
}

void PropellerManager::openNewPorts()
{
    foreach (QString s, monitor.latest())
    {
        getDevice(s)->open();
    }
}

PM::PropellerDevice * PropellerManager::getDevice(const QString & name)
{
    bool exists = devices->exists(name);
    PM::PropellerDevice * device = devices->interface(name);

    if(!exists)
        connect(device, SIGNAL(readyRead()),    sessions,   SLOT(readyBuffer()));

    return device;
}

