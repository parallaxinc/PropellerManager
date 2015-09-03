#include <QCoreApplication>
#include <QDebug>

#include <stdio.h>

#include "propellersession.h"
#include "propellerdevice.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QStringList device_list = PropellerDevice::list();
    if (device_list.isEmpty())
        return 1;

    PropellerSession session(device_list[0]);
    if (!session.open())
        return 1;

    qDebug() << "Entering terminal; press CTRL+C to exit";
    session.terminal();

    session.close();

    return 0;
}

