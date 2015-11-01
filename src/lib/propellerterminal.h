#pragma once

#include "input_console.h"
#include "propellersession.h"

/**
@class PropellerTerminal 

@brief a small reference implementation of a serial terminal with PropellerManager.

Initializing 

*/
class PropellerTerminal : public QObject
{
    Q_OBJECT

private:
    PropellerSession * session;
    Input::Console console;

signals:
    void finished();

private slots:
    void read();
    void write(const QString & text);

public:
    PropellerTerminal(PropellerSession * session, QObject * parent = 0);
    ~PropellerTerminal();
};

