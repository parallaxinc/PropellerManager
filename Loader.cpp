#include "Loader.h"

#include <QEventLoop>

Loader::Loader(QString port, int reset_gpio, QObject * parent) :
    QObject(parent)
{
    version = 0;

    serial.setPortName(port);
    serial.setBaudRate(115200);

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
    // check if open
    serial.close();

    // cleanup gpio
    if (reset_gpio > -1)
    {
//            self.GPIO.cleanup()
    }
}

// high-level functions
int Loader::get_version()
{
    open();
    handshake();
    write_long(CMD_SHUTDOWN);
    QThread::msleep(10);
    reset();
    close();
    return version;
}

void Loader::upload(QByteArray code, bool eeprom, bool run, bool terminal)
{
    open();
    code = prepare_code(code, eeprom);
    int code_len = 0; // WHAT IS THAHITJ?!~?!?
    int version = handshake();
    qDebug() << "Connected (version=" << version << ")";
    send_code(code, code_len, eeprom, run);

    if (terminal)
    {
//        while (true)
//        {
//            ser = self.serial.read()
//            if ser != None:
//                sys.stdout.write(ser)
//                sys.stdout.flush()
//        }
    }
    else
    {
        close();
    }
}

// low-level functions
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

    if (reset_gpio > -1) //and not self.GPIO == None:
    {
//        self.GPIO.output(self.reset_gpio, self.GPIO.LOW)
        QThread::msleep(25);
//        self.GPIO.output(self.reset_gpio, self.GPIO.HIGH)
        QThread::msleep(70);
    }
    else
    {
        serial.setDataTerminalReady(true);
        QThread::msleep(25);
        serial.setDataTerminalReady(false);
        QThread::msleep(70);
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

void Loader::check_response()
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
    qDebug() << result.toHex();
    return result;
}

QByteArray Loader::prepare_code(QByteArray code, bool eeprom)
{
    return QByteArray();
}

QByteArray Loader::send_code(QByteArray encoded_code, int size, bool eeprom, bool run)
{

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




int Loader::handshake()
{
    reset();
    calibrate();

    real_reply.clear();

    QByteArray header(reply.size()+8,'\xf9');

    connect(&serial, SIGNAL(readyRead()), this, SLOT(check_response()));

    serial.write(request);
    serial.write(header);

    QEventLoop loop;
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    disconnect(&serial, SIGNAL(readyRead()));

    return version;
}
