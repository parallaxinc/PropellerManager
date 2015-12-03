#include <QDebug>
#include <QObject>
#include <PropellerManager>

class PortMonitor
    : public QObject
{
    Q_OBJECT

    PropellerManager manager;
    QStringList ports;

private slots:
    void updatePorts()
    {
        ports = manager.listPorts();
        qDebug() << "Ports:" << ports;
    }

public:
    PortMonitor()
        : QObject()
    {
        manager.enablePortMonitor(true);
        updatePorts();

        connect(&manager, SIGNAL(portListChanged()), this, SLOT(updatePorts()));
    }

    ~PortMonitor()
    {
        disconnect(&manager, SIGNAL(portListChanged()), this, SLOT(updatePorts()));
    }

};

