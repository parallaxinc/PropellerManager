#include <QCoreApplication>
#include <QCommandLineParser>
#include <QObject>
#include <QDebug>
#include <QRegularExpression>
#include <QFileInfo>

#include <stdio.h>
#include <stdlib.h>

#include "propellersession.h"
#include "propellerimage.h"
#include "propellerdevice.h"

#ifndef VERSION
#define VERSION "0.0.0"
#endif

PropellerImage load_image(QCommandLineParser &parser);
void open_session(QCommandLineParser &parser, QStringList device_list);
void terminal(PropellerSession & session, QString device);
void info(PropellerImage image);
void list();
void error(const QString & message);

QCommandLineOption argList      (QStringList() << "l" << "list",    QObject::tr("List available devices"));
QCommandLineOption argWrite     (QStringList() << "w" << "write",   QObject::tr("Write program to EEPROM"));
QCommandLineOption argDevice    (QStringList() << "d" << "device",  QObject::tr("Device to program (default: first system device)"), "DEV");
QCommandLineOption argPin       (QStringList() << "p" << "pin",     QObject::tr("Pin for GPIO reset"), "PIN");
QCommandLineOption argTerm      (QStringList() << "t" << "terminal",QObject::tr("Drop into terminal after download"));
QCommandLineOption argIdentify  (QStringList() << "i" << "identify",QObject::tr("Identify device connected at port"));
QCommandLineOption argInfo      (QStringList() << "image",          QObject::tr("Print info about downloadable image"));
QCommandLineOption argClkMode   (QStringList() << "clkmode",        QObject::tr("Change clock mode before download (see Propeller datasheet for supported clock modes)"), "MODE");
QCommandLineOption argClkFreq   (QStringList() << "clkfreq",        QObject::tr("Change clock frequency before download"), "FREQ");
QCommandLineOption argHighSpeed (QStringList() << "ultrafast",      QObject::tr("Enable two-stage high-speed mode (experimental)"));

#if defined(Q_PROCESSOR_ARM_V6) && defined(Q_OS_LINUX)
    int reset_pin = 17;
#else
    int reset_pin = -1;
#endif

QStringList device_list = PropellerDevice::list();

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
            QObject::tr("\nA command-line wrapper for the Propeller Manager API"
                "\nCopyright 2015 by %1").arg(QCoreApplication::organizationName()));

    parser.addOption(argList);
    parser.addOption(argWrite);
    parser.addOption(argDevice);
    parser.addOption(argPin);
    parser.addOption(argTerm);
    parser.addOption(argIdentify);
    parser.addOption(argInfo);
    parser.addOption(argClkMode);
    parser.addOption(argClkFreq);
    parser.addOption(argHighSpeed);

    parser.addPositionalArgument("file",  QObject::tr("Binary file to download"), "FILE");

    parser.process(app);

    if (parser.isSet(argList))
    {
        list();
        return 0;
    }

    if (!parser.value(argPin).isEmpty())
        reset_pin = parser.value(argPin).toInt();

    if (reset_pin > -1)
        qDebug() << "Using GPIO pin" << reset_pin << "for hardware reset";


    if (parser.isSet(argIdentify))
    {
        foreach (QString d, device_list)
        {
            PropellerSession session(d,reset_pin);

            if (!session.open())
                continue;

            switch (session.version())
            {
                case 1:
                    printf("[ %-16s ] %s\n", qPrintable(d), "Propeller P8X32A");
                    break;
                case 0:
                default:
                    break;
            }

            fflush(stdout);
            session.close();
        }
    }
    else if (parser.isSet(argInfo))
    {
        info(load_image(parser));
    }
    else
    {
        open_session(parser, device_list);
    }

    return 0;
}

void open_session(QCommandLineParser &parser, QStringList device_list)
{
    if (device_list.isEmpty())
        error("No device available for download!");

    QString device = device_list[0];
    if (!parser.value(argDevice).isEmpty())
    {
        device = parser.value(argDevice);
        if (!device_list.contains(device))
            error("Device '"+device+"' not available");
    }

    PropellerSession session(device,reset_pin);

    if (parser.isSet(argTerm))
    {
        if (!session.open())
            error("Failed to open "+device+"!");

        terminal(session, device);
    }
    else
    {
        if (parser.positionalArguments().isEmpty())
        {
            error("Must provide name of binary");
        }
        else
        {
            if (!session.open())
                error("Failed to open "+device+"!");

            PropellerImage image = load_image(parser);

            if (parser.isSet(argClkFreq))
            {
                bool ok;
                int freq = parser.value(argClkFreq).toInt(&ok);
                if (!ok)
                    error("Invalid clock frequency: "+parser.value(argClkFreq));

                image.setClockFrequency(freq);
                image.recalculateChecksum();
            }

            if (parser.isSet(argClkMode))
            {
                bool ok;
                int mode = parser.value(argClkMode).toUInt(&ok, 16);
                if (!image.setClockMode(mode) || !ok)
                    error("Clock mode setting "+QString::number(mode, 16)+"is invalid!");
                image.recalculateChecksum();
            }

            if (!image.isValid())
                error("Image is invalid!");

            if (parser.isSet(argHighSpeed))
            {
                if (session.highSpeedUpload(image, parser.isSet(argWrite)))
                    exit(1);
            }
            else
            {
                if (session.upload(image, parser.isSet(argWrite)))
                    exit(1);
            }

            if (parser.isSet(argTerm))
                terminal(session, device);
        }
    }

    session.close();
}

void list()
{
    for (int i = 0; i < device_list.size(); i++)
    {
        printf("%s\n",qPrintable(device_list[i]));
    }
}

void terminal(PropellerSession & session, QString device)
{
    qDebug() << "--------------------------------------";
    qDebug() << "Opening terminal:" << qPrintable(device);
    qDebug() << "  (Ctrl+C to exit)";
    qDebug() << "--------------------------------------";

    session.terminal();
}

void info(PropellerImage image)
{
    printf("           Image: %s\n",qPrintable(image.fileName()));
    printf("            Type: %s\n",qPrintable(image.imageTypeText()));
    printf("            Size: %u\n",image.imageSize());
    printf("        Checksum: %u (%s)\n",image.checksum(), image.checksumIsValid() ? "VALID" : "INVALID");
    printf("    Program size: %u\n",image.programSize());
    printf("   Variable size: %u\n",image.variableSize());
    printf(" Free/stack size: %u\n",image.stackSize());
    printf("      Clock mode: %s\n",qPrintable(image.clockModeText()));

    if (image.clockMode() != 0x00 && image.clockMode() != 0x01)
        printf(" Clock frequency: %i\n",image.clockFrequency());
}

PropellerImage load_image(QCommandLineParser &parser)
{
    QString filename = parser.positionalArguments()[0];
    QRegularExpression re_binary(".*\\.binary$");
    QRegularExpression re_eeprom(".*\\.eeprom$");

    if (!QFileInfo(filename).exists())
        error("File does not exist!");

    if (!filename.contains(re_binary) && !filename.contains(re_eeprom))
        error("Invalid file specified!");

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
        return PropellerImage();

    return PropellerImage(file.readAll(),filename);
}

void error(const QString & message)
{
    qDebug() << "Error:" << qPrintable(message);
    exit(1);
}
