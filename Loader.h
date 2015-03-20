#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QThread>

class Loader
{
private:
    QSerialPort serial;
    int reset_gpio;

    int seed;
    int lfsr();
    void write_byte(char value);
    void write_long(unsigned int value);
    void calibrate();
    QByteArray prepare_code(QByteArray code, bool eeprom=false);
    QByteArray send_code(QByteArray encoded_code, int size, bool eeprom=false, bool run=false);
    int connect();

    enum Command {
        CMD_SHUTDOWN,
        CMD_LOADRAMRUN,
        CMD_LOADEPPROM,
        CMD_LOADEPPROMRUN
    };

public:
    Loader(QString port, int reset_gpio=-1);
    ~Loader();

    int open();
    int close();
    int get_version();
    void reset();
    QByteArray encode_long(unsigned int value);
    void upload(QByteArray code, bool eeprom=false, bool run=true, bool terminal=false);

};



