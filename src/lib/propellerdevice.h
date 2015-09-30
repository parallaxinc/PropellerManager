#pragma once

#include <QSerialPort>

class PropellerDevice
    : public QSerialPort
{
    Q_OBJECT

    int resource_error_count;
    int reset_gpio;
    int use_rts_reset;
    quint32 _minimum_timeout;

public:
    PropellerDevice(QObject * parent = 0);
    ~PropellerDevice();
    bool open();

    static QStringList list();
    bool reset();
    void useGpioReset(int pin);
    void useRtsReset();
    void useDtrReset();

    quint32 calculateTimeout(quint32 bytes, quint32 safety_factor = 15);
    quint32 minimumTimeout();
    void setMinimumTimeout(quint32 milliseconds);

private slots:
    void handleError(QSerialPort::SerialPortError e);
    void writeBufferEmpty();
    void timeOver();

signals:
    void finished();
    void sendError(const QString &);
};
