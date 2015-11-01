#include "propellerterminal.h"

#include <QEventLoop>

PropellerTerminal::PropellerTerminal(PropellerSession * session,
                                    QObject * parent)
    : QObject(parent)
{
    this->session = session;

    connect(session, SIGNAL(readyRead()), this, SLOT(read()));
    connect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write(const QString &)));
}

PropellerTerminal::~PropellerTerminal()
{
    disconnect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write(const QString &)));
    disconnect(session, SIGNAL(readyRead()), this, SLOT(read()));

    this->session = NULL;
}

void PropellerTerminal::write(const QString & text)
{
    session->write(qPrintable(text));
    session->write("\r");
}

void PropellerTerminal::read()
{
    foreach (char c, session->readAll())
    {
        switch (c)
        {
            case 10:
            case 13:
                fprintf(stdout,"\n");
                break;
            default:
                fprintf(stdout,"%c",c);
        }
    }
    fflush(stdout);
}

