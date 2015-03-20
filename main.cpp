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

// Processor constants
int LFSR_REQUEST_LEN   = 250;
int LFSR_REPLY_LEN     = 250;
int EEPROM_SIZE        = 32768;

void do_nothing(QString msg)
{
    return;
}

QStringList serial_ports()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QStringList result;

    foreach (QSerialPortInfo port, ports)
    {
        result.append(port.systemLocation());
    }

    qDebug() << result;
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

    QCoreApplication::setOrganizationName("Parallax");
    QCoreApplication::setOrganizationDomain("www.parallax.com");
    QCoreApplication::setApplicationVersion(VERSION);
    QCoreApplication::setApplicationName(QObject::tr("propload"));


    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(
            QObject::tr("\nA modern Propeller loader"));


    parser.addPositionalArgument("file",  QObject::tr("Binary file to download"), "FILE");

    parser.process(app);

    QStringList a = serial_ports();

    Loader loader(a[1],-1);
    loader.open();
    loader.close();
    loader.encode_long(3525);
    loader.encode_long(35353325);

    return 0;
}


