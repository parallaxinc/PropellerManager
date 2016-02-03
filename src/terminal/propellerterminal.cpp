#include "propellerterminal.h"

#include <QEventLoop>

PropellerTerminal::PropellerTerminal(PropellerManager * manager,
                                    const QString & portname,
                                    qint32 baudrate,
                                    QObject * parent)
    : QObject(parent)
{
    session = new PropellerSession(manager, portname);
    session->setBaudRate(baudrate);
}

PropellerTerminal::~PropellerTerminal()
{
    delete session;
}

void PropellerTerminal::exec()
{
    printf("Entering terminal on %s\n",qPrintable(session->portName()));
    printf("Press Ctrl+C to exit\n");
    printf("--------------------------------------\n");

    connect(session, SIGNAL(readyRead()), this, SLOT(read()));
    connect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write(const QString &)));

    QEventLoop loop;
    loop.exec();

    disconnect(&console, SIGNAL(textReceived(const QString &)),this, SLOT(write(const QString &)));
    disconnect(session, SIGNAL(readyRead()), this, SLOT(read()));
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

