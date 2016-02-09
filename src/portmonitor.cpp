#include "portmonitor.h"

#include <QSet>

#include "logging.h"
#include "propellerdevice.h"

namespace PM
{
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
        newports.sort();
    
        if(_ports != newports)
        {
            QSet<QString> set = QSet<QString>::fromList(_ports);
            QSet<QString> newset = QSet<QString>::fromList(newports);
            newset = newset.subtract(set);

            _latest = QStringList::fromSet(newset);
            _latest.sort();

            _ports = newports;
            _ports.sort();

            emit listChanged();

            qCDebug(pmanager) << "devices:" << _ports;
            qCDebug(pmanager) << "new devices:" << _latest;
        }
    }
    
    
    QStringList PortMonitor::list()
    {
        return _ports;
    }

    QStringList PortMonitor::latest()
    {
        return _latest;
    }
}
