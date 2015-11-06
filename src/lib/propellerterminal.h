#pragma once

#include "input_console.h"
#include "propellersession.h"

/**
@class PropellerTerminal 

@brief The PropellerTerminal class provides a minimal implementation of a PropellerManager-based terminal.

Usage is easy; create an instance of PropellerTerminal and it will set up an event loop. Exit with Ctrl+C.

\see PropellerLoader
*/

/**
@example terminal/main.cpp

This example sets up the PropellerTerminal for use.
*/

class PropellerTerminal : public QObject
{
    Q_OBJECT

private:
    PropellerSession * session;
    Input::Console console;

private slots:
    void read();
    void write(const QString & text);

public:
    PropellerTerminal(PropellerManager * manager,
                      const QString & portname,
                      QObject * parent = 0);
    ~PropellerTerminal();
};

