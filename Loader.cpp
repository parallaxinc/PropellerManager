#include "Loader.h"

#include <QEventLoop>

Loader::Loader(QString port, int reset_gpio, QObject * parent) :
    QObject(parent)
{
    version = 0;

    serial.setPortName(port);
    serial.setBaudRate(230400);
//    serial.setBaudRate(115200);

    this->reset_gpio = reset_gpio;

    int request_length = 250;
    int reply_length = 250;

    sequence = build_lfsr_sequence(request_length + reply_length);
    request = build_request(sequence, request_length);
    reply = build_reply(sequence, reply_length, request_length);



//    try:
//        import RPi.GPIO as GPIO
//        self.GPIO = GPIO
//        self.GPIO.setmode(self.GPIO.BCM)
//        self.GPIO.setwarnings(False)
//        self.GPIO.setup(self.reset_gpio, self.GPIO.OUT, initial=self.GPIO.HIGH)
//    except ImportError:
//        print("RPi.GPIO library required for GPIO reset.")
}

Loader::~Loader()
{
    serial.close();

    // cleanup gpio
    if (reset_gpio > -1)
    {
//            self.GPIO.cleanup()
    }
}

int Loader::get_version()
{
    handshake();
    write_long(Command::Shutdown);
    return version;
}

int Loader::open()
{
    serial.open(QIODevice::ReadWrite);
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

    int t1 = 25, t2 = 50;

    if (reset_gpio > -1) //and not self.GPIO == None:
    {
//        self.GPIO.output(self.reset_gpio, self.GPIO.LOW)
        QThread::msleep(t1);
//        self.GPIO.output(self.reset_gpio, self.GPIO.HIGH)
        QThread::msleep(t2);
    }
    else
    {
        serial.setDataTerminalReady(true);
        QThread::msleep(t1);
        serial.setDataTerminalReady(false);
        QThread::msleep(t2);
    }

    serial.clear(QSerialPort::Input);
}

void Loader::calibrate()
{
    write_byte(0xf9);
}

void Loader::write_byte(char value)
{
    serial.putChar(value);
    QCoreApplication::processEvents();
}

void Loader::write_long(unsigned int value)
{
    serial.write(encode_long(value));
    QCoreApplication::processEvents();
}

void Loader::read_handshake()
{
    qDebug() << "DATA" << serial.bytesAvailable();
    if (serial.bytesAvailable() == 258)
    {
        real_reply = serial.read(reply.size());
        qDebug() << "GOT THE DATA" << real_reply.size() << real_reply.data();

        qDebug() << reply.size() << real_reply.size();
        if (real_reply != reply)
        {
            qDebug() << "DATA IS WRONG";
            emit finished();
        }

        qDebug() << "DATA IS RIGHT";
        qDebug() << "GET VERSION";

        QByteArray versiondata = serial.read(8);

        version = 0;
        for (int i = 0; i < 8; i++)
        {
            version = ((version >> 1) & 0x7f) | ((versiondata.at(i) & 0x1) << 7);
        }
        qDebug() << QString::number(version);
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

QByteArray Loader::prepare_code(QByteArray code, bool eeprom)
{
    return QByteArray();
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

void Loader::error()
{
    qDebug() << "Download has timed out!";
}


int Loader::handshake()
{
    reset();
    calibrate();

    real_reply.clear();

    QByteArray header(reply.size()+8,'\xf9');

    connect(&serial, SIGNAL(readyRead()), this, SLOT(read_handshake()));

    serial.write(request);
    serial.write(header);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(error()));
    timer.start(100);
    loop.exec();

    disconnect(&serial);

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


void Loader::upload_binary(QByteArray binary, bool isEEPROM)
{

    if (binary.isEmpty())
    {
        qDebug() << "Image empty!";
        return;
    }

    if (binary.size() % 4 != 0)
    {
        qDebug() << "Invalid code size; must be multiple of 4!";
        return;
    }

    if (isEEPROM)
    {
        binary = convert_binary_to_eeprom(binary);
    }

    if (checksum(binary, isEEPROM))
    {
        qDebug() << "Code checksum error:"
            << QString::number(checksum(binary, false),16);
        return;
    }

    QByteArray encoded_binary = encode_binary(binary);

    send_application_image(encoded_binary, binary.size(), Command::Run);
}

void Loader::writeEmpty()
{
    qDebug() << "EMPTY";
    if (serial.bytesToWrite())
        emit finished();
}

void Loader::read_acknowledge()
{
    qDebug() << "GOT ACK" << serial.bytesAvailable();
    if (serial.bytesAvailable())
    {
        qDebug() << serial.readAll().data();
        qDebug() << "ACK" << ack;
        poll.stop();
        emit finished();
    }
}

int Loader::send_application_image(QByteArray encoded_binary, int image_size, Command::Command command)
{
    qDebug() << "SENDING DATA";

    connect(&serial, SIGNAL(readyRead()), this, SLOT(read_acknowledge()));
    connect(&serial, SIGNAL(bytesWritten(qint64)), this, SLOT(writeEmpty()));
    bool write = false, run = false;

    write_long(1);
    write_long(image_size / 4);
    serial.write(encoded_binary);

    QEventLoop loop;
//    QTimer timer;
//    timer.setSingleShot(true);
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
//    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
//    connect(&timer, SIGNAL(timeout()), this, SLOT(error()));
//    timer.start(2000);
    loop.exec();

    disconnect(&serial, SIGNAL(bytesWritten(qint64)), this, SLOT(writeEmpty()));

    qDebug() << "STARTING POLLING";

    connect(&poll, SIGNAL(timeout()), this, SLOT(calibrate()));
    poll.setInterval(20);
    poll.start();

    loop.exec();

    disconnect(&poll);
    disconnect(&serial);

    return 0;
}

