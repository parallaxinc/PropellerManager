#include "propellerdevice.h"

#include <QDebug>
#include <QThread>
#include <QSerialPortInfo>

#include "gpio.h"

PropellerDevice::PropellerDevice(QObject * parent)
 : QSerialPort(parent)
{
    resource_error_count = 0;
    _minimum_timeout = 400;

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
        case QSerialPort::TimeoutError:                         // 12
            emit finished();
            break;
        case QSerialPort::ParityError:                          // 4
        case QSerialPort::FramingError:                         // 5
        case QSerialPort::BreakConditionError:                  // 6
        case QSerialPort::WriteError:                           // 7
        case QSerialPort::ReadError:                            // 8
        case QSerialPort::UnknownError:                         // 11
        case QSerialPort::ResourceError: // SUPER IMPORTANT     // 9
            resource_error_count++;
            if (resource_error_count > 1)
            {
                close();
                emit finished();
                sendError(QString("'%1' (error %2)").arg(errorString()).arg(e));
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
    Return the minimum timeout for downloading to the Propeller.

    The default value is 200 ms.
    */

quint32 PropellerDevice::minimumTimeout()
{
    return _minimum_timeout;
}

/**
    Set the minimum timeout for downloading to the Propeller.

    The default value is 200 ms.
    */

void PropellerDevice::setMinimumTimeout(quint32 milliseconds)
{
    _minimum_timeout = milliseconds;
}

/**
    The total timeout period is calculated as follows.
    
        timeout = bytes * bits_per_character / bits_per_second
                        * (1000ms / 1s) * safety_factor / 10
                        + minimumTimeout()

    safety_factor defaults to 10, and is divided by 10, allowing <1 safety factors.
  */

quint32 PropellerDevice::calculateTimeout(quint32 bytes, quint32 safety_factor)
{
    return bytes * (dataBits() + stopBits()) * safety_factor / 10
                 * 1000 / baudRate() + minimumTimeout();
}

/**
    Use a GPIO pin for hardware reset if available.

    Raspberry Pi builds of PropellerManager are configured to use
    GPIO reset by default for as this is the hardware configuration
    of the Propeller HAT.
    */
void PropellerDevice::useGpioReset(int pin)
{
    reset_gpio = pin;
    use_rts_reset = false;
}

/**
    Use RTS for hardware reset if available. Most Parallax products use DTR for reset,
    but some third-party boards use RTS as the reset pin.

    Refer to your board schematic for more information.
    */

void PropellerDevice::useRtsReset()
{
    reset_gpio = -1;
    use_rts_reset = true;
}

/**
    Use DTR for hardware reset if available. Most Parallax products use DTR for reset,
    but some third-party boards use RTS as the reset pin.

    Refer to your board schematic for more information.
    */

void PropellerDevice::useDtrReset()
{
    reset_gpio = -1;
    use_rts_reset = false;
}

/**
    Reset your attached Propeller hardware using the configured
    hardware reset.

    DTR is the default hardware reset on all PropellerIDE distributions,
    with the exception of Raspberry Pi, where it is GPIO reset.
    */

bool PropellerDevice::reset()
{
    clear(QSerialPort::Output);

    if (reset_gpio > -1)
    {
        Gpio gpio(reset_gpio, Gpio::Out);
        gpio.Write(Gpio::Low);
        gpio.Write(Gpio::High);
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

/**
    List all available hardware devices.

    This function does not check whether Propeller hardware attached to the given devices.
    */

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
