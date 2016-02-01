#pragma once

#include <QString>
#include <QByteArray>

#include "manager.h"
#include "../session/propellersession.h"
#include "sessioninterface.h"
#include "deviceinterface.h"

class PropellerSession;

class SessionManager 
    : public QObject, 
      public Manager<PropellerSession *, SessionInterface *>
{
    Q_OBJECT

public:
    SessionManager() 
    {
    }

    ~SessionManager() 
    {
        foreach (PropellerSession * key, _interfaces.keys())
        {
            remove(key);
        }
    }

    SessionInterface * interface(PropellerSession * key)
    {
        if (exists(key)) return _interfaces[key];
    
        _interfaces[key] = new SessionInterface();

        return _interfaces[key];
    }

public slots:
    void readyBuffer()
    {
        DeviceInterface * device = (DeviceInterface *) sender();
        QByteArray newdata = device->readAll();
    
        foreach (SessionInterface * interface, _interfaces.values())
        {
            if (interface->target() == device)
                interface->append(newdata);
        }
    }
};
