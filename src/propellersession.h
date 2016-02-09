#pragma once

#include "template/connector.h"
#include "sessioninterface.h"
#include "propellermanager.h"

class PropellerManager;

class PropellerSession
    : public Connector<PM::SessionInterface *>
{
    Q_OBJECT

private:
    PropellerManager * manager;

public:
    PropellerSession(
            PropellerManager * manager,
            const QString & portname = QString());
    ~PropellerSession();

    void    setPortName(const QString & name);
    bool    reserve();
    bool    isReserved();
    void    release();
};

