#include "propellermanager.h"

#include <QDebug>

#include "../util/logging.h"


PropellerManager::PropellerManager(QObject * parent)
    : QObject(parent)
{
    _ports = PropellerDevice::list();
}

PropellerManager::~PropellerManager()
{

}

bool PropellerManager::beginSession(PropellerSession * session)
{
    if (!session) return false;

    _sessions[session] = session;
    return true;
}

void PropellerManager::endSession(PropellerSession * session)
{
    if (!session) return;

    QString port = session->portName();
    if (!_devices.contains(port)) return;

    detach(_sessions[session], _devices[port]);
    _sessions.remove(session);
}


PropellerDevice * PropellerManager::getDevice(const QString & port, bool open)
{
    if (!_devices.contains(port))
    {
        _devices[port] = new PropellerDevice();
        _devices[port]->setPortName(port);

        connect(_devices[port], &PropellerDevice::readyRead,
                          this, &PropellerManager::readyBuffer);
    }

    if (!_ports.contains(port))
    {
        qCDebug(pmanager) << "ERROR: Device does not exist:" << port;
        return _devices[port];
    }

    if (!_devices[port]->isOpen() && open)
    {
        qCDebug(pmanager) << "Opening" << _devices[port];
        _devices[port]->open();
    }

    return _devices[port];
}

void PropellerManager::attachByName(PropellerSession * session, const QString & port)
{
    PropellerDevice * olddevice = _connections[session];
    PropellerDevice * newdevice = getDevice(port);

    if (olddevice != NULL)
    {
        if (olddevice != newdevice) 
        {
            detach(session, olddevice);
            attach(session, newdevice);
        }
    }
    else
    {
        attach(session, newdevice);
    }
}

void PropellerManager::readyBuffer()
{
    PropellerDevice * device = (PropellerDevice *) sender();
    QByteArray newdata = device->readAll();

    foreach (PropellerSession * session, _connections.keys(device))
    {
        _buffers[session]->append(newdata);
    }
}

void PropellerManager::attach(PropellerSession * session, PropellerDevice * device)
{
    _buffers[session] = new ReadBuffer();

    _connections[session] = device;
    _active_sessions[device]++;

    qCDebug(pmanager) << "attaching" << session << "to" << _devices.key(device);
    // signals

    // pass-through
    connect(device,  &PropellerDevice::finished,        session,  &PropellerSession::finished);
    connect(device,  &PropellerDevice::sendError,       session,  &PropellerSession::sendError);
    connect(device,  &PropellerDevice::bytesWritten,    session,  &PropellerSession::bytesWritten);
    connect(device,  &PropellerDevice::baudRateChanged, session,  &PropellerSession::baudRateChanged);

    // buffered
    connect(_buffers[session],  &ReadBuffer::readyRead,   session,  &PropellerSession::readyRead);

    // slots
    connect(session, &PropellerSession::timeover,        device,  &PropellerDevice::timeOver);
    connect(session, &PropellerSession::allBytesWritten, device,  &PropellerDevice::writeBufferEmpty);

}

void PropellerManager::detach(PropellerSession * session, PropellerDevice * device)
{
    qCDebug(pmanager) << "detaching" << session << "from" << _devices.key(device);
    // signals

    // pass-through
    disconnect(device,  &PropellerDevice::finished,        session,  &PropellerSession::finished);
    disconnect(device,  &PropellerDevice::sendError,       session,  &PropellerSession::sendError);
    disconnect(device,  &PropellerDevice::bytesWritten,    session,  &PropellerSession::bytesWritten);
    disconnect(device,  &PropellerDevice::baudRateChanged, session,  &PropellerSession::baudRateChanged);

    // buffered
    disconnect(_buffers[session],  &ReadBuffer::readyRead,   session,  &PropellerSession::readyRead);

    // slots
    disconnect(session, &PropellerSession::timeover,        device,  &PropellerDevice::timeOver);
    disconnect(session, &PropellerSession::allBytesWritten, device,  &PropellerDevice::writeBufferEmpty);

    _active_sessions[device]--;
    _connections.remove(session);

    delete _buffers[session];
    _buffers.remove(session);


    if (!_active_sessions[device])
    {
        qCDebug(pmanager) << "closing" << _devices.key(device);
        device->close();
    }
}

void PropellerManager::deleteDevice(const QString & port)
{
    foreach(PropellerSession * s, _sessions)
    {
        if (port == s->portName())
            detach(_sessions[s], _devices[port]);
    }

    disconnect(_devices[port], &PropellerDevice::readyRead,
                         this, &PropellerManager::readyBuffer);
    _devices.remove(port);
}

/**
    Lists all available devices on the current system via any interface.
    */

const QStringList & PropellerManager::listPorts()
{
    return _ports;
}

void PropellerManager::enablePortMonitor(bool enabled)
{
    if (enabled)
    {
        checkPorts();
        connect(&portMonitor, SIGNAL(timeout()), this, SLOT(checkPorts()));
        portMonitor.start(200);
    }
    else
    {
        portMonitor.stop();
        disconnect(&portMonitor, SIGNAL(timeout()), this, SLOT(checkPorts()));
    }
}

void PropellerManager::checkPorts()
{
    QStringList newports = PropellerDevice::list();

    if(_ports != newports)
    {
        _ports = newports;
        emit portListChanged();
    }
}

bool PropellerManager::portIsBusy(PropellerSession * session, const QString & name)
{
    if (isPaused(session)) return true;

    PropellerDevice * device = getDevice(name, false);
    if (_busy.contains(device) && _busy[device] != session)
        return true;

    attachByName(session, name);
    return false;
}

bool PropellerManager::reserve(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return false;
    PropellerDevice * device = getDevice(port);
    _active_sessions[device]++; // prevent port from closing while reserving device

    foreach (PropellerSession * oldsession, _connections.keys(device))
    {
        _saved_connections[oldsession] = device;
        detach(oldsession, device);

        emit oldsession->deviceBusy();
    }

    attach(session, device);

    _active_sessions[device]--; // prevent port from closing while reserving device
    return true;
}

bool PropellerManager::isReserved(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return false;

    PropellerDevice * device = getDevice(port);
    if (_connections.keys(device).size() != 1) return false;

    return true;
}

void PropellerManager::release(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return;
    if (!isReserved(session, port)) return;

    PropellerDevice * device = getDevice(port);
    _active_sessions[device]++; // prevent port from closing while releasing device

    detach(session, device);

    foreach (PropellerSession * oldsession, _saved_connections.keys(device))
    {
        attach(oldsession, device);
        emit oldsession->deviceFree();
        _saved_connections.remove(oldsession);
    }

    _active_sessions[device]--; // prevent port from closing while releasing device
}

void PropellerManager::pause(PropellerSession * session)
{
    _paused[session] = session;
}

bool PropellerManager::isPaused(PropellerSession * session)
{
    return _paused.contains(session);
}

void PropellerManager::unpause(PropellerSession * session)
{
    _paused.remove(session);
}

bool PropellerManager::clear(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port)->clear();
}

bool PropellerManager::isOpen(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port, false)->isOpen();
}

bool PropellerManager::setBaudRate(PropellerSession * session, const QString & port, quint32 baudRate)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port)->setBaudRate(baudRate);
}

qint64 PropellerManager::bytesToWrite(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return 0;
    return getDevice(port)->bytesToWrite();
}

qint64 PropellerManager::bytesAvailable(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return 0;
    return _buffers[session]->bytesAvailable();
}

QByteArray PropellerManager::read(PropellerSession * session, const QString & port, qint64 maxSize)
{
    if (portIsBusy(session, port)) return QByteArray();
    return _buffers[session]->read(maxSize);
}

QByteArray PropellerManager::readAll(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return QByteArray();
    return _buffers[session]->readAll();
}

bool PropellerManager::putChar(PropellerSession * session, const QString & port, char c)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port)->putChar(c);
}

qint64 PropellerManager::write(PropellerSession * session, const QString & port, const QByteArray & byteArray)
{
    if (portIsBusy(session, port)) return -1;
    return getDevice(port)->write(byteArray);
}

quint32 PropellerManager::minimumTimeout(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return 0;
    return getDevice(port)->minimumTimeout();
}

void PropellerManager::setMinimumTimeout(PropellerSession * session, const QString & port, quint32 milliseconds)
{
    if (portIsBusy(session, port)) return;
    getDevice(port)->setMinimumTimeout(milliseconds);
}

quint32 PropellerManager::calculateTimeout(PropellerSession * session, const QString & port, quint32 bytes)
{
    if (portIsBusy(session, port)) return 0;
    return getDevice(port)->calculateTimeout(bytes);
}

void PropellerManager::useReset(PropellerSession * session, const QString & port, const QString & name, int pin)
{
    if (portIsBusy(session, port)) return;
    getDevice(port)->useReset(name, pin);

}

void PropellerManager::useDefaultReset(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return;
    return getDevice(port)->useDefaultReset();
}

bool PropellerManager::reset(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port)->reset();

}

int PropellerManager::error(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return 0;
    return getDevice(port)->error();
}

QString PropellerManager::errorString(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return QString();
    return getDevice(port)->errorString();
}
