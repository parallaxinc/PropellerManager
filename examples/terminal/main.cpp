#include <QCoreApplication>
#include <QDebug>

#include <PropellerTerminal>
#include <PropellerManager>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    PropellerManager manager;

    QStringList device_list = manager.listPorts();
    if (device_list.isEmpty())
        return 1;

    PropellerTerminal terminal(&manager, device_list[0]);
    return 0;
}

