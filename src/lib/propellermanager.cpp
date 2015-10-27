#include "propellermanager.h"

#include <QDebug>

#include "propellerdevice.h"

PropellerManager::PropellerManager( QObject * parent)
    : QObject(parent)
{

}

PropellerManager::~PropellerManager()
{

}

const QStringList & PropellerManager::listPorts()
{
    return _ports;
}

void PropellerManager::enablePortMonitor(bool enabled)
{
    if (enabled)
    {
        checkPorts();
        connect(&portMonitor, SIGNAL(timeout()), this, SLOT(checkPorts()));
        portMonitor.start(200);
    }
    else
    {
        portMonitor.stop();
        disconnect(&portMonitor, SIGNAL(timeout()), this, SLOT(checkPorts()));
    }
}

void PropellerManager::checkPorts()
{
    QStringList newports = PropellerDevice::list();

    if(_ports != newports)
    {
        _ports = newports;
        emit portListChanged();
    }
}
