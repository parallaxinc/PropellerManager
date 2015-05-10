#include "propellerimage.h"

PropellerImage::PropellerImage(QByteArray image, Type type)
{
    int EEPROM_SIZE = 32768;

    _image  = image;
    _clkmodesettings = initClockModeSettings();

    if (_image.isEmpty())
    {
        Utility::print_status("EMPTY IMAGE");
        _valid = false;
    }
    else if (_image.size() % 4 != 0)
    {
        Utility::print_status("INVALID IMAGE SIZE");
        _valid = false;
    }
    else if (_image.size() > EEPROM_SIZE)
    {
        Utility::print_status("Code too long for EEPROM (max "+QString::number(EEPROM_SIZE)+" bytes)");
        _valid = false;
    }
    else if (checksum())
    {
        Utility::print_status("BAD CHECKSUM");
        _valid = true;
    //    _valid = false;
    }
    else
    {
        _type   = type;
        _valid = true;
    }
}

int PropellerImage::checksum()
{
    int checksum = 0;
    for (int i = 0; i < _image.size(); i++)
    {
        checksum += (unsigned char) _image.at(i);
    }

    qDebug() << "CHECKSUM:" << checksum;

    if (_type == Binary)
    {
        checksum += 2 * (0xff + 0xff + 0xf9 + 0xff);
    }

    checksum &= 0xff;
    qDebug() << "CHECKSUM:" << checksum;

    return checksum;
}

QByteArray PropellerImage::data()
{
    return _image;
}

void PropellerImage::setData(QByteArray data) {
    Q_UNUSED(data);
}

bool PropellerImage::isValid()
{
    return _valid;
}

int PropellerImage::imageSize()
{
    return _image.size();
}

int PropellerImage::stackSize()
{
    return 0;
}

int PropellerImage::programSize()
{
    int dbase =  (unsigned char) _image.at(0x0a) + 
                ((unsigned char) _image.at(0x0b) << 8);
    return dbase;
}

int PropellerImage::variableSize()
{
    return 0;
}

int PropellerImage::freeSpace()
{
    return 0;
}

void PropellerImage::setClockFrequency(int frequency)
{
    Q_UNUSED(frequency);
}

quint8  PropellerImage::readByte(int pos)
{
    return (quint8) _image.at(pos);
}

quint16 PropellerImage::readWord(int pos)
{
    return  (readByte(pos)) +
            (readByte(pos+1) << 8);
}

quint32 PropellerImage::readLong(int pos)
{
    return  (readByte(pos)) +
            (readByte(pos+1) << 8) +
            (readByte(pos+2) << 16) +
            (readByte(pos+3) << 24);
}

quint32 PropellerImage::clockFrequency()
{
    return readLong(0x0);
}

quint8 PropellerImage::clockMode()
{
    quint8 clkmode = readByte(4);
    if (_clkmodesettings.contains(clkmode))
        return clkmode;
    else
        return 0x80;
}

QString PropellerImage::clockModeText()
{
    return _clkmodesettings.value(clockMode());
}

QHash<quint8, QString> PropellerImage::initClockModeSettings()
{
    QHash<quint8, QString> clkmode;
    clkmode[0x00] = "rcfast";
    clkmode[0x01] = "rcslow";
    clkmode[0x22] = "xinput";
    clkmode[0x2A] = "xtal1";
    clkmode[0x32] = "xtal2";
    clkmode[0x3A] = "xtal3";

    clkmode[0x63] = "xinput+pll1x";
    clkmode[0x64] = "xinput+pll2x";
    clkmode[0x65] = "xinput+pll4x";
    clkmode[0x66] = "xinput+pll8x";
    clkmode[0x67] = "xinput+pll16x";

    clkmode[0x6B] = "xtal1+pll1x";
    clkmode[0x6C] = "xtal1+pll2x";
    clkmode[0x6D] = "xtal1+pll4x";
    clkmode[0x6E] = "xtal1+pll8x";
    clkmode[0x6F] = "xtal1+pll16x";

    clkmode[0x73] = "xtal2+pll1x";
    clkmode[0x74] = "xtal2+pll2x";
    clkmode[0x75] = "xtal2+pll4x";
    clkmode[0x76] = "xtal2+pll8x";
    clkmode[0x77] = "xtal2+pll16x";

    clkmode[0x7B] = "xtal3+pll1x";
    clkmode[0x7C] = "xtal3+pll2x";
    clkmode[0x7D] = "xtal3+pll4x";
    clkmode[0x7E] = "xtal3+pll8x";
    clkmode[0x7F] = "xtal3+pll16x";

    clkmode[0x80] = "INVALID";

    return clkmode;
}

//void PropellerImage::setType(PropellerImage::Type type)
//{
//    _type = type;
//}
