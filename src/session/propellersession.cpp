#include "propellersession.h"

#include <QDebug>

#include "../device/propellerdevice.h"

class PropellerManager;

PropellerSession::PropellerSession(
        PropellerManager * manager,
        const QString & portname, 
        QObject * parent)
    : QObject(parent)
{
    setPortName(portname);
    this->manager = manager;

    manager->beginSession(this);
}

PropellerSession::~PropellerSession()
{
    manager->endSession(this);
}

/**
    Returns the name of the current device.
  */
const QString & PropellerSession::portName()
{
    return port;
}

/**
    Sets the name of the current device.
  */
void PropellerSession::setPortName(const QString & name)
{
    port = name;
}

/**
    Returns true if device is open, or false if not available or busy.
  */
bool PropellerSession::isOpen()
{
    return manager->isOpen(this);
}

/**
    Reserve the device for exclusive access.

    \returns true if successfully reserved, otherwise false.
  */
bool PropellerSession::reserve()
{
    return manager->reserve(this);
}

/**
    Returns true if this session has reserved the device, otherwise false.
  */
bool PropellerSession::isReserved()
{
    return manager->isReserved(this);
}

/**
    Releases the device so that other sessions can use it.
  */
void PropellerSession::release()
{
    manager->release(this);
}

/**
    Prevent session from sending or receiving data temporarily.
  */
void PropellerSession::pause()
{
    manager->pause(this);
}

/**
    Returns true if session is paused, otherwise returns false.
  */
bool PropellerSession::isPaused()
{
    return manager->isPaused(this);
}

/**
    Resuming sending and receiving data.
  */
void PropellerSession::unpause()
{
    manager->unpause(this);
}

/**
    Clear send and receive buffers for this session.
  */

bool PropellerSession::clear()
{
    return manager->clear(this);
}

/**
    Set the baud rate of the device.

    \note This command can be used at any time, unlike some
          serial terminal implementations.
  */

bool PropellerSession::setBaudRate(quint32 baudRate)
{
    return manager->setBaudRate(this, baudRate);
}

/**
    Returns the number of bytes waiting to be written to the device.

    This function returns 0 if the device is busy.
  */

qint64 PropellerSession::bytesToWrite()
{
    return manager->bytesToWrite(this);
}

/**
    Returns the number of bytes waiting to be read from the device.

    This function returns 0 if the device is busy.
  */

qint64 PropellerSession::bytesAvailable()
{
    return manager->bytesAvailable(this);
}

/**
    Reads the number of byte specified by maxSize from the device.

    \returns A QByteArray containing the data, or an empty one if
             no data was available or the device was not available.
  */

QByteArray PropellerSession::read(qint64 maxSize)
{
    return manager->read(this, maxSize);
}

/**
    Reads all of the available data from the device.

    \returns A QByteArray containing the data, or an empty one if
             there was no data or the device was not available.
  */

QByteArray PropellerSession::readAll()
{
    return manager->readAll(this);
}

/**
    Writes a single byte of data to the device.

    \returns
    True if the command was successful, otherwise returns false.
  */

bool PropellerSession::putChar(char c)
{
    return manager->putChar(this, c);
}

/**
    Writes a QByteArray of data to the device.

    \returns The number of bytes written if successful, otherwise
             returns 0.
  */

qint64 PropellerSession::write(const QByteArray & byteArray)
{
    return manager->write(this, byteArray);
}

/**
    Returns the minimum timeout for this device.
  */

quint32 PropellerSession::minimumTimeout()
{
    return manager->minimumTimeout(this);
}

/**
    Sets the minimum timeout for this device.
  */

void PropellerSession::setMinimumTimeout(quint32 milliseconds)
{
    manager->setMinimumTimeout(this, milliseconds);
}

/**
    Calculates a reasonable timeout value for writing the given number of bytes to the device.

    \returns the timeout value in milliseconds.
  */

quint32 PropellerSession::calculateTimeout(quint32 bytes)
{
    return manager->calculateTimeout(this, bytes);
}

/**
    Force the use of a specific reset strategy on this platform.

    This function is usually not needed, but might be necessary
    if building PropellerManager for a platform that is not
    current supported.
  */

void PropellerSession::useReset(const QString & name, int pin)
{
    manager->useReset(this, name, pin);
}

/**
    Use the default reset strategy for this platform.

    This command is not needed unless you have previously used
    useReset.
  */

void PropellerSession::useDefaultReset()
{
    manager->useDefaultReset(this);
}

/**
    Reset the connected device.

    \returns true if successful, false if not found or busy.
  */

bool PropellerSession::reset()
{
    return manager->reset(this);
}

quint32 PropellerSession::resetPeriod()
{
    return manager->resetPeriod(this);
}

/**
    Returns the error of the device.
  */

int PropellerSession::error()
{
    return manager->error(this);
}

/**
    Returns the error string of the device.
  */

QString PropellerSession::errorString()
{
    return manager->errorString(this);
}

/**
    \fn void PropellerSession::bytesWritten(qint64 bytes)

    This signal is emitted once every time data has been
    written to the device. The bytes argument is the number of
    bytes that were written.
    */

/**
    \fn void PropellerSession::readyRead()

    This signal is emitted every time new data is available for
    reading from the device.
    */

/**
    \fn void PropellerSession::baudRateChanged(qint32 baudRate)

    This signal is emitted after the baud rate has been changed.
    The new baud rate is passed as baudRate.
    */

/**
    \fn void PropellerSession::deviceFree()

    This signal is emitted when the device becomes free for use again.
    */

/**
    \fn void PropellerSession::deviceBusy()

    This signal is emitted when the device has been reserved by another
    session.
    */
