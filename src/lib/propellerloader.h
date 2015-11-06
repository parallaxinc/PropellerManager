#pragma once

#include "propellerimage.h"
#include "propellersession.h"
#include "propellerprotocol.h"

#include <QTimer>
#include <QElapsedTimer>

/**
@class PropellerLoader 

@brief The PropellerLoader class implements the Propeller download protocol.

PropellerLoader automatically selects the best download strategy based on the
given image to download and the target device.

- If downloading to an XBee device, high-speed loading must be used. If a 
  crystal oscillator is not defined, PropellerManager errors.
- If downloading through a serial connection, use high-speed download if a
  crystal oscillator is defined, otherwise use the basic protocol.

PropellerDevice selects the reset strategy based on the port name. This can be overridden via the useReset() function.
At present, all devices assume DTR reset as the default, except ttyAMA as this is specific to the ARM architecture and uses GPIO.
*/

/**
@example identify/main.cpp

This example queries all available ports for connected Propeller devices.
*/

/**
@example download/main.cpp

This example downloads a Propeller image to the first availabe device.
*/

class PropellerLoader : public QObject
{
    Q_OBJECT

private:
    PropellerSession * session;
    PropellerProtocol protocol;

    int _version;
    int _ack;

    QTimer totalTimeout;
    QTimer handshakeTimeout;
    QTimer timeoutAlarm;
    QTimer poll;
    QElapsedTimer elapsedTimer;

    void writeByte(quint8  value);
    void writeLong(quint32 value);
    bool sendPayload(QByteArray payload);
    int pollAcknowledge();
    bool isUploadSuccessful();

signals:
    void finished();

private slots:
    void read_handshake();
    void read_acknowledge();

    void calibrate();
    void timeover();
    void timestamp();

    void message(QString text);
    void error(QString text);

public:
    PropellerLoader(PropellerManager * manager,
                    const QString & portname,
                    QObject * parent = 0);
    ~PropellerLoader();

    int version();
    int upload(PropellerImage image, bool write=false, bool run=true);
    int highSpeedUpload(PropellerImage image, bool write=false, bool run=true);
};

