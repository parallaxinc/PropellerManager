#include "readbuffer.h"

ReadBuffer::ReadBuffer(QObject * parent)
    : QObject(parent)
{
}

ReadBuffer::~ReadBuffer()
{
}

QByteArray & ReadBuffer::append(QByteArray ba)
{
    array.append(ba);
    emit readyRead();
    return array;
}


QByteArray ReadBuffer::read(qint64 maxSize)
{
    QByteArray data = array.left(maxSize);
    array.remove(0,maxSize);
    return data;
}

QByteArray ReadBuffer::readAll()
{
    QByteArray data = array;
    array.clear();
    return data;
}

void ReadBuffer::clear()
{
    array.clear();
}

quint64 ReadBuffer::bytesAvailable()
{
    return array.size();
}
