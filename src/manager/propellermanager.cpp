#include "propellermanager.h"

#include <QDebug>

#include "../util/logging.h"


PropellerManager::PropellerManager(QObject * parent)
    : QObject(parent)
{
    connect(&monitor,   SIGNAL(listChanged()),
            this,       SIGNAL(portListChanged()));
}

PropellerManager::~PropellerManager()
{
}

bool PropellerManager::beginSession(PropellerSession * session)
{
    if (!session) return false;

    _sessions[session] = session;

    addConnectionByName(session, session->portName());
    return true;
}

void PropellerManager::endSession(PropellerSession * session)
{
    if (!session) return;

    QString port = session->portName();
    if (!_devices.contains(port)) return;

    removeConnection(_sessions[session], _devices[port]);
    _sessions.remove(session);
}

PropellerDevice * PropellerManager::addDevice(QString port)
{
    if (_devices.contains(port)) return _devices[port];

    _devices[port] = new PropellerDevice();
    _devices[port]->setPortName(port);

    connect(_devices[port], &PropellerDevice::readyRead,
            this,           &PropellerManager::readyBuffer);

    return _devices[port];
}

PropellerDevice * PropellerManager::getDevice(QString port, bool open)
{
    PropellerDevice * device = addDevice(port);

    if (!monitor.list().contains(port))
    {
        qCDebug(pmanager) << "ERROR: Device does not exist:" << port;
        return device;
    }

    if (!device->isOpen() && open)
    {
//        qCDebug(pmanager) << "Opening" << device;
        device->open();
    }

    return device;
}

void PropellerManager::addConnectionByName(PropellerSession * session, QString port)
{
    PropellerDevice * olddevice = _connections[session];
    PropellerDevice * newdevice = getDevice(port, false);

    if (olddevice != NULL)
    {
        if (olddevice != newdevice) 
        {
            removeConnection(session, olddevice);
            addConnection(session, newdevice);
        }
    }
    else
    {
        addConnection(session, newdevice);
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

void PropellerManager::addConnection(PropellerSession * session, PropellerDevice * device)
{
    _buffers[session] = new ReadBuffer();

    _connections[session] = device;
    _active_sessions[device]++;

//    qCDebug(pmanager) << "connecting" << session << "to" << _devices.key(device);

    // signals

    // pass-through
    connect(device,  &PropellerDevice::sendError,       session,  &PropellerSession::sendError);
    connect(device,  &PropellerDevice::bytesWritten,    session,  &PropellerSession::bytesWritten);
    connect(device,  &PropellerDevice::baudRateChanged, session,  &PropellerSession::baudRateChanged);

    // buffered
    connect(_buffers[session],  &ReadBuffer::readyRead,   session,  &PropellerSession::readyRead);
}

void PropellerManager::removeConnection(PropellerSession * session, PropellerDevice * device)
{
//    qCDebug(pmanager) << "removing" << session << "from" << _devices.key(device);

    // signals

    // pass-through
    disconnect(device,  &PropellerDevice::sendError,       session,  &PropellerSession::sendError);
    disconnect(device,  &PropellerDevice::bytesWritten,    session,  &PropellerSession::bytesWritten);
    disconnect(device,  &PropellerDevice::baudRateChanged, session,  &PropellerSession::baudRateChanged);

    // buffered
    disconnect(_buffers[session],  &ReadBuffer::readyRead,   session,  &PropellerSession::readyRead);

    _active_sessions[device]--;
    _connections.remove(session);

    delete _buffers[session];
    _buffers.remove(session);

    if (!_active_sessions[device])
    {
//        qCDebug(pmanager) << "closing" << _devices.key(device);
        device->close();
    }
}

void PropellerManager::removeDevice(QString port)
{
    foreach(PropellerSession * s, _sessions)
    {
        if (port == s->portName())
            removeConnection(_sessions[s], _devices[port]);
    }

    disconnect(_devices[port], &PropellerDevice::readyRead,
                         this, &PropellerManager::readyBuffer);
    _devices.remove(port);
}

bool PropellerManager::portIsBusy(PropellerSession * session, const QString & name)
{
    if (isPaused(session)) return true;

    PropellerDevice * device = getDevice(name, false);
    if (!_connections.contains(session) || _connections[session] != device) return true;

//    qDebug() << "CONNECTION" << session << name << device << _connections[session];

    if (_busy.contains(device) && _busy[device] != session) return true;

    addConnectionByName(session, name);
    return false;
}

void PropellerManager::saveConnections(PropellerDevice * device)
{
    foreach (PropellerSession * session, _connections.keys(device))
    {
        _saved_connections[session] = device;
        removeConnection(session, device);

        emit session->deviceBusy();
    }
}

void PropellerManager::restoreConnections(PropellerDevice * device)
{
    foreach (PropellerSession * session, _saved_connections.keys(device))
    {
        addConnection(session, device);
        emit session->deviceFree();
        _saved_connections.remove(session);
    }
}

bool PropellerManager::reserve(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return false;
    PropellerDevice * device = getDevice(port);
    _active_sessions[device]++; // prevent port from closing while reserving device

    saveConnections(device);
    addConnection(session, device);    // addConnection new device

    _active_sessions[device]--; // prevent port from closing while reserving device
    _busy[device] = session;    // mark as busy

    return true;
}

bool PropellerManager::isReserved(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return false;

    PropellerDevice * device = getDevice(port);
    if (!_busy.contains(device) || _busy[device] != session)
        return false;

    return true;
}


void PropellerManager::release(PropellerSession * session, QString port)
{
    if (!isReserved(session, port)) return;

    PropellerDevice * device = getDevice(port);
    _active_sessions[device]++;                 // prevent port from closing while releasing device

    removeConnection(session, device);
    restoreConnections(device);
    
    _active_sessions[device]--;                 // prevent port from closing while releasing device
    _busy.remove(device);                       // unmark as busy
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

bool PropellerManager::clear(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port)->clear();
}

bool PropellerManager::isOpen(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port, false)->isOpen();
}

bool PropellerManager::setBaudRate(PropellerSession * session, QString port, quint32 baudRate)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port)->setBaudRate(baudRate);
}

qint64 PropellerManager::bytesToWrite(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return 0;
    return getDevice(port)->bytesToWrite();
}

qint64 PropellerManager::bytesAvailable(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return 0;
    return _buffers[session]->bytesAvailable();
}

QByteArray PropellerManager::read(PropellerSession * session, QString port, qint64 maxSize)
{
    if (portIsBusy(session, port)) return QByteArray();
    return _buffers[session]->read(maxSize);
}

QByteArray PropellerManager::readAll(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return QByteArray();
    return _buffers[session]->readAll();
}

bool PropellerManager::putChar(PropellerSession * session, QString port, char c)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port)->putChar(c);
}

qint64 PropellerManager::write(PropellerSession * session, QString port, const QByteArray & byteArray)
{
    if (portIsBusy(session, port)) return -1;
    return getDevice(port)->write(byteArray);
}

quint32 PropellerManager::minimumTimeout(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return 0;
    return getDevice(port)->minimumTimeout();
}

void PropellerManager::setMinimumTimeout(PropellerSession * session, QString port, quint32 milliseconds)
{
    if (portIsBusy(session, port)) return;
    getDevice(port)->setMinimumTimeout(milliseconds);
}

quint32 PropellerManager::calculateTimeout(PropellerSession * session, QString port, quint32 bytes)
{
    if (portIsBusy(session, port)) return 0;
    return getDevice(port)->calculateTimeout(bytes);
}

void PropellerManager::useReset(PropellerSession * session, QString port, QString name, int pin)
{
    if (portIsBusy(session, port)) return;
    getDevice(port)->useReset(name, pin);
}

void PropellerManager::useDefaultReset(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return;
    return getDevice(port)->useDefaultReset();
}

bool PropellerManager::reset(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port)->reset();
}

quint32 PropellerManager::resetPeriod(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return 0;
    return getDevice(port)->resetPeriod();
}

int PropellerManager::error(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return 0;
    return getDevice(port)->error();
}

QString PropellerManager::errorString(PropellerSession * session, QString port)
{
    if (portIsBusy(session, port)) return QString();
    return getDevice(port)->errorString();
}

QStringList PropellerManager::listPorts()
{
    return monitor.list();
}

void PropellerManager::enablePortMonitor(bool enabled, int timeout)
{
    monitor.toggle(enabled, timeout);
}

