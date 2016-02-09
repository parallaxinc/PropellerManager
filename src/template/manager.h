#pragma once

#include "interface.h"

#include <QHash>

template <class Key, class Interface>
class Manager
{

protected:
    QHash<Key, Interface> _interfaces;

public:
    Manager()
    {
    }
    
    virtual ~Manager()
    {
    }

    bool exists(Key key)
    {
        return (_interfaces.contains(key) && _interfaces[key] != NULL);
    }

    bool enabled(Key key)
    {
        return interface(key)->isPaused();
    }

    QList<Interface> list()
    {
        return _interfaces.values();
    }

    void setEnabled(Key key, bool enabled)
    {
        interface(key)->setPaused(!enabled);
    }

    virtual Interface interface(Key key) = 0;

    void remove(Key key)
    {
        if (exists(key))
        {
            interface(key)->detach();
            delete _interfaces[key];
            _interfaces.remove(key);
        }
    }

};
