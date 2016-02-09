#pragma once

#include "propellerimage.h"
#include "propellersession.h"
#include "protocol.h"

#include <QTimer>
#include <QElapsedTimer>
#include <QStateMachine>
#include <QFinalState>
#include <QState>

/**
@class PropellerLoader loader/propellerloader.h PropellerLoader

@brief The PropellerLoader class provides an implementation of the Propeller download protocol.

PropellerLoader automatically selects the best download strategy based on the
given image to download and the target device.

- If downloading to an XBee device, high-speed loading must be used. If a 
  crystal oscillator is not defined, PropellerManager errors.
- If downloading through a serial connection, use high-speed download if a
  crystal oscillator is defined, otherwise use the basic protocol.

PropellerDevice selects the reset strategy based on the port name. This can be overridden via the useReset() function.
At present, all devices assume DTR reset as the default, except ttyAMA as this is specific to the ARM architecture and uses GPIO.

\see PropellerTerminal
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
    Q_PROPERTY(QString status MEMBER m_status NOTIFY statusChanged)
    Q_PROPERTY(int stat MEMBER m_stat)

public:
    enum LoaderError
    {
        NoError,
        DeviceBusyError,
        DeviceNotOpenError,
        DeviceNotFoundError,
        DownloadInProgressError,
        InvalidImageError,
        VerifyRamError,
        WriteEepromError,
        VerifyEepromError,
        TimeoutError,
        HandshakeError,
        InvalidHandshakeError,
        UnknownError
    };

private:
    PropellerSession * session;
    PropellerProtocol protocol;
    PropellerImage _image;

    int _command;
    int _completed;
    QString m_status;
    int m_stat;
    
    QByteArray _payload;

    QStateMachine machine;
    QState * s_active;

    LoaderError _error;
    QHash<LoaderError, QString> _errorstrings;

    int _version;
    QHash<int, QString> _versionstrings;

    int _ack;
    int _write, _run;

    QTimer totalTimeout;
    QTimer handshakeTimeout;
    QTimer resetTimer;
    QTimer poll;
    QElapsedTimer elapsedTimer;

    void writeLong(quint32 value);

signals:
    void finished();
    void success();
    void failure();

    void prepared();
    void payload_sent();
    void handshake_received();
    void upload_completed();
    void acknowledged();

    void statusChanged(const QString & message);

private slots:

    void failure_entry();
    void success_entry();

    void prepare_entry();

    void sendpayload_entry();
    void sendpayload_exit();

    void sendpayload_write();
    void handshake_read();

    void upload_status();

    void acknowledge_entry();
    void acknowledge_exit();
    void acknowledge_read();

    void calibrate();
    void timeover();
    void timestamp();

    void message(const QString & text);
    void error(const QString & text);

public:
    PropellerLoader(PropellerManager * manager,
                    const QString & portname = QString(),
                    QObject * parent = 0);
    ~PropellerLoader();

    int version();
    QString versionString(int version);

    bool upload(PropellerImage image, bool write=false, bool run=true, bool wait=false);
//    bool highSpeedUpload(PropellerImage image, bool write=false, bool run=true);
};

