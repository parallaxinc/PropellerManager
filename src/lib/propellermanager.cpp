#include "propellermanager.h"

#include <QDebug>

PropellerManager::PropellerManager( QObject * parent)
    : QObject(parent)
{
    connect(&portMonitor, SIGNAL(portsChanged()), this, SLOT(updatePorts()));

}

PropellerManager::~PropellerManager()
{

}

void PropellerManager::updatePorts()
{
    _ports = portMonitor.ports();
    emit portsChanged();
}

const QStringList & PropellerManager::ports()
{
    return _ports;
}
