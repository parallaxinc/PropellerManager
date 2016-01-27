#include <QObject>

#include "readbuffer.h"
#include "../session/propellersession.h"

class SessionInterface : public Connector<SessionInterface *>
{
    Q_OBJECT

    ReadBuffer * _buffer;

public:
    SessionInterface ()
        : Connector<SessionInterface *>()
    {
        _buffer = new ReadBuffer();
    }

    ~SessionInterface()
    {
        delete _buffer;
    }

protected:
    void attachSignals()
    {
        connect(_target,    SIGNAL(sendError(const QString &)), this,   SIGNAL(sendError(const QString &)));
        connect(_target,    SIGNAL(bytesWritten()),             this,   SIGNAL(bytesWritten()));
        connect(_target,    SIGNAL(baudRateChanged()),          this,   SIGNAL(baudRateChanged()));

        connect(_buffer,    SIGNAL(readyRead()),                this,   SIGNAL(readyRead()));
    }

    void detachSignals()
    {
        disconnect(_target,    SIGNAL(sendError(const QString &)), this,   SIGNAL(sendError(const QString &)));
        disconnect(_target,    SIGNAL(bytesWritten()),             this,   SIGNAL(bytesWritten()));
        disconnect(_target,    SIGNAL(baudRateChanged()),          this,   SIGNAL(baudRateChanged()));

        disconnect(_buffer,    SIGNAL(readyRead()),                this,   SIGNAL(readyRead()));
    }
};

