#include "propellersession.h"
#include "propellerprotocol.h"
#include "utility.h"

#include <QEventLoop>
#include <QDebug>

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

void PropellerSession::writeByte(quint8 value)
{
    device.putChar(value);
}

void PropellerSession::writeLong(quint32 value)
{
    device.write(PropellerProtocol::encodeLong(value));
}


void PropellerSession::read_handshake()
{
//    qDebug() << "BYTES AVAILABLE" << device.bytesAvailable() << reply.size();
    if (device.bytesAvailable() == reply.size() + 4)
    {
        if (device.read(reply.size()) != reply)
            emit finished();

        QByteArray versiondata = device.read(4);

        _version = 0;
        for (int i = 0; i < 4; i++)
        {
            _version += (versiondata.at(i) & 1) +
                        ((versiondata.at(i) >> 5) & 1);
        }

        emit finished();
//        qDebug() << "VERSION" << _version;
    }
}

QByteArray PropellerSession::buildRequest(Command::Command command)
{
    QByteArray array = request;
    array.append(QByteArray(125, 0x29));
    array.append(QByteArray(4, 0x29));
    array.append(PropellerProtocol::encodeLong(command));
    return array;
}

/**
  \brief Get the version of the connected device.
  
  \return The version number, or 0 if not found.
  */
int PropellerSession::version()
{
    device.reset();
    device.write(buildRequest(Command::Shutdown));

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

void PropellerSession::upload(PropellerImage image, bool write, bool run)
{

    if (!image.isValid())
        return;

    int command = 2*write + run;

    QByteArray payload;
    payload.append(buildRequest((Command::Command) command));
    payload.append(PropellerProtocol::encodeLong(image.imageSize() / 4));
    payload.append(PropellerProtocol::encodeData(image.data()));

    device.setBaudRate(115200);
    device.reset();

    if (!sendPayload(payload))
        return;

    if (!pollAcknowledge())
        return;

    if (!write)
        return;
    
    if (!pollAcknowledge())
        return;

    if (!pollAcknowledge())
        return;

    if (run)
        device.reset();
}


struct LoaderPacket {
    QByteArray data;
    quint32 id;
};


void PropellerSession::highSpeedUpload(PropellerImage image, bool write, bool run)
{
    PropellerImage loader(Utility::readFile("miniloader.binary"));


    QList<LoaderPacket> loadercomponents;

    int last = 0;
    quint32 lastvalue = 0;
    for (int i = 0; i < loader.imageSize(); i += 4)
    {
        quint32 value = loader.readLong(i);
        if ((value & 0xFFFFFFF0) == 0x11111110)
        {
            LoaderPacket p;
            p.id = lastvalue;
            p.data = loader.data().mid(last, i-last);

            loadercomponents.append(p);

            last = i+4;
            lastvalue = value;
        }
    }
    
    if ((loader.imageSize() - last) > 0)
    {
        LoaderPacket p;
        p.id = lastvalue;
        p.data = loader.data().mid(last, loader.imageSize() - last);
        loadercomponents.append(p);
    }

    foreach (LoaderPacket a, loadercomponents)
    {
        qDebug() << QString::number(a.id, 16) << a.data.toHex();
//        if (a.id == 0x11111110)
//            qDebug() << a.data.toHex();
    }


    quint32 initialbaud = 115200;
    quint32 finalbaud = 460800; // 921600

//    QByteArray loaderCore, loaderVerify

//    int last = 0, i = 0;
//    while (last == 0 && i < loader.imageSize())
//    {
//        if (loader.readLong(i) == 0x11111110)
//            last = i+4;
//        i += 4;
//    }

    int hostvalues = last;




//    SetHostInitializedValue(RawSize*4+RawLoaderInitOffset, Round(80000000 / InitialBaud));                        {Initial Bit Time}
//    SetHostInitializedValue(RawSize*4+RawLoaderInitOffset + 4, Round(80000000 / FinalBaud));                      {Final Bit Time}
//    SetHostInitializedValue(RawSize*4+RawLoaderInitOffset + 8, Round(((1.5 * 80000000) / FinalBaud) - MaxRxSenseError));  {1.5x Final Bit Time minus maximum start bit sense error}
//    SetHostInitializedValue(RawSize*4+RawLoaderInitOffset + 12, 2 * 80000000 div (3 * 4));                        {Failsafe Timeout (seconds-worth of Loader's Receive loop iterations)}
//    SetHostInitializedValue(RawSize*4+RawLoaderInitOffset + 16, Round(2 * 80000000 / FinalBaud * 10 / 12));       {EndOfPacket Timeout (2 bytes worth of Loader's Receive loop iterations)}
//    SetHostInitializedValue(RawSize*4+RawLoaderInitOffset + 20, PacketID);                                        {First Expected Packet ID; total packet count}


    upload(loader, write, run);

    upload(image, write, run);
}

void PropellerSession::read_acknowledge()
{
//    qDebug() << "GOT ACK" << device.bytesAvailable();
    if (device.bytesAvailable())
    {
        ack = QString(device.readAll().data()).toInt();
//        qDebug() << "ACK" << ack;
        poll.stop();
        emit finished();
    }
}

bool PropellerSession::sendPayload(QByteArray payload)
{
    connect(&device, SIGNAL(bytesWritten(qint64)), &device, SLOT(writeBufferEmpty()));
    connect(&device, SIGNAL(readyRead()), this, SLOT(read_handshake()));

    device.write(payload);

    QEventLoop loop;
    connect(this,    SIGNAL(finished()), &loop, SLOT(quit()));
    connect(&device, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    disconnect(&device, SIGNAL(readyRead()), this, SLOT(read_handshake()));
    disconnect(&device, SIGNAL(bytesWritten(qint64)), &device, SLOT(writeBufferEmpty()));

    if (device.error())
        return false;
    else
        return true;
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

    if (device.error())
        return false;
    else
        return true;
}
