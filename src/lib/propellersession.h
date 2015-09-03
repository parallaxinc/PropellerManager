#pragma once

#include "input_console.h"
#include "propellerimage.h"
#include "propellerdevice.h"
#include "propellerprotocol.h"

#include <QTimer>

/**
  @class PropellerSession 
  
  @brief an interface for connecting to and managing a single Propeller device.

  */
class PropellerSession : public QObject
{
    Q_OBJECT

private:
    PropellerDevice device;
    PropellerProtocol protocol;

    void writeByte(quint8  value);
    void writeLong(quint32 value);

    int _version;
    int ack;

    bool sendPayload(QByteArray payload);
    int pollAcknowledge();

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
    void upload(PropellerImage image, bool write=false, bool run=true);
    void highSpeedUpload(PropellerImage image, bool write=false, bool run=true);
    int terminal();

};

