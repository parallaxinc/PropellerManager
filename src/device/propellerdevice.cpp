#include "propellerdevice.h"

#include <QSerialPortInfo>
#include <QTimer>
#include <QEventLoop>

#include "gpio.h"
#include "../util/logging.h"

PropellerDevice::PropellerDevice()
 : Interface()
{
    _resource_error_count = 0;
    _minimum_timeout = 400;

    device.setSettingsRestoredOnClose(false);
    device.setBaudRate(115200);

    _reset_defaults["ttyAMA"]   = "gpio";
    _reset_defaults["ttyS"]     = "dtr";
    _reset_defaults["ttyUSB"]   = "dtr";

    _reset_gpio = 17;

    useDefaultReset();

    connect(&device,    SIGNAL(error(QSerialPort::SerialPortError)),
            this,       SLOT(handleError(QSerialPort::SerialPortError))); 

    connect(&device,    SIGNAL(bytesWritten(qint64)),       this,   SIGNAL(bytesWritten(qint64)));
    connect(&device,    SIGNAL(readyRead()),                this,   SIGNAL(readyRead()));

    connect(&device,    SIGNAL(baudRateChanged(qint32, QSerialPort::Directions)),
            this,       SIGNAL(baudRateChanged(qint32)));
}
    
PropellerDevice::~PropellerDevice()
{
    close();

    disconnect(&device,    SIGNAL(error(QSerialPort::SerialPortError)),
               this,       SLOT(handleError(QSerialPort::SerialPortError))); 

    disconnect(&device,    SIGNAL(bytesWritten(qint64)),   this,   SIGNAL(bytesWritten(qint64)));
    disconnect(&device,    SIGNAL(readyRead()),            this,   SIGNAL(readyRead()));

    disconnect(&device,    SIGNAL(baudRateChanged(qint32, QSerialPort::Directions)),
               this,       SIGNAL(baudRateChanged(qint32)));
}

void PropellerDevice::setPortName(const QString & name)
{
    device.setPortName(name);
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
            device.clearError();
            break;
        case QSerialPort::TimeoutError:                         // 12
            break;
        case QSerialPort::ParityError:                          // 4
        case QSerialPort::FramingError:                         // 5
        case QSerialPort::BreakConditionError:                  // 6
        case QSerialPort::WriteError:                           // 7
        case QSerialPort::ReadError:                            // 8
        case QSerialPort::UnknownError:                         // 11
        case QSerialPort::ResourceError: // SUPER IMPORTANT     // 9
            _resource_error_count++;
            if (_resource_error_count > 1)
            {
                close();
                emit sendError(QString("'%1' (error %2)").arg(device.errorString()).arg(e));
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
    _resource_error_count = 0;

    if (!device.open(QSerialPort::ReadWrite))
    {
        close();
        if (!device.open(QSerialPort::ReadWrite))
        {
            qCDebug(pdevice) << "Failed to open device:" << portName();
            return false;
        }
    }

    reset();

    return true;
}

void PropellerDevice::close()
{
    device.close();
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

quint32 PropellerDevice::calculateTimeout(quint32 bytes)
{
    return bytes * (device.dataBits() + device.stopBits()) * 25 / 10
                 * 1000 / baudRate() + minimumTimeout();
}

/**
    Set the reset strategy for your device.
    */

void PropellerDevice::useReset(QString name, int pin)
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
            device.setRequestToSend(true);

            QTimer wait;
            QEventLoop loop;
            connect(&wait, SIGNAL(timeout()), &loop, SLOT(quit()));
            wait.start(20);
            loop.exec();
            disconnect(&wait, SIGNAL(timeout()), &loop, SLOT(quit()));

            device.setRequestToSend(false);
        }
        else if (_reset == "dtr")
        {
            device.setDataTerminalReady(true);

            QTimer wait;
            QEventLoop loop;
            connect(&wait, SIGNAL(timeout()), &loop, SLOT(quit()));
            wait.start(20);
            loop.exec();
            disconnect(&wait, SIGNAL(timeout()), &loop, SLOT(quit()));

            device.setDataTerminalReady(false);
        } 
        else
        {
            qCDebug(pdevice) << "Unknown reset strategy:" << _reset;
        }
    }

    clear();

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


bool PropellerDevice::isOpen()
{
    if (!device.isOpen())
    {
        if (!open())
        {
            close();
            return open();
        }
    }
    return true;
}

bool PropellerDevice::clear()
{
    device.clear();
    device.readAll();      // clear doesn't appear to actually do anything
    return true;
}

bool PropellerDevice::setBaudRate(quint32 baudRate)
{
    return device.setBaudRate(baudRate);
}

QString PropellerDevice::portName()
{
    return device.portName();
}

quint32 PropellerDevice::baudRate()
{
    return device.baudRate();
}

qint64 PropellerDevice::bytesToWrite()
{
    return device.bytesToWrite();
}

qint64 PropellerDevice::bytesAvailable()
{
    return device.bytesAvailable();
}

QByteArray PropellerDevice::read(qint64 maxSize)
{
    return device.read(maxSize);
}

QByteArray PropellerDevice::readAll()
{
    return device.readAll();
}

bool PropellerDevice::putChar(char c)
{
    return device.putChar(c);
}
qint64 PropellerDevice::write(QByteArray ba)
{
    return device.write(ba);
}

