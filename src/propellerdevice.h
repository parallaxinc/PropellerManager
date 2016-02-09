#pragma once

#include "template/interface.h"

#include <QSerialPort>
#include <QStringList>
#include <QHash>

/**
    @class PropellerDevice device/propellerdevice.h PropellerDevice
  
    @brief The PropellerDevice class provides access to serial Propeller devices.

    This class inherits from QSerialPort but provides Propeller
    specific functionality such as hardware reset and download
    timeouts.
  */

namespace PM
{

class PropellerDevice : public Interface
{
    Q_OBJECT
    
    QSerialPort device;

    QHash<QString, QString> _reset_defaults;

    int         _resource_error_count;
    quint32     _minimum_timeout;

    QString     _reset;
    int         _reset_gpio;

    bool        _enabled;

private slots:
    void        handleError(QSerialPort::SerialPortError e);

public:
    PropellerDevice(QString devicename = QString());
    ~PropellerDevice();

    static      QStringList list();

    void        setEnabled(bool enabled);
    bool        enabled();

    bool        open();
    void        close();
    bool        isOpen();

    bool        clear();
    bool        setBaudRate(quint32 baudRate);

    QString     portName();
    quint32     baudRate();
    qint64      bytesToWrite();
    qint64      bytesAvailable();
    QByteArray  read(qint64 maxSize);
    QByteArray  readAll();
    bool        putChar(char c);
    qint64      write(QByteArray ba);

    quint32     minimumTimeout();
    void        setMinimumTimeout(quint32 milliseconds);
    quint32     calculateTimeout(quint32 bytes);

    void        useReset(QString name, int pin = 17);
    void        useDefaultReset();
    bool        reset();
    quint32     resetPeriod();

};

}
