#include "sessionmanager.h"

#include <QByteArray>

#include "sessioninterface.h"
#include "deviceinterface.h"
#include "../propellersession/propellersession.h"

namespace PM
{

    SessionManager::SessionManager() 
    {
    }
    
    SessionManager::~SessionManager() 
    {
        foreach (PropellerSession * key, _interfaces.keys())
        {
            remove(key);
        }
    }
    
    SessionInterface * SessionManager::interface(PropellerSession * key)
    {
        if (exists(key)) return _interfaces[key];
    
        _interfaces[key] = new SessionInterface();
    
        return _interfaces[key];
    }
    
    void SessionManager::readyBuffer()
    {
        DeviceInterface * device = (DeviceInterface *) sender();
        QByteArray newdata = device->readAll();
    
        foreach (SessionInterface * interface, _interfaces.values())
        {
            if (interface->target() == device 
                    && !interface->isPaused()
                    && !_interfaces.key(interface)->isPaused())
                interface->append(newdata);
        }
    }

}
