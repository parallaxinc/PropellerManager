#include <QCoreApplication>
#include <QCommandLineParser>
#include <QObject>
#include <QDebug>
#include <QRegularExpression>

#include "utility.h"
#include "propellerdevice.h"
#include "propellerimage.h"

#ifndef VERSION
#define VERSION "0.0.0"
#endif

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

    QCommandLineOption argList(     QStringList() << "l" << "list",  QObject::tr("List available devices"));
    QCommandLineOption argWrite(    QStringList() << "w" << "write", QObject::tr("Write program to EEPROM"));
    QCommandLineOption argDevice(   QStringList() << "d" << "device",QObject::tr("Device to program (default: first system device)"), "DEV");
    QCommandLineOption argPin(      QStringList() << "p" << "pin",   QObject::tr("Pin for GPIO reset"), "PIN");
    QCommandLineOption argTerm(     QStringList() << "t" << "terminal",   QObject::tr("Drop into terminal after download"));
    QCommandLineOption argIdentify( QStringList() << "i" << "identify",   QObject::tr("Identify device connected at port"));

    parser.addOption(argList);
    parser.addOption(argWrite);
    parser.addOption(argDevice);
    parser.addOption(argPin);
    parser.addOption(argTerm);
    parser.addOption(argIdentify);

    parser.addPositionalArgument("file",  QObject::tr("Binary file to download"), "FILE");

    parser.process(app);

    QStringList device_list = PropellerDevice::list_devices();
    if (parser.isSet(argList))
    {
        for (int i = 0; i < device_list.size(); i++)
        {
            qDebug() << qPrintable(device_list[i]);
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

#if defined(Q_PROCESSOR_ARM_V6) && defined(Q_OS_LINUX)
    int reset_pin = 17;
#else
    int reset_pin = -1;
#endif

    if (!parser.value(argPin).isEmpty())
    {
        reset_pin = parser.value(argPin).toInt();
    }
    if (reset_pin > -1)
        qDebug() << "Using GPIO pin" << reset_pin << "for hardware reset";


    if (parser.isSet(argIdentify))
    {
        foreach (QString d, device_list)
        {
            PropellerDevice loader(d,reset_pin);
            loader.open();

            switch (loader.get_version())
            {
                case 1:
                    qDebug("[ %-16s ] %s", qPrintable(d), "Propeller P8X32A");
                    break;
                case 0:
                default:
                    qDebug("[ %-16s ] %-9s",qPrintable(d), "NOT FOUND");
                    break;
            }

            loader.close();
        }
    }
    else
    {
        PropellerDevice loader(device,reset_pin);
        if (loader.open())
            return 1;

        if (parser.positionalArguments().isEmpty())
        {
            qDebug() << "Error: Must provide name of binary";
            return 1;
        }

        QRegularExpression re_binary(".*\\.binary$");
        QRegularExpression re_eeprom(".*\\.eeprom$");

        if (parser.positionalArguments()[0].contains(re_binary))
        {
            PropellerImage image(Utility::readFile(parser.positionalArguments()[0]),PropellerImage::Binary);
            qDebug() << image.imageSize() << image.programSize();
            qDebug() << image.clockFrequency() << image.clockModeText();
            loader.upload_binary(image, parser.isSet(argWrite));
        }
        else if (parser.positionalArguments()[0].contains(re_eeprom))
        {
            PropellerImage image(Utility::readFile(parser.positionalArguments()[0]),PropellerImage::Eeprom);
            qDebug() << image.imageSize() << image.programSize();
            qDebug() << image.clockFrequency() << image.clockModeText();
            loader.upload_binary(image, parser.isSet(argWrite));
        }
        else
        {
            qDebug() << "Invalid file specified";
        }

        if (parser.isSet(argTerm))
            loader.terminal();

        loader.close();
    }

    return 0;
}


