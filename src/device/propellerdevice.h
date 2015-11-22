#pragma once

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

class PropellerDevice
    : public QSerialPort
{
    Q_OBJECT

    int resource_error_count;
    quint32 _minimum_timeout;

    QString _reset;
    QHash<QString, QString> _reset_defaults;
    int _reset_gpio;

public:
    PropellerDevice(QObject * parent = 0);
    ~PropellerDevice();
    bool open();

    static QStringList list();
    void useReset(const QString & name, int pin = 17);
    void useDefaultReset();
    void setPortName(const QString & name);

    quint32 calculateTimeout(quint32 bytes, quint32 safety_factor = 15);
    quint32 minimumTimeout();
    void setMinimumTimeout(quint32 milliseconds);

public slots:
    void handleError(QSerialPort::SerialPortError e);
    bool reset();

signals:
    void finished();
    void sendError(const QString &);
};
