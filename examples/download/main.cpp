#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include "propellersession.h"
#include "propellerimage.h"
#include "propellerdevice.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QStringList device_list = PropellerDevice::list();
    if (device_list.isEmpty())
        qDebug() << "No device available for download!";

    PropellerSession session(device_list[0]);

    if (!session.open())
        qDebug() << "Failed to open" << device_list[0] << "!";

    QString filename = "../../test/images/ls/ImAlive.binary";

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        qDebug() << "Couldn't open file";

    PropellerImage image = PropellerImage(file.readAll(),filename);

    session.upload(image);
    session.close();

    return 0;
}
