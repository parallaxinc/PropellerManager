#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include <PropellerLoader>
#include <PropellerImage>
#include <PropellerManager>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    PropellerManager manager;
    QStringList device_list = manager.listPorts();
    if (device_list.isEmpty())
    {
        qDebug() << "No device available for download!";
        return 1;
    }

    PropellerLoader loader(&manager, device_list[0]);

    QString filename = "../../test/images/ls/ImAlive.binary";
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Couldn't open file";
        return 1;
    }

    PropellerImage image = PropellerImage(file.readAll(),filename);
    loader.upload(image);

    return 0;
}
