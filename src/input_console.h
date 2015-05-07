#include <QObject>
#include <QSocketNotifier>
#include <QTextStream>

#include <unistd.h> //Provides STDIN_FILENO

/**
  \class ConsoleReader

  This class provides some stuff for doing things.


  */

namespace Input
{
    class Console : public QObject
    {
        Q_OBJECT
    public:
        explicit Console (QObject *parent = 0) : 
            QObject(parent),
            notifier(STDIN_FILENO, QSocketNotifier::Read)
        {
            connect(&notifier, SIGNAL(activated(int)), this, SLOT(text()));
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
