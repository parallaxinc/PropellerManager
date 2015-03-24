#include <QCoreApplication>
#include <QCommandLineParser>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMap>
#include <QFile>
#include <QByteArray>

#include "Loader.h"

#ifndef VERSION
#define VERSION "0.0.0"
#endif

QStringList serial_ports()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QStringList result;

    foreach (QSerialPortInfo port, ports)
    {
        result.append(port.systemLocation());
    }
    return result;
}

QByteArray readFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return QByteArray();
    QByteArray blob = file.readAll();
    return blob;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Parallax Inc.");
    QCoreApplication::setOrganizationDomain("www.parallax.com");
    QCoreApplication::setApplicationVersion(VERSION);
    QCoreApplication::setApplicationName(QObject::tr("Propeller Manager CLI"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(
            QObject::tr("\nA command-line wrapper to the Propeller Manager API"
                "\nCopyright 2015 by %1").arg(QCoreApplication::organizationName()));

    QCommandLineOption argList(  QStringList() << "l" << "list",  QObject::tr("List available devices"));
    QCommandLineOption argWrite( QStringList() << "w" << "write", QObject::tr("Write program to EEPROM"));
    QCommandLineOption argDevice(QStringList() << "d" << "device",QObject::tr("Device to program (default: first system device)"), "DEV");
    QCommandLineOption argPin(   QStringList() << "p" << "pin",   QObject::tr("Pin for GPIO reset"), "PIN");

    parser.addOption(argList);
    parser.addOption(argWrite);
    parser.addOption(argDevice);
    parser.addOption(argPin);

    parser.addPositionalArgument("file",  QObject::tr("Binary file to download"), "FILE");

    parser.process(app);

    QStringList device_list = serial_ports();
    if (parser.isSet(argList))
    {
        for (int i = 0; i < device_list.size(); i++)
        {
            qDebug() << device_list[i];
        }
        return 0;
    }

    if (device_list.isEmpty())
    {
        qDebug() << "No devices available for download!";
        return 1;
    }

    QString device = device_list[0];
    if (!parser.value(argDevice).isEmpty())
    {
        device = parser.value(argDevice);
        if (!device_list.contains(device))
        {
            qDebug() << "Device name not available";
            return 1;
        }
    }

    int reset_pin = -1;
    if (!parser.value(argPin).isEmpty())
    {
        reset_pin = parser.value(argPin).toInt();
        qDebug() << "Using GPIO pin" << reset_pin << "for hardware reset";
    }

    qDebug() << "Selecting" << device;

    if (parser.positionalArguments().isEmpty())
    {
        qDebug() << "Error: Must provide name of binary";
        return 1;
    }

    Loader loader(device_list[0],reset_pin);
    loader.open();
//    loader.get_version();
    loader.upload_binary(readFile(parser.positionalArguments()[0]),parser.isSet(argWrite));
    loader.close();

    return 0;
}


