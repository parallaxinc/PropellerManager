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

namespace Error {
    enum Error {
        None,
        Timeout,
        BadReply,
        Program,
        Verification,
        Checksum,
        NotFound
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
    int error;

    int checksum(QByteArray binary, bool isEEPROM);
    QByteArray convert_binary_to_eeprom(QByteArray binary);
    QByteArray encode_binary(QByteArray binary);
    int send_application_image(QByteArray encoded_binary, int image_size);
    int poll_acknowledge();

    QTimer poll;

signals:
    void finished();

private slots:
    void read_handshake();
    void read_acknowledge();
    void loader_error();
    void download_error();
    void calibrate();
    void writeEmpty();

public:
    QSerialPort serial;
    int reset_gpio;
    void write_byte(char value);
    void write_long(unsigned int value);
    int handshake();

public:
    Loader(QString port, int reset_gpio=-1, QObject * parent = 0);
    ~Loader();

    int open();
    int close();
    int get_version();
    void reset();
    QByteArray encode_long(unsigned int value);
    void upload_binary(QByteArray binary, bool eeprom=false, bool run=true);
    void list();

    void open_terminal();
    void close_terminal();
};

