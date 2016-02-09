#include "protocol.h"

#include <QDataStream>
#include <QDebug>

PropellerProtocol::PropellerProtocol()
{
    _request = QByteArray((char*) Propeller::request, Propeller::request_size);
    _reply = QByteArray((char*) Propeller::reply, Propeller::reply_size);
}

QByteArray PropellerProtocol::encodeData(QByteArray image)
{
    int bits_processed = 0;
    int total_bits = image.size()*8;

    QByteArray encoded_image;

    while (bits_processed < total_bits)
    {

        int bits_in = qMin(5, image.size()*8 - bits_processed);
        quint8 mask = (1 << bits_in) - 1;
        quint8 bValue =(
                        (((quint8) (image[bits_processed/8] & 0xFF))     >> (bits_processed % 8)) + 
                        (((quint8) (image[bits_processed/8 + 1] & 0xFF)) << (8 - bits_processed % 8))
                    ) & mask;
        encoded_image.append(Propeller::translator[bValue][bits_in - 1][0]);
        bits_processed += Propeller::translator[bValue][bits_in - 1][1];

//        qDebug()
//            << bits_in
//            << mask
//            << QString::number(Propeller::translator[bValue][bits_in -1][0], 16)
//            << QString::number(bValue, 2);
    }

 //   qDebug() << "Totals:" << bits_processed << encoded_image.size() << bits_processed / 8 << bits_processed / 4 /8;

    return encoded_image;

}

QByteArray PropellerProtocol::packLong(quint32 value)
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << value;
    return result;
}

QByteArray PropellerProtocol::encodeLong(quint32 value)
{
    return encodeData(packLong(value));
}


int PropellerProtocol::lfsr(int * seed)
{
    char ret = *seed & 0x01;
    *seed = ((*seed << 1) & 0xfe) | (((*seed >> 7) ^ (*seed >> 5) ^ (*seed >> 4) ^ (*seed >> 1)) & 1);
    return ret;
}

QList<char> PropellerProtocol::buildLfsrSequence(int size)
{
    int seed = 'P';
    
    QList<char> seq;
    for (int i = 0; i < size; i++)
    {
        seq.append(lfsr(&seed));
    }

    return seq;
}


QByteArray PropellerProtocol::buildRequest(Command::Command command)
{
    QByteArray array = _request;
    array.append(QByteArray(125, 0x29));
    array.append(QByteArray(4, 0x29));
    array.append(encodeLong(command));
    return array;
}


QByteArray PropellerProtocol::reply()
{
    return _reply;
}

QByteArray PropellerProtocol::request()
{
    return _request;
}
