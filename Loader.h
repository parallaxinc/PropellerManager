#include <QCoreApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QThread>
#include <QTimer>

class Loader : public QObject
{
    Q_OBJECT

private:
    int lfsr(int * seed);
    QList<char> sequence;
    QList<char> build_lfsr_sequence(int size);

    QByteArray request;
    QByteArray reply;
    QByteArray real_reply;

    QByteArray build_request(QList<char> seq, int size);
    QByteArray build_reply(  QList<char> seq, int size, int offset);
    int version;

signals:
    void finished();

private slots:
    void check_response();

public:
    QSerialPort serial;
    int reset_gpio;
    void write_byte(char value);
    void write_long(unsigned int value);
    void calibrate();
    int handshake();
    QByteArray prepare_code(QByteArray code, bool eeprom=false);
    QByteArray send_code(QByteArray encoded_code, int size, bool eeprom=false, bool run=false);

    enum Command {
        CMD_SHUTDOWN,
        CMD_LOADRAMRUN,
        CMD_LOADEPPROM,
        CMD_LOADEPPROMRUN
    };

    enum Errors {
        Error_None,
        Error_Timeout,
        Error_BadReply,
        Error_Program,
        Error_Verification,
        Error_Checksum,
        Error_NotFound
    };

public:
    Loader(QString port, int reset_gpio=-1, QObject * parent = 0);
    ~Loader();

    int open();
    int close();
    int get_version();
    void reset();
    QByteArray encode_long(unsigned int value);
    void upload(QByteArray code, bool eeprom=false, bool run=true, bool terminal=false);
    void list();

};



