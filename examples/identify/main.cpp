#include <QCoreApplication>
#include <QDebug>

#include "propellersession.h"
#include "propellerdevice.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QStringList device_list = PropellerDevice::list();

    foreach (QString d, device_list)
    {
        PropellerSession session(d);

        if (!session.open())
            continue;

        qDebug() << d << session.version();

        session.close();
    }

    return 0;
}
