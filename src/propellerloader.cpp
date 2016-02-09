#include "propellerloader.h"

#include <QEventLoop>
#include <QDebug>
#include <QFile>
#include <QElapsedTimer>

#include "logging.h"

PropellerLoader::PropellerLoader(PropellerManager * manager, const QString & portname,
                                 QObject * parent)
    : QObject(parent)
{
    _versionstrings[0] = tr("");
    _versionstrings[1] = tr("Propeller P8X32A");

    _error = NoError;
    _errorstrings[NoError]                  = tr("No Error");
    _errorstrings[DeviceBusyError]          = tr("Device is busy");
    _errorstrings[DeviceNotOpenError]       = tr("Device not open");
    _errorstrings[DeviceNotFoundError]      = tr("Device not found");
    _errorstrings[DownloadInProgressError]  = tr("Download already in progress");
    _errorstrings[InvalidImageError]        = tr("Invalid image");
    _errorstrings[VerifyRamError]           = tr("Verify RAM failed");
    _errorstrings[WriteEepromError]         = tr("EEPROM write failed");
    _errorstrings[VerifyEepromError]        = tr("Verify EEPROM failed");
    _errorstrings[TimeoutError]             = tr("Download timed out");
    _errorstrings[HandshakeError]           = tr("Handshake not received");
    _errorstrings[InvalidHandshakeError]    = tr("Invalid handshake");
    _errorstrings[UnknownError]             = tr("Device error");

    _version = 0;
    _ack     = 0;

    this->session = new PropellerSession(manager, portname);

    totalTimeout.setSingleShot(true);
    handshakeTimeout.setSingleShot(true);
    resetTimer.setSingleShot(true);

    connect(&totalTimeout,      SIGNAL(timeout()), this, SLOT(timeover()));
    connect(&handshakeTimeout,  SIGNAL(timeout()), this, SLOT(timeover()));
    connect(&resetTimer,        SIGNAL(timeout()), this, SIGNAL(prepared()));

    connect(session,&PropellerSession::sendError,
            this,   &PropellerLoader::error);

    QFinalState * s_failure = new QFinalState();
    QFinalState * s_success = new QFinalState();

    connect(s_success,     SIGNAL(entered()), this, SLOT(success_entry()));
    connect(s_failure,     SIGNAL(entered()), this, SLOT(failure_entry()));

    s_active                = new QState();

    QState * s_start        = new QState(s_active);
    QState * s_prepare      = new QState(s_active);
    QState * s_payload      = new QState(s_active);
    QState * s_verify       = new QState(s_active);
    QState * s_write        = new QState(s_active);
    QState * s_verifywrite  = new QState(s_active);

    s_prepare    ->assignProperty(this, "status", tr("Preparing image..."));
    s_payload    ->assignProperty(this, "status", tr("Downloading to RAM..."));
    s_verify     ->assignProperty(this, "status", tr("Verifying RAM..."));
    s_write      ->assignProperty(this, "status", tr("Writing to EEPROM..."));
    s_verifywrite->assignProperty(this, "status", tr("Verifying EEPROM..."));

    s_verify     ->assignProperty(this, "stat", 1);
    s_write      ->assignProperty(this, "stat", 2);
    s_verifywrite->assignProperty(this, "stat", 3);

    s_active->setInitialState(s_start);
    s_active-> addTransition(this,      SIGNAL(failure()), s_failure);
    s_active-> addTransition(&machine,  SIGNAL(started()), s_prepare);

    machine.addState(s_failure);
    machine.addState(s_success);
    machine.addState(s_active);

    // prepare
    connect(s_prepare,       SIGNAL(entered()), this, SLOT(prepare_entry()));

    s_prepare  ->addTransition(this,  SIGNAL(prepared()),s_payload);

    connect(s_payload,       SIGNAL(entered()), this, SLOT(sendpayload_entry()));
    connect(s_payload,       SIGNAL(exited()),  this, SLOT(sendpayload_exit()));

    s_payload  ->addTransition(this,  SIGNAL(upload_completed()),  s_verify);
    s_payload  ->addTransition(this,  SIGNAL(success()),           s_success);

    // verify
    connect(s_verify,        SIGNAL(entered()), this, SLOT(acknowledge_entry()));
    connect(s_verify,        SIGNAL(exited()),  this, SLOT(acknowledge_exit()));

    s_verify     ->addTransition(this,  SIGNAL(acknowledged()),  s_write);
    s_verify     ->addTransition(this,  SIGNAL(success()),       s_success);

    connect(s_write,         SIGNAL(entered()), this, SLOT(acknowledge_entry()));
    connect(s_write,         SIGNAL(exited()),  this, SLOT(acknowledge_exit()));

    s_write      ->addTransition(this,  SIGNAL(acknowledged()),  s_verifywrite);

    connect(s_verifywrite,   SIGNAL(entered()), this, SLOT(acknowledge_entry()));
    connect(s_verifywrite,   SIGNAL(exited()),  this, SLOT(acknowledge_exit()));
 
    s_verifywrite->addTransition(this,  SIGNAL(success()),       s_success);
}

PropellerLoader::~PropellerLoader()
{
    session->release();
    delete session;
}

void PropellerLoader::message(const QString & text)
{
    qCDebug(ploader) << qPrintable("["+session->portName()+"]") << qPrintable(text);
}

void PropellerLoader::error(const QString & text)
{
    message("ERROR: "+text);
}

void PropellerLoader::calibrate()
{
    session->putChar(0xf9);
}

void PropellerLoader::writeLong(quint32 value)
{
    session->write(protocol.encodeLong(value));
}

void PropellerLoader::failure_entry()
{
    setProperty("status", tr("ERROR: %1")
            .arg(_errorstrings[_error]));
    totalTimeout.stop();
    session->release();
    emit finished();
}

void PropellerLoader::success_entry()
{
    setProperty("status", tr("Success!"));
    totalTimeout.stop();
    session->release();
    emit finished();
}

/**
  Get the version of the connected device.
  
  \return The version number, or 0 if not found.
  */
int PropellerLoader::version()
{
    _write = 0;
    _run = 0;
    machine.setInitialState(s_active);
    machine.start();

    QEventLoop loop;
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    return _version;
}

void PropellerLoader::prepare_entry()
{
    _error = NoError;
    _completed = 0;
    m_stat = 0;

    _command = 2*_write + _run;

    QByteArray request = protocol.buildRequest((Command::Command) _command);

    _payload = request;

    if (_command > 0)
    {
        _payload.append(protocol.encodeLong(_image.imageSize() / 4));
        _payload.append(protocol.encodeData(_image.data()));
    }

    int timeout_payload = session->calculateTimeout(_payload.size());
    if (_write)
        timeout_payload += 5000; // ms (EEPROM write speed is constant.
                                 // the Propeller firmware only does 32kB EEPROMs
                                 // and this transaction is handled entirely by the firmware.

    session->reset();

    totalTimeout.start(timeout_payload);
    handshakeTimeout.start(session->calculateTimeout(request.size()));
    resetTimer.start(session->resetPeriod());
    elapsedTimer.start();
}

void PropellerLoader::handshake_read()
{
//    qDebug() << "BYTES" << session->bytesAvailable();
    if (session->bytesAvailable() == protocol.reply().size() + 4)
    {
        handshakeTimeout.stop();
        if (session->read(protocol.reply().size()) != protocol.reply())
        {
            _error = InvalidHandshakeError;
            emit failure();
        }

        QByteArray versiondata = session->read(4);

        _version = 0;
        for (int i = 0; i < 4; i++)
        {
            _version += (versiondata.at(i) & 1) +
                        ((versiondata.at(i) >> 5) & 1);
        }

        if (_version != 1)
        {
            _error = HandshakeError;
            emit failure();
        }

        if (_command > 0)
            emit handshake_received();
        else
            emit success();
    }
}

void PropellerLoader::sendpayload_entry()
{
    connect(session,    SIGNAL(bytesWritten(qint64)),   this, SLOT(sendpayload_write()));
    connect(session,    SIGNAL(readyRead()),            this, SLOT(handshake_read()));
    connect(this,       SIGNAL(handshake_received()),   this, SLOT(upload_status()));
    connect(this,       SIGNAL(payload_sent()),         this, SLOT(upload_status()));

    session->clear();         // clear mysterious junk data that arrives just before this write().
    session->write(_payload);
}

void PropellerLoader::sendpayload_exit()
{
    disconnect(this,    SIGNAL(handshake_received()),   this, SLOT(upload_status()));
    disconnect(this,    SIGNAL(payload_sent()),         this, SLOT(upload_status()));
    disconnect(session, SIGNAL(bytesWritten(qint64)),   this, SLOT(sendpayload_write()));
    disconnect(session, SIGNAL(readyRead()),            this, SLOT(handshake_read()));
}

void PropellerLoader::sendpayload_write()
{
    if (!session->bytesToWrite())
        emit payload_sent();
}

void PropellerLoader::upload_status()
{
    _completed++;   // QStateMachine ParallelStates don't work; work around apparent bug in Qt

    if (_completed > 1)
    {
        emit upload_completed();
    }
}

void PropellerLoader::acknowledge_entry()
{
    connect(session,    SIGNAL(readyRead()),this,   SLOT(acknowledge_read()));
    connect(&poll,      SIGNAL(timeout()),  this,   SLOT(calibrate()));

    poll.setInterval(20);
    poll.start();
}

void PropellerLoader::acknowledge_exit()
{
    disconnect(&poll,   SIGNAL(timeout()),  this,   SLOT(calibrate()));
    disconnect(session, SIGNAL(readyRead()),this,   SLOT(acknowledge_read()));
}

void PropellerLoader::acknowledge_read()
{
    if (session->bytesAvailable())
    {
        _ack = QString(session->readAll().data()).toInt();

        poll.stop();
//        message(QString("ACK: %1").arg(_ack));
        if (_ack)
        {
            switch (m_stat)
            {
                case 1:     _error = VerifyRamError;    break;
                case 2:     _error = WriteEepromError;  break;
                case 3:     _error = VerifyEepromError; break;
                default:    _error = UnknownError;      break;
            }
            emit failure();
        }
        else
        {
            if ((m_stat == 3 && _write) 
                    || (m_stat == 1 && !_write))
            {
                if (_run && _write)
                    session->reset();

                emit success();
            }
            else
            {
                emit acknowledged();
            }
        }
    }
}

/**
  Upload a PropellerImage object to the target.

  \param image The PropellerImage to upload.
  \param write Write the image to the EEPROM.
  \param run Run the image after downloading.
  */

bool PropellerLoader::upload(PropellerImage image, bool write, bool run, bool wait)
{
    if (!session->reserve())
    {
        error("Device is busy");
        return false;
    }

    if (!session->isOpen())
    {
        error("Device not open");
        return false;
    }
    
    if (machine.isRunning())
    {
        error("Download already in progress");
        return false;
    }

    if (!image.isValid())
    {
        error("Image is invalid");
        return false;
    }

    if (!session->setBaudRate(115200))
    {
        error("Couldn't set baud rate");
        return false;
    }

    _image = image;
    _write = write;
    _run = run;

    machine.setInitialState(s_active);
    machine.start();

    if (wait)
    {
        QEventLoop loop;
        connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
    }

    return true;
}


//bool PropellerLoader::highSpeedUpload(PropellerImage image, bool write, bool run)
//{
//    QFile file("miniloaders/miniloader.binary");
//
//    if (!file.open(QIODevice::ReadOnly))
//        return false;
//
//    PropellerImage loader(file.readAll());
//
//    // SPLIT ALL PACKETS
//
//    /*
//    QMap<quint32, QByteArray> pieces;
//
//    int last = 0;
//    quint32 lastvalue = 0;
//    for (int i = 0; i < loader.imageSize(); i += 4)
//    {
//        quint32 value = loader.readLong(i);
//        if ((value & 0xFFFFFFF0) == 0x11111110)
//        {
//
//            if (value != 0x11111110)
//            {
//                pieces[lastvalue] = loader.data().mid(last, i-last);
//                last = i+4;
//            }
//            else
//            {
//                pieces[lastvalue] = loader.data().mid(last, i-last);
//                last = i;
//            }
//            lastvalue = value;
//        }
//    }
//    
//    if ((loader.imageSize() - last) > 0)
//        pieces[lastvalue] = loader.data().mid(last, loader.imageSize() - last);
//
//    foreach (quint32 v, pieces.keys())
//        qDebug() << QString::number(v,16) << pieces[v].toHex();
//
//
//    // GENERATE HOST INITIALIZED VALUES
//
//    quint32 maxRxSenseError = 23; // maximum number of cycles by which the detection of a start bit could be off (SHOULD BE AUTO CALCULATED?!?)
//
//    quint32 clkfreq = image.clockFrequency();
//
//    quint32 initialbaud = 115200;
//    quint32 initialperiod = clkfreq / initialbaud;
//
//    quint32 finalbaud = 460800; // 921600
//    quint32 finalperiod = clkfreq / finalbaud;
//
//    quint32 errorperiod = ((1.5 * clkfreq) / finalbaud) - maxRxSenseError;  // no idea what this does
//    quint32 timeout_failsafe = (2 * clkfreq / (3 * 4));                     //
//    quint32 timeout_endofpacket = (2 * finalperiod * 10 / 12);              //
//
//    quint32 total_packet_count = image.imageSize() / (Propeller::_max_data_size - 4); // binary image / (max XBee packet - packet header)
//    quint32 packetid = total_packet_count;
//
//
//    // INJECT HOST VALUES INTO IMAGE AND MOVE SPIN CODE
//
//    PropellerImage l(pieces[0x11111110]);
//
//    l.writeLong(4+0, initialperiod);
//    l.writeLong(4+4, finalperiod);
//    l.writeLong(4+8, errorperiod);
//    l.writeLong(4+12, timeout_failsafe);
//    l.writeLong(4+16, timeout_endofpacket);
//    l.writeLong(4+20, packetid);
//
//    QByteArray newdata = pieces[0x0];
//    QByteArray spincode = pieces[pieces.lastKey()].right(8);
//    pieces[pieces.lastKey()].chop(8);
//
//    newdata.append(l.data());
//    newdata.append(spincode);
//
//    l.setData(newdata);
//
//
//    // MAGIC TRANSFORMATION
//
//    int removedbytes = loader.imageSize() - l.imageSize();
//    qDebug() << removedbytes;
//
//    for (int i = 8; i <= 14; i += 2)
//        l.writeWord(i, l.readWord(i) - removedbytes);
//
//    qDebug() << "SPIN METHODS??" << l.readWord(18) << l.readWord(18)-1;
//    for (int i = 0; i <= (l.readWord(18)-1); i++)
//    {
//        int offset = 16+i*4;
//        qDebug() << offset << l.readWord(offset);
//        l.writeWord(offset, l.readWord(offset) - removedbytes);
//        qDebug() << offset << l.readWord(offset);
//    }
//
//
//
//    // finalize loader image
//
//    loader.setData(newdata);
//    loader.recalculateChecksum();
//
//    qDebug() << loader.checksumIsValid();
//    qDebug() << loader.isValid();
//
//
//    qDebug() << loader.data().size() << loader.data().toHex();
//
//    QByteArray dataz = loader.data();
//    QByteArray pretty = dataz.toHex().toUpper();
//    for (int i = 0 ; i < pretty.size() ; i++)
//    {
//        printf("%c",(unsigned char) pretty[i]);
//        if (i % 2 == 1)
//            printf("\n");
//    }
//
//    */
//
//    // GENERATE HOST INITIALIZED VALUES
//
//    quint32 maxRxSenseError = 23; // maximum number of cycles by which the detection of a start bit could be off (SHOULD BE AUTO CALCULATED?!?)
//
//    quint32 clkfreq = image.clockFrequency();
//
//    quint32 initialbaud = 115200;
//    quint32 initialperiod = clkfreq / initialbaud;
//
//    quint32 finalbaud = 460800; // 921600
//    quint32 finalperiod = clkfreq / finalbaud;
//
//    quint32 errorperiod = ((1.5 * clkfreq) / finalbaud) - maxRxSenseError;  // no idea what this does
//    quint32 timeout_failsafe = (2 * clkfreq / (3 * 4));                     //
//    quint32 timeout_endofpacket = (2 * finalperiod * 10 / 12);              //
//
//    int total_packet_count = image.imageSize() / (Propeller::_max_data_size - 4) + 1; // binary image / (max XBee packet - packet header)
//    quint32 packetid = total_packet_count;
//
//
//    const quint8 rawLoaderImage[348] = {
//        0x00,0xB4,0xC4,0x04,0x6F,0xC3,0x10,0x00,0x5C,0x01,0x64,0x01,0x54,0x01,0x68,0x01,
//        0x4C,0x01,0x02,0x00,0x44,0x01,0x00,0x00,0x48,0xE8,0xBF,0xA0,0x48,0xEC,0xBF,0xA0,
//        0x49,0xA0,0xBC,0xA1,0x01,0xA0,0xFC,0x28,0xF1,0xA1,0xBC,0x80,0xA0,0x9E,0xCC,0xA0,
//        0x49,0xA0,0xBC,0xF8,0xF2,0x8F,0x3C,0x61,0x05,0x9E,0xFC,0xE4,0x04,0xA4,0xFC,0xA0,
//        0x4E,0xA2,0xBC,0xA0,0x08,0x9C,0xFC,0x20,0xFF,0xA2,0xFC,0x60,0x00,0xA3,0xFC,0x68,
//        0x01,0xA2,0xFC,0x2C,0x49,0xA0,0xBC,0xA0,0xF1,0xA1,0xBC,0x80,0x01,0xA2,0xFC,0x29,
//        0x49,0xA0,0xBC,0xF8,0x48,0xE8,0xBF,0x70,0x11,0xA2,0x7C,0xE8,0x0A,0xA4,0xFC,0xE4,
//        0x4A,0x92,0xBC,0xA0,0x4C,0x3C,0xFC,0x50,0x54,0xA6,0xFC,0xA0,0x53,0x3A,0xBC,0x54,
//        0x53,0x56,0xBC,0x54,0x53,0x58,0xBC,0x54,0x04,0xA4,0xFC,0xA0,0x00,0xA8,0xFC,0xA0,
//        0x4C,0x9E,0xBC,0xA0,0x4B,0xA0,0xBC,0xA1,0x00,0xA2,0xFC,0xA0,0x80,0xA2,0xFC,0x72,
//        0xF2,0x8F,0x3C,0x61,0x21,0x9E,0xF8,0xE4,0x31,0x00,0x78,0x5C,0xF1,0xA1,0xBC,0x80,
//        0x49,0xA0,0xBC,0xF8,0xF2,0x8F,0x3C,0x61,0x00,0xA3,0xFC,0x70,0x01,0xA2,0xFC,0x29,
//        0x26,0x00,0x4C,0x5C,0x51,0xA8,0xBC,0x68,0x08,0xA8,0xFC,0x20,0x4D,0x3C,0xFC,0x50,
//        0x1E,0xA4,0xFC,0xE4,0x01,0xA6,0xFC,0x80,0x19,0x00,0x7C,0x5C,0x1E,0x9E,0xBC,0xA0,
//        0xFF,0x9F,0xFC,0x60,0x4C,0x9E,0x7C,0x86,0x00,0x84,0x68,0x0C,0x4E,0xA8,0x3C,0xC2,
//        0x09,0x00,0x54,0x5C,0x01,0x9C,0xFC,0xC1,0x55,0x00,0x70,0x5C,0x55,0xA6,0xFC,0x84,
//        0x40,0xAA,0x3C,0x08,0x04,0x80,0xFC,0x80,0x43,0x74,0xBC,0x80,0x3A,0xA6,0xFC,0xE4,
//        0x55,0x74,0xFC,0x54,0x09,0x00,0x7C,0x5C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//        0x80,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x80,0x00,0x00,0xFF,0xFF,0xF9,0xFF,
//        0x10,0xC0,0x07,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x40,0xB6,0x02,0x00,0x00,
//        0x5B,0x01,0x00,0x00,0x08,0x02,0x00,0x00,0x55,0x73,0xCB,0x00,0x50,0x45,0x01,0x00,
//        0x00,0x00,0x00,0x00,0x35,0xC7,0x08,0x35,0x2C,0x32,0x00,0x00
//    };
//
//    QByteArray _loader((char*) rawLoaderImage, 348);
//
//    const int offset = 348 - 8*4; // offset from end
//
//    loader.setData(_loader);
//
//    loader.writeLong(offset+0, initialperiod);
//    loader.writeLong(offset+4, finalperiod);
//    loader.writeLong(offset+8, errorperiod);
//    loader.writeLong(offset+12, timeout_failsafe);
//    loader.writeLong(offset+16, timeout_endofpacket);
//    loader.writeLong(offset+20, packetid);
//
//    loader.recalculateChecksum();
//
//    upload(loader, write, run);
//
//
//
//    session->setBaudRate(finalbaud);
//
//    QByteArray data = image.data();
//    qDebug() << data.size();
//
//    int maxsize = Propeller::_max_data_size - 4;
//
//    qDebug() << "DOWNLOADING";
//    for (int i = total_packet_count ; i > 0 ; i--)
//    {
//        qDebug() << "ASSEMBLE";
//        QByteArray payload;
//        payload.append(protocol.packLong(i));
//        payload.append(data.left(maxsize));
//
//        qDebug() << payload.size();
//
////        sendPayload(payload);
//        qDebug() << "RETURN FROM PAYHLOAD";
//    }
//
//
////    upload(image, write, run);
////
//    return true;
//}

void PropellerLoader::timestamp()
{
    message(QString("%1... %2 ms elapsed")
            .arg(QString(20,QChar(' ')))
            .arg(elapsedTimer.elapsed()));
}

void PropellerLoader::timeover()
{
    _error = TimeoutError;
    emit failure();
}

QString PropellerLoader::versionString(int version)
{
    return _versionstrings[version];
}
