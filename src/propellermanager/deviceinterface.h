#pragma once

#include <QObject>
#include <QHash>

#include "../common/connector.h"
#include "../propellerdevice/propellerdevice.h"

namespace PM
{

    class DeviceInterface : public Connector<PropellerDevice *>
    {
        Q_OBJECT
    
    public:
        DeviceInterface(QString port)
            : Connector<PropellerDevice *>()
        {
            _target = new PropellerDevice();
            _target->setPortName(port);
    
            attach(_target);
            open();
        }
    
        ~DeviceInterface()
        {
            close();
            detach();
    
            delete _target;
        }
    
        bool isActive()
        {
            if (!isAttached() || isPaused()) return false;
            if (!isOpen())
            {
                return open();
            }
            return true;
        }
    
        bool open()
        {
            return _target->open();
        }
    
        void close()
        {
            _target->close();
        }
    
        void saveBaudRate();
    };

}
