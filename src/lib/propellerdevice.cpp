#include "propellerdevice.h"

#include <QDebug>
#include <QThread>
#include <QSerialPortInfo>

#include "gpio.h"

PropellerDevice::PropellerDevice(QObject * parent)
 : QSerialPort(parent)
{
    resource_error_count = 0;
    _minimum_timeout = 200;

    setSettingsRestoredOnClose(false);
    useDtrReset();

    connect(this,SIGNAL(error(QSerialPort::SerialPortError)),
            this,  SLOT(handleError(QSerialPort::SerialPortError))); 
}
    

PropellerDevice::~PropellerDevice()
{
    close();
}


void PropellerDevice::writeBufferEmpty()
{
    if (!bytesToWrite())
    {
//        qDebug() << "Download finished";

        if (!bytesAvailable())
            emit finished();
    }
}


void PropellerDevice::handleError(QSerialPort::SerialPortError e)
{
    switch (e)
    {
        case QSerialPort::NoError:                              // 0
            break;

        case QSerialPort::DeviceNotFoundError:                  // 1
        case QSerialPort::PermissionError:                      // 2
        case QSerialPort::NotOpenError:                         // 13
        case QSerialPort::UnsupportedOperationError:            // 10
            clearError();
            break;
        case QSerialPort::ParityError:                          // 4
        case QSerialPort::FramingError:                         // 5
        case QSerialPort::BreakConditionError:                  // 6
        case QSerialPort::WriteError:                           // 7
        case QSerialPort::ReadError:                            // 8
        case QSerialPort::UnknownError:                         // 11
        case QSerialPort::TimeoutError:                         // 12
        case QSerialPort::ResourceError: // SUPER IMPORTANT     // 9
            resource_error_count++;
            if (resource_error_count > 1)
            {
                close();
                emit finished();
                sendError(QString("ERROR %1: %2").arg(e).arg(errorString()));
            }
            break;
        default:
            break;
    }
}

/**
    Open the PropellerSession for use.
  */
bool PropellerDevice::open()
{
    resource_error_count = 0;

    if (!QSerialPort::open(QSerialPort::ReadWrite))
    {
        return false;
    }

    setBaudRate(115200);

    return true;
}

/**
    The timeout period is calculated as follows.
    
        timeout = bytes * bits_per_character / bits_per_second
                        * (1000ms / 1s) * safety_factor / 10;

    safety_factor defaults to 10, and is divided by 10, allowing <1 safety factors.
  */

quint32 PropellerDevice::minimumTimeout()
{
    return _minimum_timeout;
}

void PropellerDevice::setMinimumTimeout(quint32 milliseconds)
{
    _minimum_timeout = milliseconds;
}

quint32 PropellerDevice::calculateTimeout(quint32 bytes, quint32 safety_factor)
{
    return bytes * (dataBits() + stopBits()) * safety_factor / 10
                 * 1000 / baudRate() + minimumTimeout();
}

void PropellerDevice::useGpioReset(int pin)
{
    reset_gpio = pin;
    use_rts_reset = false;
}

void PropellerDevice::useRtsReset()
{
    reset_gpio = -1;
    use_rts_reset = true;
}

void PropellerDevice::useDtrReset()
{
    reset_gpio = -1;
    use_rts_reset = false;
}


bool PropellerDevice::reset()
{
    clear(QSerialPort::Output);

    if (reset_gpio > -1)
    {
        GPIO gpio(reset_gpio, GPIO::Out);
        gpio.Write(GPIO::Low);
        gpio.Write(GPIO::High);
    }
    else
    {
        if (use_rts_reset)
        {
            setRequestToSend(true);
            setRequestToSend(false);
        }
        else
        {
            setDataTerminalReady(true);
            setDataTerminalReady(false);
        }
    }

    QThread::msleep(80);

    clear(QSerialPort::Input);

    return true;
}

QStringList PropellerDevice::list()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QStringList result;

    foreach (QSerialPortInfo port, ports)
    {
        if (!port.systemLocation().contains("ttyS") &&
            !port.systemLocation().contains("Bluetooth"))
            result.append(port.portName());
    }
    return result;
}


void PropellerDevice::timeOver()
{
    qDebug() << "Time over";
    emit error(QSerialPort::TimeoutError);
}
