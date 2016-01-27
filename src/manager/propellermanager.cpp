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
    foreach (QString port, _devices.keys())
    {
        delete _devices[port];
        _devices.remove(port);
    }
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

PropellerDevice * PropellerManager::createDevice(QString port)
{
    if (_devices.contains(port) && _devices[port] != NULL) return _devices[port];

    _devices[port] = new PropellerDevice();
    _devices[port]->setPortName(port);

    connect(_devices[port], &PropellerDevice::readyRead,
            this,           &PropellerManager::readyBuffer);

    return _devices[port];
}

PropellerDevice * PropellerManager::getDevice(QString port, bool open)
{
    PropellerDevice * device = createDevice(port);

    qCDebug(pmanager) << port << _devices.count() << _devices[port];

    if (!monitor.list().contains(port))
    {
        qCDebug(pmanager) << "ERROR: Device does not exist:" << port;
        return device;
    }

    if (!device->isOpen() && open)
    {
        qCDebug(pmanager) << "Opening" << device;
        device->open();
    }

    return device;
}

void PropellerManager::addConnectionByName(PropellerSession * session, QString port)
{
    PropellerDevice * olddevice = _connections[session];
    PropellerDevice * newdevice = getDevice(port);

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
    qCDebug(pmanager) << "connecting" << session << "to" << _devices.key(device);

    _buffers[session] = new ReadBuffer();

    if (_busy.contains(device))
        _saved_connections[session] = device;
    else
        _connections[session] = device;

    _active_sessions[device]++;


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
    qCDebug(pmanager) << "removing" << session << "from" << _devices.key(device) << _buffers[session];

    // signals

    // pass-through
    disconnect(device,  &PropellerDevice::sendError,       session,  &PropellerSession::sendError);
    disconnect(device,  &PropellerDevice::bytesWritten,    session,  &PropellerSession::bytesWritten);
    disconnect(device,  &PropellerDevice::baudRateChanged, session,  &PropellerSession::baudRateChanged);

    // buffered
    disconnect(_buffers[session],  &ReadBuffer::readyRead,   session,  &PropellerSession::readyRead);

    _active_sessions[device]--;

    if (_busy.contains(device))
        _saved_connections.remove(session);
    else
        _connections.remove(session);

    delete _buffers[session];
    _buffers.remove(session);

    if (!_active_sessions[device])
    {
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

bool PropellerManager::portIsBusy(PropellerSession * session)
{
    QString port = session->portName();

//    if (isPaused(session)) return true;

    PropellerDevice * device = getDevice(port, false);

    if (!_connections.contains(session)) return true;
    if (_busy.contains(device) && _busy[device] != session) return true;

    addConnectionByName(session, port);
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

bool PropellerManager::reserve(PropellerSession * session)
{
    if (portIsBusy(session)) return false;
    PropellerDevice * device = getDevice(session->portName());
    _active_sessions[device]++; // prevent port from closing while reserving device

    saveConnections(device);
    addConnection(session, device);    // addConnection new device

    _active_sessions[device]--; // prevent port from closing while reserving device
    _busy[device] = session;    // mark as busy

    return true;
}

bool PropellerManager::isReserved(PropellerSession * session)
{
    if (portIsBusy(session)) return false;

    PropellerDevice * device = getDevice(session->portName());
    if (!_busy.contains(device) || _busy[device] != session)
        return false;

    return true;
}


void PropellerManager::release(PropellerSession * session)
{
    if (!isReserved(session)) return;

    PropellerDevice * device = getDevice(session->portName());
    _active_sessions[device]++;                 // prevent port from closing while releasing device

    removeConnection(session, device);
    restoreConnections(device);
    
    _active_sessions[device]--;                 // prevent port from closing while releasing device
    _busy.remove(device);                       // unmark as busy
}

QStringList PropellerManager::listPorts()
{
    return monitor.list();
}

void PropellerManager::enablePortMonitor(bool enabled, int timeout)
{
    monitor.toggle(enabled, timeout);
}

