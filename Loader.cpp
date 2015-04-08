#include "Loader.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QSerialPortInfo>
#include <QDebug>
#include <QThread>

#include "GPIO.h"
#include <stdio.h>

Loader::Loader(QString port, int reset_gpio, bool useRtsReset, QObject * parent) :
    QObject(parent)
{
    this->useRtsReset = useRtsReset;
    version = 0;

    serial.setSettingsRestoredOnClose(false);
    serial.setPortName(port);
    serial.setBaudRate(230400);
//    serial.setBaudRate(115200);

    connect(&serial,    SIGNAL(error(QSerialPort::SerialPortError)),
            this,       SLOT(device_error(QSerialPort::SerialPortError))); 

    connect(this,    SIGNAL(sendError(int, const QString &)),
            this,   SLOT(print_error(int, const QString &)));

    this->reset_gpio = reset_gpio;

    int request_length = 250;
    int reply_length = 250;

    sequence = build_lfsr_sequence(request_length + reply_length);
    request = build_request(sequence, request_length);
    reply = build_reply(sequence, reply_length, request_length);

}

Loader::~Loader()
{
    serial.close();
}

int Loader::get_version()
{
    handshake();
    write_long(Command::Shutdown);
    QCoreApplication::processEvents();
    return version;
}

int Loader::open()
{
    if (!serial.open(QIODevice::ReadWrite))
    {
        qDebug() << serial.errorString();
        return 1;
    }
    return 0;
}

int Loader::close()
{
    serial.close();
    return 0;
}

void Loader::reset()
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

    QThread::msleep(60);

    serial.clear(QSerialPort::Input);
}

void Loader::calibrate()
{
    write_byte(0xf9);
}

void Loader::write_byte(char value)
{
    serial.putChar(value);
}

void Loader::write_long(unsigned int value)
{
    serial.write(encode_long(value));
}


void Loader::print_task(const QString & text)
{
    fprintf(stderr, "%-30s",qPrintable(text));
    fflush(stderr);
}

void Loader::print_status(const QString & text)
{
    print("[ ");
    print(text);
    print(" ]\n");
}

void Loader::print(const QString & text)
{
    fprintf(stderr, "%s",qPrintable(text));
    fflush(stderr);
}

void Loader::read_handshake()
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

QByteArray Loader::encode_long(unsigned int value)
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

int Loader::lfsr(int * seed)
{
    char ret = *seed & 0x01;
    *seed = ((*seed << 1) & 0xfe) | (((*seed >> 7) ^ (*seed >> 5) ^ (*seed >> 4) ^ (*seed >> 1)) & 1);
    return ret;
}

QList<char> Loader::build_lfsr_sequence(int size)
{
    int seed = 'P';
    
    QList<char> seq;
    for (int i = 0; i < size; i++)
    {
        seq.append(lfsr(&seed));
    }

    return seq;
}

QByteArray Loader::build_request(QList<char> seq, int size)
{
    QByteArray array;
    for (int i = 0; i < size; i++)
    {
        array.append(seq[i] | 0xfe);
    }
    return array;
}


QByteArray Loader::build_reply(QList<char> seq, int size, int offset)
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

void Loader::loader_error()
{
    error = Error::Timeout;
}

void Loader::print_error(int code, const QString & message)
{
    qDebug("[ ERROR %2i ]: %s",code,qPrintable(message));
}

void Loader::device_error(QSerialPort::SerialPortError e)
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
            qDebug() << "ERROR: " << e;
            close();
            emit finished();
            emit sendError(e,"Device unexpectedly disconnected!"); 
            break;
        default:
            break;
    }
    
    error = Error::Timeout;
}


int Loader::handshake()
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

int Loader::checksum(QByteArray binary, bool isEEPROM)
{
    int checksum = 0;
    for (int i = 0; i < binary.size(); i++)
    {
        checksum += (unsigned char) binary.at(i);
    }

    if (!isEEPROM)
    {
        checksum += 2 * (0xff + 0xff + 0xf9 + 0xff);
    }
    checksum &= 0xff;

    return checksum;
}

QByteArray Loader::convert_binary_to_eeprom(QByteArray binary)
{
    int EEPROM_SIZE = 32768;

    if (binary.size() > EEPROM_SIZE - 8)
    {
        qDebug() << "Code too long for EEPROM (max" << EEPROM_SIZE - 8 << "bytes)";
        return QByteArray();
    }

    int dbase =  (unsigned char) binary.at(0x0a) + 
                ((unsigned char) binary.at(0x0b) << 8);

    if (dbase > EEPROM_SIZE)
    {
        qDebug() << "Binary size greater than EEPROM_SIZE";
        return QByteArray();
    }

//    qDebug() << "DBASE" << dbase 
//        << QString::number((unsigned char) binary.at(0x0a)) 
//        << QString::number((unsigned char) binary.at(0x0b));

    binary.append(QByteArray(dbase - 8 - binary.size(),0x00));
    binary.append(QByteArray::fromHex("fffff9fffffff9ff"));
    binary.append(QByteArray(EEPROM_SIZE - binary.size(),0x00));

    return binary;
}


QByteArray Loader::encode_binary(QByteArray binary)
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

void Loader::write_terminal(const QString & text)
{
    serial.write(qPrintable(text));
    serial.write("\r");
}

void Loader::read_terminal()
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

void Loader::terminal()
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

void Loader::upload_binary(QByteArray binary, bool eeprom, bool run)
{
    if (binary.isEmpty())
    {
        print_status("EMPTY IMAGE");
        return;
    }

    if (binary.size() % 4 != 0)
    {
        print_status("INVALID IMAGE SIZE");
        return;
    }

    if (eeprom)
    {
        binary = convert_binary_to_eeprom(binary);
    }

    if (checksum(binary, eeprom))
    {
        print_status("BAD CHECKSUM");
        return;
    }

    QByteArray encoded_binary = encode_binary(binary);

    print_task("Connecting to '"+serial.portName()+"'...");
    if (!handshake())
    {
        print_status("NOT FOUND");
        return;
    }
    print_status("DONE");

    int command = 2*eeprom + run;
    write_long(command);

    print_task("Downloading to RAM...");
    if (send_application_image(encoded_binary, binary.size()) != 0)
        return;

    print_status("DONE");

    print_task("Verifying RAM...");
    if (poll_acknowledge() != 0)
    {
        print_status("BAD CHECKSUM");
        return;
    }

    print_status("DONE");

    if (!eeprom)
        return;
    
    print_task("Writing EEPROM...");

    if (poll_acknowledge() != 0)
    {
        print_status("FAIL");
        return;
    }
    print_status("DONE");

    print_task("Verifying EEPROM...");

    if (poll_acknowledge() != 0)
    {
        print_status("FAIL");
        return;
    }
    print_status("DONE");

    if (run)
        reset();
}

void Loader::writeEmpty()
{
    if (serial.bytesToWrite())
        error = 0;
        emit finished();
}

void Loader::read_acknowledge()
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

int Loader::send_application_image(QByteArray encoded_binary, int image_size)
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

int Loader::poll_acknowledge()
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


QStringList Loader::list_devices()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QStringList result;

    foreach (QSerialPortInfo port, ports)
    {
        result.append(port.systemLocation());
    }
    return result;
}

