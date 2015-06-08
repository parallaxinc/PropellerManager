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
    void writeByte(quint8  value);
    void writeLong(quint32 value);

    QByteArray buildRequest(Command::Command command = Command::Shutdown);
    int _version;
    int ack;

    QByteArray reply;
    QByteArray real_reply;
    QByteArray request;

    bool sendPayload(QByteArray payload);
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

