#pragma once

#include "input_console.h"
#include "propellerimage.h"

#include "propellerdevice.h"
#include <QTimer>

namespace Command {
    enum Command {
        Shutdown,
        Run,
        Write,
        WriteRun
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
    PropellerDevice device;
    void writeByte(char value);
    void writeLong(unsigned int value);
    int handshake();

    int lfsr(int * seed);
    QList<char> sequence;
    QList<char> buildLfsrSequence(int size);

    QByteArray request;
    QByteArray reply;
    QByteArray real_reply;

    QByteArray buildRequest(QList<char> seq, int size);
    QByteArray buildReply(  QList<char> seq, int size, int offset);
    int _version;
    int ack;

    QByteArray encodeApplicationImage(PropellerImage image);
    int sendApplicationImage(QByteArray encoded_image, int image_size);
    int pollAcknowledge();

    QByteArray encodeLong(unsigned int value);

    QTimer poll;
    Input::Console console;

signals:
    void finished();

private slots:
    void read_handshake();
    void read_acknowledge();
    void read_terminal();
    void write_terminal(const QString & text);

    void calibrate();

public:

    PropellerSession(QString port, int reset_gpio=-1, bool useRtsReset = false, QObject * parent = 0);
    ~PropellerSession();

    bool open();
    bool isOpen();
    void close();
    int version();
    void upload(PropellerImage binary, bool write=false, bool run=true);
    int terminal();

};

