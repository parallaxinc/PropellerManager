#include "propellerdevice.h"

#include <QDebug>
#include <QSerialPortInfo>
#include <QTimer>
#include <QEventLoop>

#include "gpio.h"
#include "../util/logging.h"

PropellerDevice::PropellerDevice(QObject * parent)
 : QSerialPort(parent)
{
    resource_error_count = 0;
    _minimum_timeout = 400;

    setSettingsRestoredOnClose(false);

    _reset_defaults["ttyAMA"]   = "gpio";
    _reset_defaults["ttyS"]     = "dtr";
    _reset_defaults["ttyUSB"]   = "dtr";

    _reset_gpio = 17;

    useDefaultReset();

    connect(this,SIGNAL(error(QSerialPort::SerialPortError)),
            this,  SLOT(handleError(QSerialPort::SerialPortError))); 
}
    

PropellerDevice::~PropellerDevice()
{
    close();
}


void PropellerDevice::setPortName(const QString & name)
{
    QSerialPort::setPortName(name);
    useDefaultReset();
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
    Open the PropellerDevice for use.
  */
bool PropellerDevice::open()
{
    resource_error_count = 0;

    if (!QSerialPort::open(QSerialPort::ReadWrite))
    {
        qCDebug(pdevice) << "Reattempting device open:" << portName();

        QTimer wait;
        QEventLoop loop;
        connect(&wait, SIGNAL(timeout()), &loop, SLOT(quit()));
        wait.start(1000);
        loop.exec();
        disconnect(&wait, SIGNAL(timeout()), &loop, SLOT(quit()));

        if (!QSerialPort::open(QSerialPort::ReadWrite))
        {
            qCDebug(pdevice) << "Failed to open device:" << portName();
            return false;
        }
    }

    reset();
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
    Set the reset strategy for your device.
    */

void PropellerDevice::useReset(const QString & name, int pin)
{
    if (_reset_defaults.contains(name))
    {
        _reset = name;
        _reset_gpio = pin;
    }
    else
    {
        qCDebug(pdevice) << "Invalid reset type:" << name;
    }
}

/**
    Use the default reset strategy for your device.

    Most Parallax products use DTR for reset,
    but some third-party boards use RTS as the reset pin.

    Raspberry Pi builds of PropellerManager are configured to use
    GPIO reset by default for as this is the hardware configuration
    of the Propeller HAT.
    */

void PropellerDevice::useDefaultReset()
{
    foreach (QString s, _reset_defaults.keys())
    {
        if (portName().startsWith(s))
        {
            _reset = _reset_defaults[s];
            return;
        }
    }
    _reset = "dtr";
}

/**
    Reset your attached Propeller hardware using the configured
    hardware reset.

    DTR is the default hardware reset on all PropellerIDE distributions,
    with the exception of Raspberry Pi, where it is GPIO reset.
    */

bool PropellerDevice::reset()
{
    if (_reset == "gpio")
    {
        Gpio gpio(_reset_gpio, Gpio::Out);
        gpio.Write(Gpio::Low);
        gpio.Write(Gpio::High);
    }
    else
    {
        if (_reset == "rts")
        {
            setRequestToSend(true);
            setRequestToSend(false);
        }
        else if (_reset == "dtr")
        {
            setDataTerminalReady(true);
            setDataTerminalReady(false);
        } 
        else
        {
            qCDebug(pdevice) << "Unknown reset strategy:" << _reset;
        }
    }

    clear();
    readAll();      // clear doesn't appear to actually do anything

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

quint32 PropellerDevice::resetPeriod()
{
    return 80;
}
