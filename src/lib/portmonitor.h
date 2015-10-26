#pragma once

#include <QTimer>
#include "propellerdevice.h"


class PortMonitor : public QObject
{
    Q_OBJECT

    QTimer timer;
    QStringList _ports;

private slots:
    void checkPorts()
    {
        QStringList newports = PropellerDevice::list();

        if(_ports != newports)
        {
            _ports = newports;
            emit portsChanged();
        }
    }

public:
    explicit PortMonitor(QObject *parent = 0)
        : QObject(parent)
    {
        connect(&timer, SIGNAL(timeout()), this, SLOT(checkPorts()));
        timer.start(200);
    }

    ~PortMonitor()
    {
        timer.stop();
        disconnect(&timer, SIGNAL(timeout()), this, SLOT(checkPorts()));
    }

    const QStringList & ports()
    {
        return _ports;
    }

signals:
    void portsChanged();

};
