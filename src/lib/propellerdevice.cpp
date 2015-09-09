#include "propellerdevice.h"

#include <QDebug>
#include <QThread>
#include <QSerialPortInfo>

#include "gpio.h"

PropellerDevice::PropellerDevice(QObject * parent)
 : QSerialPort(parent)
{
    resource_error_count = 0;

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
//    qDebug() << "   " << bytesToWrite() << "...";
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
        case QSerialPort::NoError:
            break;
        case QSerialPort::DeviceNotFoundError:
        case QSerialPort::PermissionError:
        case QSerialPort::NotOpenError:
            break;
        case QSerialPort::ParityError:
        case QSerialPort::FramingError:
        case QSerialPort::BreakConditionError:
        case QSerialPort::WriteError:
        case QSerialPort::ReadError:
        case QSerialPort::UnsupportedOperationError:
        case QSerialPort::TimeoutError:
        case QSerialPort::UnknownError:
        case QSerialPort::ResourceError: // SUPER IMPORTANT
            resource_error_count++;
            if (resource_error_count > 1)
            {
                close();
                emit finished();
                emit error(e);
                qDebug() << "ERROR" << e << ": device unexpectedly disconnected!";
            }
            break;
        default:
            break;
    }
}


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
    emit error(QSerialPort::TimeoutError);
}
