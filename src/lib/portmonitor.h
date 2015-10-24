#pragma once

#include <QTimer>
#include "propellerdevice.h"


class PortMonitor : public QObject
{
    Q_OBJECT

    QTimer timer;
    QStringList ports;

private slots:
    void checkPorts()
    {
        QStringList newports = PropellerDevice::list();

        if(ports != newports)
        {
            emit portChanged(newports);
            ports = newports;
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

signals:
    void portChanged(const QStringList &);

};
