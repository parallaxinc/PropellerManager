#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include <PropellerManager>
#include <PropellerLoader>
#include <PropellerImage>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    PropellerManager manager;

    QStringList devices = manager.listPorts();

    if (devices.isEmpty())
    {
        qDebug() << "No device available for download!";
        return 1;
    }

    PropellerLoader loader(&manager, devices[0]);

    QFile file("../../test/images/ls/ImAlive.binary");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Couldn't open file";
        return 1;
    }

    PropellerImage image = PropellerImage(file.readAll());
    if (!loader.upload(image))
        return 1;

    return 0;
}
