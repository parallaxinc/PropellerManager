#pragma once

#include "template/manager.h"
#include "sessioninterface.h"

class PropellerSession;

namespace PM
{

    class SessionManager 
        : public QObject, 
          public Manager<PropellerSession *, SessionInterface *>
    {
        Q_OBJECT
    
    public:
        SessionManager();
        ~SessionManager();
        SessionInterface * interface(PropellerSession * key);
    
    public slots:
        void readyBuffer();
    };

}
