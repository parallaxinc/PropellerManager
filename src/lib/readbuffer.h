#include <QByteArray>
#include <QObject>

class ReadBuffer : public QObject
{
    Q_OBJECT

    QByteArray array;

public:
    ReadBuffer(QObject * parent = 0)
        : QObject(parent)
    {
    }

    ~ReadBuffer()
    {
    }


    QByteArray & append(const QByteArray & ba)
    {
        array.append(ba);
        emit readyRead();
        return array;
    }


    QByteArray read(qint64 maxSize)
    {
        QByteArray data = array.left(maxSize);
        array.remove(0,maxSize);
        return data;
    }

    QByteArray readAll()
    {
        QByteArray data = array;
        array.clear();
        return data;
    }


    quint64 bytesAvailable()
    {
        return array.size();
    }

signals:
    void readyRead();

};

