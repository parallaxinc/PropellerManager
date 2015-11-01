#pragma once

#include "propellerimage.h"
#include "propellersession.h"
#include "propellerprotocol.h"

#include <QTimer>
#include <QElapsedTimer>

/**
@class PropellerLoader 

@brief a high-level interface for managing a single PropellerSession.

PropellerLoader provides a persistent environment from which to
interact with the Propeller. Any functionality relevant to the Propeller
can be performed from this class: RAM and EEPROM downloads, 
terminal sessions, and identification of attached hardware, can all be
done without having to disconnect from the attached session.

This ensures that the program always behaves as expected and makes it
possible to debug the Propeller in RAM on non-Windows platforms.
*/
class PropellerLoader : public QObject
{
    Q_OBJECT

private:
    PropellerSession * session;
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

    PropellerLoader(PropellerSession * session, QObject * parent = 0);
    ~PropellerLoader();

    int version();
    int upload(PropellerImage image, bool write=false, bool run=true);
    int highSpeedUpload(PropellerImage image, bool write=false, bool run=true);
};

