#include "propellersession.h"
#include "gpio.h"
#include "utility.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QSerialPortInfo>
#include <QDebug>
#include <QThread>

#include <stdio.h>

PropellerSession::PropellerSession(   QString port,
                                    int reset_gpio,
                                    bool useRtsReset,
                                    QObject * parent)
    : QObject(parent)
{
    this->useRtsReset = useRtsReset;
    version = 0;
    resourceErrorCount = 0;

    serial.setSettingsRestoredOnClose(false);
    serial.setPortName(port);

#if not defined(Q_PROCESSOR_ARM_V6)
    serial.setBaudRate(230400);
#endif

    connect(&serial,SIGNAL(error(QSerialPort::SerialPortError)),
            this,   SLOT(device_error(QSerialPort::SerialPortError))); 

    connect(this,   &PropellerSession::sendError,
            this,   &Utility::print_error);

    this->reset_gpio = reset_gpio;

    int request_length = 250;
    int reply_length = 250;

    sequence = build_lfsr_sequence(request_length + reply_length);
    request = build_request(sequence, request_length);
    reply = build_reply(sequence, reply_length, request_length);

}

PropellerSession::~PropellerSession()
{
    serial.close();
}

int PropellerSession::open()
{
    resourceErrorCount = 0;

    if (!serial.open(QIODevice::ReadWrite))
    {
        qDebug() << serial.errorString();
        return 1;
    }
#if defined(Q_PROCESSOR_ARM_V6) && defined(Q_OS_LINUX)
    serial.setBaudRate(115200);
#endif
    return 0;
}

int PropellerSession::close()
{
    serial.close();
    return 0;
}

void PropellerSession::reset()
{
    serial.clear(QSerialPort::Output);

    if (reset_gpio > -1)
    {
        GPIO gpio(reset_gpio, GPIO::Out);
        gpio.Write(GPIO::Low);
        gpio.Write(GPIO::High);
    }
    else
    {
        if (useRtsReset)
        {
            serial.setRequestToSend(true);
            serial.setRequestToSend(false);
        }
        else
        {
            serial.setDataTerminalReady(true);
            serial.setDataTerminalReady(false);
        }
    }

#if defined(Q_PROCESSOR_ARM_V6) && defined(Q_OS_LINUX)
    QThread::msleep(80);
#else
    QThread::msleep(60);
#endif

    serial.clear(QSerialPort::Input);
}

void PropellerSession::calibrate()
{
    write_byte(0xf9);
}

void PropellerSession::write_byte(char value)
{
    serial.putChar(value);
}

void PropellerSession::write_long(unsigned int value)
{
    serial.write(encode_long(value));
}


void PropellerSession::read_handshake()
{
    if (serial.bytesAvailable() == 258)
    {
        real_reply = serial.read(reply.size());

        if (real_reply != reply)
        {
            emit finished();
        }

        QByteArray versiondata = serial.read(8);

        version = 0;
        for (int i = 0; i < 8; i++)
        {
            version = ((version >> 1) & 0x7f) | ((versiondata.at(i) & 0x1) << 7);
        }
//        qDebug() << QString::number(version);
        emit finished();
    }
}

int PropellerSession::get_version()
{
    handshake();
    write_long(Command::Shutdown);
    QCoreApplication::processEvents();
    return version;
}

QByteArray PropellerSession::encode_long(unsigned int value)
{
    QByteArray result;
    for (int i = 0; i < 10; i++)
    {
        result.append(0x92 | (value & 0x01) | ((value & 2) << 2) | ((value & 4) << 4));
        value >>= 3;
    }
    result.append(0xf2 | (value & 0x01) | ((value & 2) << 2));
//    qDebug() << result.toHex().data();
    return result;
}

int PropellerSession::lfsr(int * seed)
{
    char ret = *seed & 0x01;
    *seed = ((*seed << 1) & 0xfe) | (((*seed >> 7) ^ (*seed >> 5) ^ (*seed >> 4) ^ (*seed >> 1)) & 1);
    return ret;
}

QList<char> PropellerSession::build_lfsr_sequence(int size)
{
    int seed = 'P';
    
    QList<char> seq;
    for (int i = 0; i < size; i++)
    {
        seq.append(lfsr(&seed));
    }

    return seq;
}

QByteArray PropellerSession::build_request(QList<char> seq, int size)
{
    QByteArray array;
    for (int i = 0; i < size; i++)
    {
        array.append(seq[i] | 0xfe);
    }
    return array;
}


QByteArray PropellerSession::build_reply(QList<char> seq, int size, int offset)
{
    QByteArray array;
    for (int i = offset;
            i < (offset + size);
            i++)
    {
        array.append(seq[i] | 0xfe);
    }
    return array;
}

void PropellerSession::loader_error()
{
    error = Error::Timeout;
}

void PropellerSession::device_error(QSerialPort::SerialPortError e)
{
    switch (e)
    {
        case QSerialPort::NoError:
            break;
        case QSerialPort::DeviceNotFoundError:
        case QSerialPort::PermissionError:
        case QSerialPort::NotOpenError:
            break;
        case QSerialPort::ParityError:
        case QSerialPort::FramingError:
        case QSerialPort::BreakConditionError:
        case QSerialPort::WriteError:
        case QSerialPort::ReadError:
        case QSerialPort::UnsupportedOperationError:
        case QSerialPort::TimeoutError:
        case QSerialPort::UnknownError:
        case QSerialPort::ResourceError: // SUPER IMPORTANT
            resourceErrorCount++;
            if (resourceErrorCount > 1)
            {
                qDebug() << "ERROR: " << e;
                close();
                emit finished();
                emit sendError(e,"Device unexpectedly disconnected!"); 
            }
            break;
        default:
            break;
    }
    
    error = Error::Timeout;
}


int PropellerSession::handshake()
{
    reset();

    real_reply.clear();

    QByteArray header(reply.size()+8,'\xf9');

    connect(&serial, SIGNAL(readyRead()), this, SLOT(read_handshake()));

    calibrate();
    serial.write(request);
    serial.write(header);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(loader_error()));
    timer.start(500);
    loop.exec();

    disconnect(&serial, SIGNAL(readyRead()), this, SLOT(read_handshake()));

    return version;
}

QByteArray PropellerSession::encode_binary(QByteArray binary)
{
    QByteArray encoded_binary;
    for (int i = 0 ; i < binary.size() ; i += 4)
    {
        encoded_binary.append(encode_long(
                ((unsigned char) binary.at(i)) | 
                ((unsigned char) binary.at(i + 1) << 8)  | 
                ((unsigned char) binary.at(i + 2) << 16) |
                ((unsigned char) binary.at(i + 3) << 24)
                ));
    }
    return encoded_binary;
}

void PropellerSession::write_terminal(const QString & text)
{
    serial.write(qPrintable(text));
    serial.write("\r");
}

void PropellerSession::read_terminal()
{
    foreach (char c, serial.readAll())
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

void PropellerSession::terminal()
{
    serial.setBaudRate(115200);
    connect(&serial, SIGNAL(readyRead()), this, SLOT(read_terminal()));
    connect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write_terminal(const QString &)));



    QEventLoop loop;
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    disconnect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write_terminal(const QString &)));
    disconnect(&serial, SIGNAL(readyRead()), this, SLOT(read_terminal()));

    return;
}

void PropellerSession::upload_binary(PropellerImage binary, bool write, bool run)
{
    if (!binary.isValid())
        return;

    QByteArray encoded_binary = encode_binary(binary.data());

    Utility::print_task("Connecting to '"+serial.portName()+"'...");
    if (!handshake())
    {
        Utility::print_status("NOT FOUND");
        return;
    }
    Utility::print_status("DONE");

    int command = 2*write + run;
    write_long(command);

    Utility::print_task("Downloading to RAM...");
    if (send_application_image(encoded_binary, binary.imageSize()) != 0)
        return;

    Utility::print_status("DONE");

    Utility::print_task("Verifying RAM...");
    if (poll_acknowledge() != 0)
    {
        Utility::print_status("RAM CHECKSUM INVALID");
        return;
    }

    Utility::print_status("DONE");

    if (!write)
        return;
    
    Utility::print_task("Writing EEPROM...");

    if (poll_acknowledge() != 0)
    {
        Utility::print_status("FAIL");
        return;
    }
    Utility::print_status("DONE");

    Utility::print_task("Verifying EEPROM...");

    if (poll_acknowledge() != 0)
    {
        Utility::print_status("FAIL");
        return;
    }
    Utility::print_status("DONE");

    if (run)
        reset();
}

void PropellerSession::writeEmpty()
{
    if (serial.bytesToWrite())
        error = 0;
        emit finished();
}

void PropellerSession::read_acknowledge()
{
//    qDebug() << "GOT ACK" << serial.bytesAvailable();
    if (serial.bytesAvailable())
    {
        ack = QString(serial.readAll().data()).toInt();
//        qDebug() << "ACK" << ack;
        poll.stop();
        error = 0;
        emit finished();
    }
}

int PropellerSession::send_application_image(QByteArray encoded_binary, int image_size)
{
    error = 0;
    connect(&serial, SIGNAL(bytesWritten(qint64)), this, SLOT(writeEmpty()));

    write_long(image_size / 4);
    serial.write(encoded_binary);

    QEventLoop loop;
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    disconnect(&serial, SIGNAL(bytesWritten(qint64)), this, SLOT(writeEmpty()));

    return error;
}

int PropellerSession::poll_acknowledge()
{
    connect(&serial, SIGNAL(readyRead()), this, SLOT(read_acknowledge()));
    connect(&poll, SIGNAL(timeout()), this, SLOT(calibrate()));

    poll.setInterval(20);
    poll.start();

    QEventLoop loop;
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));

    QTimer timeout;
    timeout.setSingleShot(true);
    connect(&timeout, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(&timeout, SIGNAL(timeout()), this, SLOT(loader_error()));
    timeout.start(5000);

    loop.exec();

    disconnect(&poll);
    disconnect(&poll, SIGNAL(timeout()), this, SLOT(calibrate()));

    disconnect(&serial, SIGNAL(readyRead()), this, SLOT(read_acknowledge()));

    return error;
}


QStringList PropellerSession::list_devices()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QStringList result;

    foreach (QSerialPortInfo port, ports)
    {
        result.append(port.systemLocation());
    }
    return result;
}

