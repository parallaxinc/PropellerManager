#include "propellermanager.h"

#include <QDebug>


PropellerManager::PropellerManager(QObject * parent)
    : QObject(parent)
{
    _ports = PropellerDevice::list();
}

PropellerManager::~PropellerManager()
{

}

PropellerSession * PropellerManager::newSession(const QString & port)
{
    PropellerSession * s = new PropellerSession(port, this);
    return _sessions[s] = s;
}

PropellerDevice * PropellerManager::getDevice(const QString & port)
{
    if (!_devices.contains(port))
    {
        _devices[port] = new PropellerDevice();
        _devices[port]->setPortName(port);
    }

    if (!_ports.contains(port))
        message("Device does not exist!", port);

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

void PropellerManager::attach(PropellerSession * session, PropellerDevice * device)
{
    // signals
    connect(device,  &PropellerDevice::finished,        session,  &PropellerSession::finished);
    connect(device,  &PropellerDevice::sendError,       session,  &PropellerSession::sendError);
    connect(device,  &PropellerDevice::bytesWritten,    session,  &PropellerSession::bytesWritten);
    connect(device,  &PropellerDevice::readyRead,       session,  &PropellerSession::readyRead);
    connect(device,  &PropellerDevice::baudRateChanged, session,  &PropellerSession::baudRateChanged);

    // slots
    connect(session, &PropellerSession::timeover,           device,  &PropellerDevice::timeOver);
    connect(session, &PropellerSession::_write_buffer_empty,   device,  &PropellerDevice::writeBufferEmpty);

    _active_sessions[device]++;
    _connections[session] = device;

    if (!device->isOpen())
        device->open();
}

void PropellerManager::detach(PropellerSession * session, PropellerDevice * device)
{
    // signals
    disconnect(device,  &PropellerDevice::finished,        session,  &PropellerSession::finished);
    disconnect(device,  &PropellerDevice::sendError,       session,  &PropellerSession::sendError);
    disconnect(device,  &PropellerDevice::bytesWritten,    session,  &PropellerSession::bytesWritten);
    disconnect(device,  &PropellerDevice::readyRead,       session,  &PropellerSession::readyRead);
    disconnect(device,  &PropellerDevice::baudRateChanged, session,  &PropellerSession::baudRateChanged);

    // slots
    disconnect(session, &PropellerSession::timeover,           device,  &PropellerDevice::timeOver);
    disconnect(session, &PropellerSession::_write_buffer_empty,   device,  &PropellerDevice::writeBufferEmpty);

    _active_sessions[device]--;
    _connections.remove(session);

    if (!_active_sessions[device])
        device->close();
}

void PropellerManager::deleteSession(PropellerSession * session)
{
    if (!session) return;

    QString port = session->portName();
    if (!_devices.contains(port)) return;

    detach(_sessions[session], _devices[port]);
    delete _sessions[session];
    _sessions.remove(session);
}

void PropellerManager::deleteDevice(const QString & port)
{
    foreach(PropellerSession * s, _sessions)
    {
        if (port == s->portName())
            detach(_sessions[s], _devices[port]);
    }
    _devices.remove(port);
}

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
    attachByName(session, name);
    return false;
}

bool PropellerManager::clear(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return false;
    return getDevice(port)->clear();
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
    return getDevice(port)->bytesAvailable();
}

QByteArray PropellerManager::read(PropellerSession * session, const QString & port, qint64 maxSize)
{
    if (portIsBusy(session, port)) return QByteArray();
    return getDevice(port)->read(maxSize);
}

QByteArray PropellerManager::readAll(PropellerSession * session, const QString & port)
{
    if (portIsBusy(session, port)) return QByteArray();
    return getDevice(port)->readAll();
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




void PropellerManager::message(const QString & message, const QString & port)
{
    QString text = "PropellerManager "+port+": "+message;
    fprintf(stderr, "%s\n", qPrintable(text));
    fflush(stderr);
}
