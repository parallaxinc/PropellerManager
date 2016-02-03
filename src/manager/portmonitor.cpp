#include "portmonitor.h"

#include "../device/propellerdevice.h"
#include "../util/logging.h"
    
PortMonitor::PortMonitor(QObject * parent)
    : QObject(parent)
{
    _ports = PropellerDevice::list();
}

PortMonitor::~PortMonitor()
{
}

void PortMonitor::toggle(bool enabled, int timeout)
{
    if (enabled)
    {
        check();
        connect(&timer, SIGNAL(timeout()), this, SLOT(check()));
        timer.start(timeout);
    }
    else
    {
        timer.stop();
        disconnect(&timer, SIGNAL(timeout()), this, SLOT(check()));
    }
}

void PortMonitor::check()
{
    QStringList newports = PropellerDevice::list();

    if(_ports != newports)
    {
        _ports = newports;
//        qCDebug(pmanager) << "devices changed:" << _ports;
        emit listChanged();
    }
}


QStringList PortMonitor::list()
{
    return _ports;
}

