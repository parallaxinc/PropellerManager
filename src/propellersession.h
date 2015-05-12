#pragma once

#include "input_console.h"
#include "propellerimage.h"

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

/**
  @class PropellerSession 
  
  PropellerSession provides an interface for connecting and managing
  a single Propeller device.

  */
class PropellerSession : public QObject
{
    Q_OBJECT

private:
    QSerialPort serial;
    int reset_gpio;
    void write_byte(char value);
    void write_long(unsigned int value);
    int handshake();

    int lfsr(int * seed);
    QList<char> sequence;
    QList<char> build_lfsr_sequence(int size);

    QByteArray request;
    QByteArray reply;
    QByteArray real_reply;

    QByteArray build_request(QList<char> seq, int size);
    QByteArray build_reply(  QList<char> seq, int size, int offset);
    int _version;
    int ack;
    int error;
    bool useRtsReset;
    int resourceErrorCount;

    QByteArray encode_binary(PropellerImage image);
    int send_application_image(QByteArray encoded_binary, int image_size);
    int poll_acknowledge();

    QByteArray encode_long(unsigned int value);

    QTimer poll;
    Input::Console console;

signals:
    void finished();
    void sendError(int code, const QString & message);
    void requestPrint(QString text);
    void requestPrint_color(QString text);

private slots:
    void read_handshake();
    void read_acknowledge();
    void read_terminal();
    void write_terminal(const QString & text);

    void loader_error();
    void device_error(QSerialPort::SerialPortError e);
    void calibrate();
    void writeEmpty();

public:

    PropellerSession(QString port, int reset_gpio=-1, bool useRtsReset = false, QObject * parent = 0);
    ~PropellerSession();

    bool open();
    bool isOpen();
    void close();
    int version();
    void reset();
    void upload(PropellerImage binary, bool write=false, bool run=true);
    void terminal();

    static QStringList list_devices();
};

