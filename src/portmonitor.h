#pragma once

#include <QTimer>
#include <QStringList>

namespace PM
{
    class PortMonitor : public QObject
    {
        Q_OBJECT
    
        QStringList _ports;
        QTimer timer;
    
    private slots:
        void check();
    
    public:
        PortMonitor(QObject *parent = 0);
        ~PortMonitor();
    
        QStringList list();
        void toggle(bool enabled, int timeout = 200);
    
    signals:
        void listChanged();
    };
}
