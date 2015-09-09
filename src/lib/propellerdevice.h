#pragma once

#include <QSerialPort>

class PropellerDevice
    : public QSerialPort
{
    Q_OBJECT

    int resource_error_count;
    int reset_gpio;
    int use_rts_reset;

public:
    PropellerDevice(QObject * parent = 0);
    ~PropellerDevice();
    bool open();

    static QStringList list();
    bool reset();
    void useGpioReset(int pin);
    void useRtsReset();
    void useDtrReset();

private slots:
    void handleError(QSerialPort::SerialPortError e);
    void writeBufferEmpty();
    void timeOver();

signals:
    void finished();

};
