#include "propellersession.h"
#include "utility.h"
#include "algorithms.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QDebug>
#include <QThread>

#include <bitset>

/**
  \param port A string representing the port (e.g. '`/dev/ttyUSB0`', '`/./COM1`').
  \param reset_gpio Enable GPIO reset on the selected pin. The default value of -1 disables GPIO reset.
  \param useRtsReset Use RTS for hardware reset instead of DTR; overridden by reset_gpio.
  */

PropellerSession::PropellerSession(   QString port,
                                    int reset_gpio,
                                    bool useRtsReset,
                                    QObject * parent)
    : QObject(parent)
{
    _version = 0;

    if (reset_gpio > -1)
    {
        device.useGpioReset(reset_gpio);
    }
    else
    {
        if (useRtsReset)
            device.useRtsReset();
    }

    device.setPortName(port);

    connect(&device,&PropellerDevice::finished,
            this,   &PropellerSession::finished);
    connect(&device,&PropellerDevice::sendError,
            this,   &Utility::print_error);

    request = QByteArray((char*) Propeller::request, Propeller::request_size);
    reply = QByteArray((char*) Propeller::reply, Propeller::reply_size);

}

PropellerSession::~PropellerSession()
{
    device.close();
}

/**
  Open the PropellerSession for use.
  */

bool PropellerSession::open()
{
    if (!device.open())
    {
        qDebug() << "Failed to open" << device.portName()
            << ":" << device.errorString();
        return false;
    }
    return true;
}

/**
  Return whether the PropellerSession is now open.
  */

bool PropellerSession::isOpen()
{
    return device.isOpen();
}

/**
  Close the PropellerSession; this function is called when the PropellerSession is destroyed.
  */

void PropellerSession::close()
{
    device.close();
}

/**
  This function sends a reset to the connected device using
  whatever method is available.

  Methods supported:

  - Serial
    - Data Terminal Ready (DTR)
    - Request To Send (RTS)
    - GPIO (Linux only)

  - Wireless
    - TBD
  */

void PropellerSession::calibrate()
{
    writeByte(0xf9);
}

void PropellerSession::writeByte(char value)
{
    device.putChar(value);
}

void PropellerSession::writeLong(unsigned int value)
{
    device.write(encodeLong(value));
}


void PropellerSession::read_handshake()
{
//    qDebug() << "BYTES AVAILABLE" << device.bytesAvailable();
    if (device.bytesAvailable() == reply.size() + 4)
    {
        real_reply = device.read(reply.size());
        if (real_reply != reply)
        {
            emit finished();
        }

        QByteArray versiondata = device.read(4);

        _version = 0;
        for (int i = 0; i < 4; i++)
            _version = ((_version >> 1) & 0x7f) | ((versiondata.at(i) & 0x1) << 7);

        emit finished();
    }
}

/**
  \brief Get the version of the connected device.
  
  \return The version number, or 0 if not found.
  */

int PropellerSession::version()
{
    handshake();
    return _version;
}

QByteArray PropellerSession::encodeLong(unsigned int value)
{
// //   qDebug () << "LONG" << QString::number(value,32);
//    int thing = value;
//    QByteArray byteArray;
//    QDataStream stream(&byteArray, QIODevice::WriteOnly);
//    stream.setByteOrder(QDataStream::LittleEndian);
//    stream << thing;
//
//    byteArray = encodeData(byteArray);
////    qDebug() << "CONV" << byteArray.toHex();
//    return byteArray;


    QByteArray result;
    for (int i = 0; i < 10; i++)
    {
        result.append(0x92 | (value & 0x01) | ((value & 2) << 2) | ((value & 4) << 4));
        value >>= 3;
    }
    result.append(0xf2 | (value & 0x01) | ((value & 2) << 2));
    qDebug () << "LONG" << result.toHex();

    return result;
}

int PropellerSession::lfsr(int * seed)
{
    char ret = *seed & 0x01;
    *seed = ((*seed << 1) & 0xfe) | (((*seed >> 7) ^ (*seed >> 5) ^ (*seed >> 4) ^ (*seed >> 1)) & 1);
    return ret;
}

QList<char> PropellerSession::buildLfsrSequence(int size)
{
    int seed = 'P';
    
    QList<char> seq;
    for (int i = 0; i < size; i++)
    {
        seq.append(lfsr(&seed));
    }

    return seq;
}

QByteArray PropellerSession::buildRequest(Command::Command command)
{
    QByteArray array = request;
    array.append(QByteArray(125, 0x29));
    array.append(QByteArray(4, 0x29));
    array.append(encodeLong(command));
    return array;
}

int PropellerSession::handshake(Command::Command command)
{
    real_reply.clear();

    device.reset();
    device.write(buildRequest(command));

    connect(&device, SIGNAL(readyRead()), this, SLOT(read_handshake()));
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &device, SLOT(timeOver()));
    timer.start(500);
    loop.exec();

    disconnect(&device, SIGNAL(readyRead()), this, SLOT(read_handshake()));

    return _version;
}

QByteArray PropellerSession::encodeApplicationImage(PropellerImage image)
{
    QByteArray encoded_image;

    encoded_image = encodeData(image.data());

    for (int i = 0; i < encoded_image.size(); i++)
    {
        if (i % 16 == 0)
            printf("\n");
        printf("%02X,",(unsigned char) encoded_image.at(i));
    }

//    qDebug() << encoded_image.toHex();

//    for (int i = 0 ; i < image.imageSize() ; i += 4)
//        encoded_image.append(encodeLong(image.readLong(i)));
//

//    const quint8 imalive[177] = {
//        0x92,0x92,0x52,0x2A,0x52,0xA9,0xCA,0x92,0x92,0x92,0x92,0x92,0xD2,0xAA,0xC9,0xD2,
//        0x92,0x92,0x92,0x92,0xD2,0xD2,0x92,0x92,0x92,0xD2,0x4A,0x92,0x92,0x92,0x52,0xC9,
//        0x92,0x92,0x92,0x92,0xA5,0x92,0x92,0x92,0x52,0xCA,0x92,0x92,0x92,0xCA,0x92,0xD2,
//        0x92,0x92,0x92,0x25,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x2A,0xC9,0x92,
//        0x92,0x92,0xCA,0x92,0x92,0x92,0x92,0x92,0x95,0x92,0x2A,0xD2,0xAA,0xC9,0x2A,0xA9,
//        0x52,0xC9,0x92,0xAA,0x92,0x52,0x92,0xA9,0x25,0xD2,0x4A,0xC9,0x2A,0xD2,0x52,0x49,
//        0xC9,0x92,0x92,0x4A,0x25,0xCA,0x92,0xCA,0x92,0xD2,0xD2,0x92,0x52,0xD5,0x92,0xAA,
//        0x92,0x52,0x92,0xA9,0x25,0xD2,0x4A,0x49,0x25,0xCA,0xAA,0x52,0x92,0x92,0x92,0xA9,
//        0xC9,0xCA,0x92,0xCA,0x92,0xD2,0xD2,0x92,0x52,0xD5,0x92,0xC9,0x92,0x2A,0x2A,0xCA,
//        0x2A,0x92,0x25,0x92,0x92,0x92,0xD2,0x92,0x92,0x92,0x92,0x92,0xD2,0x92,0x92,0x92,
//        0x92,0x92,0x92,0x92,0x92,0x92,0xD2,0x52,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,
//        0xF2
//    };
//    encoded_image = QByteArray((char*) imalive, 177);

    return encoded_image;
}

void PropellerSession::write_terminal(const QString & text)
{
    device.write(qPrintable(text));
    device.write("\r");
}

void PropellerSession::read_terminal()
{
    foreach (char c, device.readAll())
    {
        switch (c)
        {
            case 10:
            case 13:
                fprintf(stdout,"\n");
                break;
            default:
                fprintf(stdout,"%c",c);
        }
    }
    fflush(stdout);
}

/**
  Open a device terminal on this device.
  */

int PropellerSession::terminal()
{
    device.setBaudRate(115200);

    connect(&device, SIGNAL(readyRead()), this, SLOT(read_terminal()));
    connect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write_terminal(const QString &)));

    QEventLoop loop;
    connect(&device, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    disconnect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write_terminal(const QString &)));
    disconnect(&device, SIGNAL(readyRead()), this, SLOT(read_terminal()));

    return device.error();
}

/**
  Upload a PropellerImage object to the target.
  */

void PropellerSession::upload(PropellerImage binary, bool write, bool run)
{
    if (!binary.isValid())
        return;

    QByteArray encoded_image = encodeApplicationImage(binary);

    Utility::print_task("Connecting to '"+device.portName()+"'...");

    int command = 2*write + run;
    qDebug() << "COMMAND" << command;
    if (!handshake((Command::Command) command))
    {
        Utility::print_status("NOT FOUND");
        return;
    }
    Utility::print_status("DONE");

    Utility::print_task("Downloading to RAM...");
    if (sendApplicationImage(encoded_image, binary.imageSize()) != 0)
        return;

    Utility::print_status("DONE");

    Utility::print_task("Verifying RAM...");
    if (pollAcknowledge() != 0)
    {
        Utility::print_status("RAM CHECKSUM INVALID");
        return;
    }

    Utility::print_status("DONE");

    if (!write)
        return;
    
    Utility::print_task("Writing EEPROM...");

    if (pollAcknowledge() != 0)
    {
        Utility::print_status("FAIL");
        return;
    }
    Utility::print_status("DONE");

    Utility::print_task("Verifying EEPROM...");

    if (pollAcknowledge() != 0)
    {
        Utility::print_status("FAIL");
        return;
    }
    Utility::print_status("DONE");

    if (run)
        device.reset();
}

void PropellerSession::read_acknowledge()
{
    qDebug() << "GOT ACK" << device.bytesAvailable();
    if (device.bytesAvailable())
    {
        ack = QString(device.readAll().data()).toInt();
        qDebug() << "ACK" << ack;
        poll.stop();
        emit finished();
    }
}

int PropellerSession::sendApplicationImage(QByteArray encoded_image, int image_size)
{
    connect(&device, SIGNAL(bytesWritten(qint64)), &device, SLOT(writeBufferEmpty()));

    writeLong(image_size / 4);
    device.write(encoded_image);

    QEventLoop loop;
    connect(&device, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    disconnect(&device, SIGNAL(bytesWritten(qint64)), &device, SLOT(writeBufferEmpty()));

    return device.error();
}

int PropellerSession::pollAcknowledge()
{
    connect(&device,    SIGNAL(readyRead()),this,   SLOT(read_acknowledge()));
    connect(&poll,      SIGNAL(timeout()),  this,   SLOT(calibrate()));

    poll.setInterval(20);
    poll.start();

    QEventLoop loop;
    connect(this,       SIGNAL(finished()), &loop,  SLOT(quit()));

    QTimer timeout;
    timeout.setSingleShot(true);
    connect(&timeout,   SIGNAL(timeout()),  &loop,  SLOT(quit()));
    connect(&timeout,   SIGNAL(timeout()),  &device,SLOT(timeOver()));
    timeout.start(5000);

    loop.exec();

    disconnect(&poll);
    disconnect(&poll,   SIGNAL(timeout()),  this,   SLOT(calibrate()));
    disconnect(&device, SIGNAL(readyRead()),this,   SLOT(read_acknowledge()));

    return device.error();
}
