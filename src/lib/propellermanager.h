#pragma once

#include "portmonitor.h"

class PropellerManager : public QObject
{
    Q_OBJECT

private:
    PortMonitor portMonitor;
    QStringList _ports;

public:
    PropellerManager(QObject *parent = 0);
    ~PropellerManager();

    const QStringList & ports();

public slots:
    void updatePorts();

signals:
    void portsChanged();

};
