#pragma once

#include "input_console.h"
#include "propellerimage.h"
#include "propellerdevice.h"
#include "propellerprotocol.h"

#include <QTimer>
#include <QElapsedTimer>

/**
@class PropellerSession 

@brief a high-level interface for managing a single PropellerDevice.

PropellerSession provides a persistent environment from which to
interact with the Propeller. Any functionality relevant to the Propeller
can be performed from this class: RAM and EEPROM downloads, 
terminal sessions, and identification of attached hardware, can all be
done without having to disconnect from the attached device.

This ensures that the program always behaves as expected and makes it
possible to debug the Propeller in RAM on non-Windows platforms.
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
    int _ack;

    bool sendPayload(QByteArray payload);
    int pollAcknowledge();

    QTimer totalTimeout;
    QTimer handshakeTimeout;
    QTimer timeoutAlarm;
    QTimer poll;
    QElapsedTimer elapsedTimer;
    Input::Console console;

signals:
    void finished();

private slots:
    void read_handshake();
    void read_acknowledge();
    void read_terminal();
    void write_terminal(const QString & text);

    void calibrate();
    void timeover();
    void timestamp();

    void message(QString text);
    void error(QString text);

public:

    PropellerSession(QString port, int reset_gpio=-1, bool useRtsReset = false, QObject * parent = 0);
    ~PropellerSession();

    bool open();
    bool isOpen();
    void close();
    int version();
    int upload(PropellerImage image, bool write=false, bool run=true);
    bool isUploadSuccessful();
    int highSpeedUpload(PropellerImage image, bool write=false, bool run=true);
    int terminal();
};

