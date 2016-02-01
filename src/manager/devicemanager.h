#pragma once

#include <QString>

#include "manager.h"
#include "deviceinterface.h"

class DeviceManager 
    : public Manager<QString, DeviceInterface *>
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

    DeviceInterface * interface(QString key)
    {
        if (exists(key)) return _interfaces[key];

        _interfaces[key] = new DeviceInterface(key);

        return _interfaces[key];
    }
};
