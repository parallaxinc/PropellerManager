#include <QSerialPort>
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


#include <QObject>
#include <QSocketNotifier>
#include <QTextStream>
#include <unistd.h> //Provides STDIN_FILENO

/**
  \class ConsoleReader

  This class provides some stuff for doing things.


  */

class ConsoleReader : public QObject
{
    Q_OBJECT
public:
    explicit ConsoleReader(QObject *parent = 0) : 
        QObject(parent),
        notifier(STDIN_FILENO, QSocketNotifier::Read)
    {
        connect(&notifier, SIGNAL(activated(int)), this, SLOT(text()));
    }

signals:
    void textReceived(QString message);

public slots:
    void text()
    {
        QTextStream qin(stdin);
        QString line = qin.readLine();
        emit textReceived(line);
    }
    private:
    QSocketNotifier notifier;
};



class PropellerDevice : public QObject
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
    bool useRtsReset;
    int resourceErrorCount;

    int checksum(QByteArray binary, bool isEEPROM);
    QByteArray convert_binary_to_eeprom(QByteArray binary);
    QByteArray encode_binary(QByteArray binary);
    int send_application_image(QByteArray encoded_binary, int image_size);
    int poll_acknowledge();

    QTimer poll;
    ConsoleReader console;

signals:
    void finished();
    void sendError(int code, const QString & message);

private slots:
    void read_handshake();
    void read_acknowledge();
    void read_terminal();
    void write_terminal(const QString & text);

    void loader_error();
    void device_error(QSerialPort::SerialPortError e);
    void print_error(int code, const QString & message);
    void calibrate();
    void writeEmpty();

public:
    QSerialPort serial;
    int reset_gpio;
    void write_byte(char value);
    void write_long(unsigned int value);
    int handshake();

public:
    PropellerDevice(QString port, int reset_gpio=-1, bool useRtsReset = false, QObject * parent = 0);
    ~PropellerDevice();

    int open();
    int close();
    int get_version();
    void reset();
    QByteArray encode_long(unsigned int value);
    void upload_binary(QByteArray binary, bool eeprom=false, bool run=true);
    void list();

    void terminal();

    static QStringList list_devices();

signals:
    void requestPrint(QString text);
    void requestPrint_color(QString text);

public slots:
    void print(const QString & text);
    void print_task(const QString & text);
    void print_status(const QString & text);
};

