#include <QCoreApplication>
#include <QDebug>

#include "propellermanager.h"
#include "propellerloader.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    PropellerManager manager;
    foreach (QString d, manager.listPorts())
    {
        PropellerLoader loader(&manager, d);
        qDebug() << d << loader.version();
    }

    return 0;
}
