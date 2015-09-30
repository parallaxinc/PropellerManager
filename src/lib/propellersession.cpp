#include "propellersession.h"

#include <QEventLoop>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QElapsedTimer>

#include <stdio.h>

/**
  \param port A string representing the port (e.g. '`/dev/ttyUSB0`', '`/./COM1`').
  \param reset_gpio Enable GPIO reset on the selected pin. The default value of -1 disables GPIO reset.
  \param useRtsReset Use RTS for hardware reset instead of DTR; overridden by reset_gpio.
  */

PropellerSession::PropellerSession( QString port,
                                    int reset_gpio,
                                    bool useRtsReset,
                                    QObject * parent)
    : QObject(parent)
{
    _version = 0;
    _ack     = 0;

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
    device.setBaudRate(115200);

    timeout.setSingleShot(true);
    connect(&timeout, SIGNAL(timeout()), &device, SLOT(timeOver()));

    connect(&device,&PropellerDevice::finished,
            this,   &PropellerSession::finished);

    connect(&device,&PropellerDevice::sendError,
            this,   &PropellerSession::error);

}

PropellerSession::~PropellerSession()
{
    device.close();
}

/**
  Open the PropellerSession for use.
  */

void PropellerSession::message(QString text)
{
    text = "[PropellerManager] "+device.portName()+": "+text;
    printf("%s\n", qPrintable(text));
    fflush(stdout);
}

void PropellerSession::error(QString text)
{
    message("ERROR: "+text);
}

bool PropellerSession::open()
{
    if (!device.open())
    {
        error("Device failed to open: "+device.errorString());
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

void PropellerSession::writeByte(quint8 value)
{
    device.putChar(value);
}

void PropellerSession::writeLong(quint32 value)
{
    device.write(protocol.encodeLong(value));
}


void PropellerSession::read_handshake()
{
//    message(QString("Receiving handshake (%1/%2B)")
//                    .arg(device.bytesAvailable())
//                    .arg(protocol.reply().size() + 4)
//                    );
    if (device.bytesAvailable() == protocol.reply().size() + 4)
    {
        if (device.read(protocol.reply().size()) != protocol.reply())
        {
            error("Handshake invalid");
            emit finished();
        }

        QByteArray versiondata = device.read(4);

        _version = 0;
        for (int i = 0; i < 4; i++)
        {
            _version += (versiondata.at(i) & 1) +
                        ((versiondata.at(i) >> 5) & 1);
        }

        if (!device.bytesToWrite())
        {
            emit finished();
        }

//        message(QString("Handshake received: hardware version %1").arg(_version));
    }
}

/**
  \brief Get the version of the connected device.
  
  \return The version number, or 0 if not found.
  */
int PropellerSession::version()
{
    QByteArray payload = protocol.buildRequest(Command::Shutdown);

    int timeout_payload = device.calculateTimeout(payload.size());
    timeout.start(timeout_payload);

    device.reset();
    device.write(payload);

    connect(&device, SIGNAL(readyRead()), this, SLOT(read_handshake()));

    QEventLoop loop;

    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));


    loop.exec();

    disconnect(&device, SIGNAL(readyRead()), this, SLOT(read_handshake()));

    return _version;
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
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    disconnect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write_terminal(const QString &)));
    disconnect(&device, SIGNAL(readyRead()), this, SLOT(read_terminal()));

    return device.error();
}

/**
  Upload a PropellerImage object to the target.
  */

int PropellerSession::upload(PropellerImage image, bool write, bool run)
{
    device.setBaudRate(115200);

    if (!image.isValid())
    {
        error("Image is invalid");
        return 1;
    }

    int command = 2*write + run;

    QByteArray payload;
    payload.append(protocol.buildRequest((Command::Command) command));
    payload.append(protocol.encodeLong(image.imageSize() / 4));
    payload.append(protocol.encodeData(image.data()));


    int timeout_payload = device.calculateTimeout(payload.size());
    if (write)
        timeout_payload += 5000; // ms (EEPROM write speed is constant.
                                 // the Propeller firmware only does 32kB EEPROMs
                                 // and this transaction is handled entirely by the firmware.

    timeout.start(timeout_payload);
    _elapsedtime.start();

    device.reset();

    message(QString("Downloading image (%1 bytes, %2 ms)").arg(image.imageSize()).arg(timeout_payload));
    if (!sendPayload(payload))
    {
        return 1;
    }

    message("Verifying RAM");
    if (!pollAcknowledge())
    {
        error("Verify RAM Failed");
        return 1;
    }

    if (!write)
    {
        message("DOWNLOAD COMPLETE");
        return 0;
    }
    
    message("Writing to EEPROM");
    if (!pollAcknowledge())
    {
        error("Write EEPROM Failed");
        return 1;
    }

    message("Verifying EEPROM");
    if (!pollAcknowledge())
    {
        error("Verify EEPROM Failed");
        return 1;
    }

    if (run)
    {
        device.reset();
    }

    message("DOWNLOAD COMPLETE");
    return 0;
}


int PropellerSession::highSpeedUpload(PropellerImage image, bool write, bool run)
{
    QFile file("miniloaders/miniloader.binary");

    if (!file.open(QIODevice::ReadOnly))
        return 1;

    PropellerImage loader(file.readAll());

    // SPLIT ALL PACKETS

    /*
    QMap<quint32, QByteArray> pieces;

    int last = 0;
    quint32 lastvalue = 0;
    for (int i = 0; i < loader.imageSize(); i += 4)
    {
        quint32 value = loader.readLong(i);
        if ((value & 0xFFFFFFF0) == 0x11111110)
        {

            if (value != 0x11111110)
            {
                pieces[lastvalue] = loader.data().mid(last, i-last);
                last = i+4;
            }
            else
            {
                pieces[lastvalue] = loader.data().mid(last, i-last);
                last = i;
            }
            lastvalue = value;
        }
    }
    
    if ((loader.imageSize() - last) > 0)
        pieces[lastvalue] = loader.data().mid(last, loader.imageSize() - last);

    foreach (quint32 v, pieces.keys())
        qDebug() << QString::number(v,16) << pieces[v].toHex();


    // GENERATE HOST INITIALIZED VALUES

    quint32 maxRxSenseError = 23; // maximum number of cycles by which the detection of a start bit could be off (SHOULD BE AUTO CALCULATED?!?)

    quint32 clkfreq = image.clockFrequency();

    quint32 initialbaud = 115200;
    quint32 initialperiod = clkfreq / initialbaud;

    quint32 finalbaud = 460800; // 921600
    quint32 finalperiod = clkfreq / finalbaud;

    quint32 errorperiod = ((1.5 * clkfreq) / finalbaud) - maxRxSenseError;  // no idea what this does
    quint32 timeout_failsafe = (2 * clkfreq / (3 * 4));                     //
    quint32 timeout_endofpacket = (2 * finalperiod * 10 / 12);              //

    quint32 total_packet_count = image.imageSize() / (Propeller::_max_data_size - 4); // binary image / (max XBee packet - packet header)
    quint32 packetid = total_packet_count;


    // INJECT HOST VALUES INTO IMAGE AND MOVE SPIN CODE

    PropellerImage l(pieces[0x11111110]);

    l.writeLong(4+0, initialperiod);
    l.writeLong(4+4, finalperiod);
    l.writeLong(4+8, errorperiod);
    l.writeLong(4+12, timeout_failsafe);
    l.writeLong(4+16, timeout_endofpacket);
    l.writeLong(4+20, packetid);

    QByteArray newdata = pieces[0x0];
    QByteArray spincode = pieces[pieces.lastKey()].right(8);
    pieces[pieces.lastKey()].chop(8);

    newdata.append(l.data());
    newdata.append(spincode);

    l.setData(newdata);


    // MAGIC TRANSFORMATION

    int removedbytes = loader.imageSize() - l.imageSize();
    qDebug() << removedbytes;

    for (int i = 8; i <= 14; i += 2)
        l.writeWord(i, l.readWord(i) - removedbytes);

    qDebug() << "SPIN METHODS??" << l.readWord(18) << l.readWord(18)-1;
    for (int i = 0; i <= (l.readWord(18)-1); i++)
    {
        int offset = 16+i*4;
        qDebug() << offset << l.readWord(offset);
        l.writeWord(offset, l.readWord(offset) - removedbytes);
        qDebug() << offset << l.readWord(offset);
    }



    // finalize loader image

    loader.setData(newdata);
    loader.recalculateChecksum();

    qDebug() << loader.checksumIsValid();
    qDebug() << loader.isValid();


    qDebug() << loader.data().size() << loader.data().toHex();

    QByteArray dataz = loader.data();
    QByteArray pretty = dataz.toHex().toUpper();
    for (int i = 0 ; i < pretty.size() ; i++)
    {
        printf("%c",(unsigned char) pretty[i]);
        if (i % 2 == 1)
            printf("\n");
    }

    */

    // GENERATE HOST INITIALIZED VALUES

    quint32 maxRxSenseError = 23; // maximum number of cycles by which the detection of a start bit could be off (SHOULD BE AUTO CALCULATED?!?)

    quint32 clkfreq = image.clockFrequency();

    quint32 initialbaud = 115200;
    quint32 initialperiod = clkfreq / initialbaud;

    quint32 finalbaud = 460800; // 921600
    quint32 finalperiod = clkfreq / finalbaud;

    quint32 errorperiod = ((1.5 * clkfreq) / finalbaud) - maxRxSenseError;  // no idea what this does
    quint32 timeout_failsafe = (2 * clkfreq / (3 * 4));                     //
    quint32 timeout_endofpacket = (2 * finalperiod * 10 / 12);              //

    int total_packet_count = image.imageSize() / (Propeller::_max_data_size - 4) + 1; // binary image / (max XBee packet - packet header)
    quint32 packetid = total_packet_count;


    const quint8 rawLoaderImage[348] = {
        0x00,0xB4,0xC4,0x04,0x6F,0xC3,0x10,0x00,0x5C,0x01,0x64,0x01,0x54,0x01,0x68,0x01,
        0x4C,0x01,0x02,0x00,0x44,0x01,0x00,0x00,0x48,0xE8,0xBF,0xA0,0x48,0xEC,0xBF,0xA0,
        0x49,0xA0,0xBC,0xA1,0x01,0xA0,0xFC,0x28,0xF1,0xA1,0xBC,0x80,0xA0,0x9E,0xCC,0xA0,
        0x49,0xA0,0xBC,0xF8,0xF2,0x8F,0x3C,0x61,0x05,0x9E,0xFC,0xE4,0x04,0xA4,0xFC,0xA0,
        0x4E,0xA2,0xBC,0xA0,0x08,0x9C,0xFC,0x20,0xFF,0xA2,0xFC,0x60,0x00,0xA3,0xFC,0x68,
        0x01,0xA2,0xFC,0x2C,0x49,0xA0,0xBC,0xA0,0xF1,0xA1,0xBC,0x80,0x01,0xA2,0xFC,0x29,
        0x49,0xA0,0xBC,0xF8,0x48,0xE8,0xBF,0x70,0x11,0xA2,0x7C,0xE8,0x0A,0xA4,0xFC,0xE4,
        0x4A,0x92,0xBC,0xA0,0x4C,0x3C,0xFC,0x50,0x54,0xA6,0xFC,0xA0,0x53,0x3A,0xBC,0x54,
        0x53,0x56,0xBC,0x54,0x53,0x58,0xBC,0x54,0x04,0xA4,0xFC,0xA0,0x00,0xA8,0xFC,0xA0,
        0x4C,0x9E,0xBC,0xA0,0x4B,0xA0,0xBC,0xA1,0x00,0xA2,0xFC,0xA0,0x80,0xA2,0xFC,0x72,
        0xF2,0x8F,0x3C,0x61,0x21,0x9E,0xF8,0xE4,0x31,0x00,0x78,0x5C,0xF1,0xA1,0xBC,0x80,
        0x49,0xA0,0xBC,0xF8,0xF2,0x8F,0x3C,0x61,0x00,0xA3,0xFC,0x70,0x01,0xA2,0xFC,0x29,
        0x26,0x00,0x4C,0x5C,0x51,0xA8,0xBC,0x68,0x08,0xA8,0xFC,0x20,0x4D,0x3C,0xFC,0x50,
        0x1E,0xA4,0xFC,0xE4,0x01,0xA6,0xFC,0x80,0x19,0x00,0x7C,0x5C,0x1E,0x9E,0xBC,0xA0,
        0xFF,0x9F,0xFC,0x60,0x4C,0x9E,0x7C,0x86,0x00,0x84,0x68,0x0C,0x4E,0xA8,0x3C,0xC2,
        0x09,0x00,0x54,0x5C,0x01,0x9C,0xFC,0xC1,0x55,0x00,0x70,0x5C,0x55,0xA6,0xFC,0x84,
        0x40,0xAA,0x3C,0x08,0x04,0x80,0xFC,0x80,0x43,0x74,0xBC,0x80,0x3A,0xA6,0xFC,0xE4,
        0x55,0x74,0xFC,0x54,0x09,0x00,0x7C,0x5C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x80,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x80,0x00,0x00,0xFF,0xFF,0xF9,0xFF,
        0x10,0xC0,0x07,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x40,0xB6,0x02,0x00,0x00,
        0x5B,0x01,0x00,0x00,0x08,0x02,0x00,0x00,0x55,0x73,0xCB,0x00,0x50,0x45,0x01,0x00,
        0x00,0x00,0x00,0x00,0x35,0xC7,0x08,0x35,0x2C,0x32,0x00,0x00
    };

    QByteArray _loader((char*) rawLoaderImage, 348);

    const int offset = 348 - 8*4; // offset from end

    loader.setData(_loader);

    loader.writeLong(offset+0, initialperiod);
    loader.writeLong(offset+4, finalperiod);
    loader.writeLong(offset+8, errorperiod);
    loader.writeLong(offset+12, timeout_failsafe);
    loader.writeLong(offset+16, timeout_endofpacket);
    loader.writeLong(offset+20, packetid);

    loader.recalculateChecksum();

    upload(loader, write, run);



    device.setBaudRate(finalbaud);

    QByteArray data = image.data();
    qDebug() << data.size();

    int maxsize = Propeller::_max_data_size - 4;


    QThread::msleep(40);
    qDebug() << "DOWNLOADING";
    for (int i = total_packet_count ; i > 0 ; i--)
    {
        qDebug() << "ASSEMBLE";
        QByteArray payload;
        payload.append(protocol.packLong(i));
        payload.append(data.left(maxsize));

        qDebug() << payload.size();

        sendPayload(payload);
        qDebug() << "RETURN FROM PAYHLOAD";
    }


//    upload(image, write, run);
//
    return 0;
}


/**
    \brief Transmit a complete data payload on this device.
    
    sendPayload will exit even when all of the data hasn't been written to the Propeller yet,
    which is why the timeout period only accounts for the length of the handshake.

    This function makes two attempts to receive the handshake and then gives up.
  */

bool PropellerSession::sendPayload(QByteArray payload)
{
    connect(&device, SIGNAL(bytesWritten(qint64)), &device, SLOT(writeBufferEmpty()));
    connect(&device, SIGNAL(readyRead()), this, SLOT(read_handshake()));

    device.write(payload);

    QEventLoop loop;
    connect(this,    SIGNAL(finished()), &loop, SLOT(quit()));


    loop.exec();

    if (!_version)
    {
        // if handshake not received on first finish, try again
        loop.exec();

        if (!_version)
        {
            error("Device not found");
            return false;
        }
    } 

    disconnect(&device, SIGNAL(readyRead()), this, SLOT(read_handshake()));
    disconnect(&device, SIGNAL(bytesWritten(qint64)), &device, SLOT(writeBufferEmpty()));

    if (timeout.remainingTime() <= 0)
    {
        error(QString("Download timed out after %1 ms").arg(_elapsedtime.elapsed()));
        return false;
    }
    else if (device.error())
    {
        return false;
    }
    else
    {
        printTime();
        return true;
    }
}

void PropellerSession::printTime()
{
    message(QString("%1... %2 ms elapsed")
            .arg(QString(40,QChar(' ')))
            .arg(_elapsedtime.elapsed()));
}

int PropellerSession::pollAcknowledge()
{
    connect(&device,    SIGNAL(readyRead()),this,   SLOT(read_acknowledge()));
    connect(&poll,      SIGNAL(timeout()),  this,   SLOT(calibrate()));

    poll.setInterval(20);
    poll.start();

    QEventLoop loop;
    connect(this,       SIGNAL(finished()), &loop,  SLOT(quit()));

    loop.exec();

    disconnect(&poll);
    disconnect(&poll,   SIGNAL(timeout()),  this,   SLOT(calibrate()));
    disconnect(&device, SIGNAL(readyRead()),this,   SLOT(read_acknowledge()));


    if (timeout.remainingTime() <= 0)
    {
        error(QString("Download timed out after %1 ms").arg(_elapsedtime.elapsed()));
        return false;
    }
    else if (device.error())
    {
        return false;
    }
    else
    {   
        printTime();
        return true;
    }
}

void PropellerSession::read_acknowledge()
{
    if (device.bytesAvailable())
    {
        _ack = QString(device.readAll().data()).toInt();

        if (_ack)
            message(QString("Invalid ACK: %1").arg(_ack));
        poll.stop();
        emit finished();
    }
}

