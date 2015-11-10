#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <QTextStream>

#include <unistd.h> //Provides STDIN_FILENO

namespace Input
{
    /**
      @class Console
      
      The Console class receives input from stdin, useful for using `propman`
      from the command-line.

      */

    class Console : public QObject
    {
        Q_OBJECT
    public:
        explicit Console (QObject *parent = 0) : 
            QObject(parent),
            notifier(STDIN_FILENO, QSocketNotifier::Read)
        {
            connect(&notifier, &QSocketNotifier::activated, this, &Console::text);
        }

    signals:
        void textReceived(QString message);

    public slots:
        void text()
        {
            QTextStream qin(stdin);
            QString line = qin.readLine();
            emit textReceived(line);
        }
        private:
        QSocketNotifier notifier;
    };

}
