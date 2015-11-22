#pragma once

#include "../manager/propellermanager.h"

#include <QTimer>
#include <QStringList>

/**
@class PropellerSession session/propellersession.h PropellerSession
  
@brief The PropellerSession class provides a persistent interface to Propeller hardware via PropellerManager.

Sessions can communicate with any available device without needing to 
manually open and close ports. Many sessions can be connected to a single
device at a time. However, sessions may also request exclusive access to
a device to complete a long-running operation.

### Usage

Using PropellerSession is easy because it handles all of the device setup transparently. In just a few
lines of code, you'll be able to develop Propeller-based applications.

Here is a random function that writes some data to the device.

@code
void writeToDevice()
{
    PropellerManager manager;
    PropellerSession session(&manager);
    session.setPortName("ttyUSB0");
    session.write(QByteArray("some data!"))
}
@endcode

PropellerSession automatically disconnects from PropellerManager when destroyed, so manual cleanup
isn't needed, and it's easy to make sure your connected devices don't end up in an invalid state.

\see PropellerManager
    */

class PropellerManager;

class PropellerSession : public QObject
{
    Q_OBJECT

private:
    QString port;
    PropellerManager * manager;

public:
    PropellerSession(
            PropellerManager * manager,
            const QString & portname = QString(), 
            QObject * parent = 0);
    ~PropellerSession();

    const QString & portName();
    void        setPortName(const QString & name);
    bool        isOpen();

/**
    @name Access Control
  */
/**@{*/
    bool        reserve();
    bool        isReserved();
    void        release();

    void        pause();
    bool        isPaused();
    void        unpause();
/**@}*/

/**
    @name Device I/O
  */

/**@{*/
    bool        clear();
    bool        setBaudRate(quint32 baudRate);

    qint64      bytesToWrite();
    qint64      bytesAvailable();

    QByteArray  read(qint64 maxSize);
    QByteArray  readAll();

    bool        putChar(char c);
    qint64      write(const QByteArray & byteArray);
    int         error();
    QString     errorString();
/**@}*/

/**
    @name Download-Related
  */

/**@{*/
    void        useReset(const QString & name, int pin);
    void        useDefaultReset();
    bool        reset();

    quint32     minimumTimeout();
    void        setMinimumTimeout(quint32 milliseconds);
    quint32     calculateTimeout(quint32 bytes);
/**@}*/

signals:
    void        bytesWritten(qint64 bytes);
    void        readyRead();
    void        baudRateChanged(qint32 baudRate);
    void        sendError(const QString & message);
    void        deviceFree();
    void        deviceBusy();

};

