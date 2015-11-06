#include "propellerterminal.h"

#include <QEventLoop>

PropellerTerminal::PropellerTerminal(PropellerManager * manager,
                                    const QString & portname,
                                    QObject * parent)
    : QObject(parent)
{
    this->session = new PropellerSession(manager, portname);

    printf("Entering terminal on %s\n",qPrintable(portname));
    printf("Press Ctrl+C to exit\n");
    printf("--------------------------------------\n");

    connect(session, SIGNAL(readyRead()), this, SLOT(read()));
    connect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write(const QString &)));

    QEventLoop loop;
    loop.exec();
}

PropellerTerminal::~PropellerTerminal()
{
    disconnect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write(const QString &)));
    disconnect(session, SIGNAL(readyRead()), this, SLOT(read()));

    delete session;
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

