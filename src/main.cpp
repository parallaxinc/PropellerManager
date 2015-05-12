#include <QCoreApplication>
#include <QCommandLineParser>
#include <QObject>
#include <QDebug>
#include <QRegularExpression>
#include <QFileInfo>

#include <stdio.h>
#include <stdlib.h>

#include "utility.h"
#include "propellersession.h"
#include "propellerimage.h"

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

#if defined(Q_PROCESSOR_ARM_V6) && defined(Q_OS_LINUX)
    int reset_pin = 17;
#else
    int reset_pin = -1;
#endif

QStringList device_list = PropellerSession::list_devices();

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
                    printf("[ %-16s ] %-9s\n",qPrintable(d), "NOT FOUND");
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
    if (!session.open())
        error("Failed to open "+device+"!");

    if (parser.positionalArguments().isEmpty())
    {
        if (parser.isSet(argTerm))
            terminal(session, device);
        else
            error("Must provide name of binary");
    }
    else
    {
        PropellerImage image = load_image(parser);
        if (!image.isValid())
            error("Image is invalid!");

        session.upload(image, parser.isSet(argWrite));

        if (parser.isSet(argTerm))
            terminal(session, device);
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
    printf("        Checksum: %u\n",image.checksum());
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

    return PropellerImage(Utility::readFile(filename),filename);
}

void error(const QString & message)
{
    qDebug() << "Error:" << qPrintable(message);
    exit(1);
}
