#include "Loader.h"

Loader::Loader(QString port, int reset_gpio)
{
    serial.setPortName(port);
    serial.setBaudRate(115200);

    this->reset_gpio = reset_gpio;
    seed = 'P';

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

int Loader::lfsr()
{
    seed = ((seed << 1) & 0xfe) | (((seed >> 7) ^ (seed >> 5) ^ (seed >> 4) ^ (seed >> 1)) & 1);
    return seed & 0x01;
}

// high-level functions
int Loader::get_version()
{
    open();
//    int version = connect();
    int version = 0;
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
    int version = connect();
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
        QThread::msleep(90);
    }
    else
    {
        serial.setDataTerminalReady(true);
        QThread::msleep(25);
        serial.setDataTerminalReady(false);
        QThread::msleep(90);
    }

    serial.clear(QSerialPort::Input);
}

void Loader::calibrate()
{
    write_byte(0xf9);
}

void Loader::write_byte(char value)
{
    serial.write(QByteArray(value,1));
}

void Loader::write_long(unsigned int value)
{
    serial.write(encode_long(value));
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

int Loader::connect()
{
    return 0;
}
