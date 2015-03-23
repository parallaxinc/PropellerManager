#include <QCoreApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QThread>
#include <QTimer>

namespace Command {
    enum Command {
        Shutdown,
        Run,
        Write,
        WriteRun
    };
};


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
    int ack;

    int checksum(QByteArray binary, bool isEEPROM);
    QByteArray convert_binary_to_eeprom(QByteArray binary);
    QByteArray encode_binary(QByteArray binary);
    int send_application_image(QByteArray encoded_binary, int image_size, Command::Command command);

    QTimer poll;

signals:
    void finished();

private slots:
    void read_handshake();
    void read_acknowledge();
    void error();
    void calibrate();
    void writeEmpty();

public:
    QSerialPort serial;
    int reset_gpio;
    void write_byte(char value);
    void write_long(unsigned int value);
    int handshake();
    QByteArray prepare_code(QByteArray code, bool eeprom=false);

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
    void upload_binary(QByteArray binary, bool isEEPROM);
    void list();

    void open_terminal();
    void close_terminal();
};

