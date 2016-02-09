#pragma once

#include <QString>

#include "template/manager.h"
#include "propellerdevice.h"

namespace PM
{

    class DeviceManager 
        : public Manager<QString, PropellerDevice *>
    {
    
    public:
        DeviceManager() 
        {
        }
    
        ~DeviceManager() 
        {
            foreach (QString key, _interfaces.keys())
            {
                remove(key);
            }
        }
    
        PropellerDevice * interface(QString key)
        {
            if (exists(key)) return _interfaces[key];
    
            _interfaces[key] = new PropellerDevice(key);
    
            return _interfaces[key];
        }

        void remove(QString key)
        {
            if (exists(key))
            {
                delete _interfaces[key];
                _interfaces.remove(key);
            }
        }

        bool enabled(QString key)
        {
            return interface(key)->isOpen();
        }

        void setEnabled(QString key, bool enabled)
        {
            if (enabled)
                interface(key)->open();
            else
                interface(key)->close();
        }
    };

}
