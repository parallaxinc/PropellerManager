#pragma once

#include <QSerialPort>
#include <QStringList>
#include <QHash>

/**
    @class PropellerDevice
  
    @brief this class encapsulates a Propeller hardware device.

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
    bool reset();
    void useReset(const QString & name, int pin = 17);
    void useDefaultReset();
    void setPortName(const QString & name);

    quint32 calculateTimeout(quint32 bytes, quint32 safety_factor = 15);
    quint32 minimumTimeout();
    void setMinimumTimeout(quint32 milliseconds);

public slots:
    void handleError(QSerialPort::SerialPortError e);
    void writeBufferEmpty();
    void timeOver();

signals:
    void finished();
    void sendError(const QString &);
};
