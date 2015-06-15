#include "propellerimage.h"

PropellerImage::PropellerImage(QByteArray image, QString filename)
{

    _image  = image;
    _filename = filename;
    _clkmodesettings = initClockModeSettings();
    _type = imageType();

    _typenames[Invalid] = "Invalid";
    _typenames[Binary] = "Binary";
    _typenames[Eeprom] = "EEPROM";
}

quint8 PropellerImage::checksum()
{
    quint8 sum = 0;
    foreach (quint8 c, _image)
        sum += c;

    if (_type == Binary)
        sum += 2 * (0xff + 0xff + 0xff + 0xf9);

    return sum;
}

bool PropellerImage::checksumIsValid()
{
    if (checksum())
        return false;
    else
        return true;
}

bool PropellerImage::recalculateChecksum()
{
    _image[_byte_checksum] = 0;
    _image[_byte_checksum] = 0x100 - checksum(); 

    imageType();
    return checksumIsValid();
}

QByteArray PropellerImage::data()
{
    return _image;
}

void PropellerImage::setData(QByteArray data) {
    _image = data;
}

bool PropellerImage::isValid()
{
    return (_type != Invalid);
}

/**
Total size of stored image file on disk.
*/

int PropellerImage::imageSize()
{
    return _image.size();
}

/**
Size of the application code. This value will be larger than the total file size for PropellerImage::Binary images.

This value is equivalent to startOfStackSpace().
*/

int PropellerImage::programSize()
{
    return startOfStackSpace();
}

int PropellerImage::variableSize()
{
    return startOfStackSpace() - 8 - startOfVariables();
}

int PropellerImage::stackSize()
{
    return EEPROM_SIZE - startOfStackSpace();
}

/**
Start of Code pointer (address 0x06). This value must always be equal to $0010.
*/

quint16 PropellerImage::startOfCode()
{
    int start = readWord(_word_code);

    if (start != 0x0010)
        qDebug() << "Code start is invalid!";

    return start;
}

/**
Start of Variables pointer (address 0x08).
*/

quint16 PropellerImage::startOfVariables()
{
    return readWord(_word_variables);
}

/**
Start of Stack Space pointer (address 0x0A). Otherwise known as DBase.
*/

quint16 PropellerImage::startOfStackSpace()
{
    return readWord(_word_stackspace);
}

/**
Read a byte from address pos of the image.

@param pos The address in bytes to read from.

@return an 8-bit unsigned value.
*/

quint8  PropellerImage::readByte(int pos)
{
    return (quint8) _image.at(pos);
}

/**
Read a word from address pos of the image.

@param pos The address in bytes to read from.

@return a 16-bit unsigned value.

*This function does not perform word-align on the address passed.*
*/

quint16 PropellerImage::readWord(int pos)
{
    return  (readByte(pos)) +
        (readByte(pos+1) << 8);
}

/**
Read a long from address pos of the image.

@param pos The address in bytes to read from.

@return a 32-bit unsigned value.

*This function does not perform long-align on the address passed.*
*/

quint32 PropellerImage::readLong(int pos)
{
    return  (readByte(pos)) +
        (readByte(pos+1) << 8) +
        (readByte(pos+2) << 16) +
        (readByte(pos+3) << 24);
}


void PropellerImage::writeByte(int pos, quint8 value)
{
    _image[pos] = value;
}

void PropellerImage::writeWord(int pos, quint16 value)
{
    for (int i = 0; i < 2; i++)
        writeByte(pos+i, (value >> (i * 8)) & 0xFF);
}

void PropellerImage::writeLong(int pos, quint32 value)
{
    for (int i = 0; i < 4; i++)
        writeByte(pos+i, (value >> (i * 8)) & 0xFF);
}


/**
**NOT YET IMPLEMENTED**

Replace the current clock frequency with another value and recalculate the checksum.
*/

void PropellerImage::setClockFrequency(quint32 frequency)
{
    writeLong(_long_clockfrequency, frequency);
}

bool PropellerImage::setClockMode(quint8 value)
{
    if (_clkmodesettings.contains(value))
    {
        writeByte(_byte_clockmode, value);
        return true;
    }
    else
        return false;
}


/**
Get the clock frequency.

@return a 32-bit unsigned value containing the clock frequency in Hertz.
*/

quint32 PropellerImage::clockFrequency()
{
    return readLong(_long_clockfrequency);
}

quint8 PropellerImage::clockMode()
{
    quint8 clkmode = readByte(_byte_clockmode);
    if (_clkmodesettings.contains(clkmode))
        return clkmode;
    else
        return 0x80;
}

QString PropellerImage::clockModeText()
{
    return clockModeText(clockMode());
}

QString PropellerImage::clockModeText(quint8 value)
{
    return _clkmodesettings.value(value);
}

QHash<quint8, QString> PropellerImage::initClockModeSettings()
{
    QHash<quint8, QString> clkmode;

    clkmode[0x00] = "RCFAST";
    clkmode[0x01] = "RCSLOW";
    clkmode[0x22] = "XINPUT";
    clkmode[0x2A] = "XTAL1";
    clkmode[0x32] = "XTAL2";
    clkmode[0x3A] = "XTAL3";

    clkmode[0x63] = "XINPUT+PLL1X";
    clkmode[0x64] = "XINPUT+PLL2X";
    clkmode[0x65] = "XINPUT+PLL4X";
    clkmode[0x66] = "XINPUT+PLL8X";
    clkmode[0x67] = "XINPUT+PLL16X";

    clkmode[0x6B] = "XTAL1+PLL1X";
    clkmode[0x6C] = "XTAL1+PLL2X";
    clkmode[0x6D] = "XTAL1+PLL4X";
    clkmode[0x6E] = "XTAL1+PLL8X";
    clkmode[0x6F] = "XTAL1+PLL16X";

    clkmode[0x73] = "XTAL2+PLL1X";
    clkmode[0x74] = "XTAL2+PLL2X";
    clkmode[0x75] = "XTAL2+PLL4X";
    clkmode[0x76] = "XTAL2+PLL8X";
    clkmode[0x77] = "XTAL2+PLL16X";

    clkmode[0x7B] = "XTAL3+PLL1X";
    clkmode[0x7C] = "XTAL3+PLL2X";
    clkmode[0x7D] = "XTAL3+PLL4X";
    clkmode[0x7E] = "XTAL3+PLL8X";
    clkmode[0x7F] = "XTAL3+PLL16X";

    clkmode[0x80] = "INVALID";

    return clkmode;
}

QString PropellerImage::fileName()
{
    return _filename;
}

PropellerImage::ImageType PropellerImage::imageType()
{
    if (_image.isEmpty())
    {
        _type = Invalid;
    }
    else if (_image.size() % 4 != 0)
    {
        _type = Invalid;
    }
    else if (_image.size() > _size_eeprom)
    {
        _type = Invalid;
    }
    else
    {
        if (imageSize() > programSize())
            _type = Eeprom;
        else
            _type = Binary;
    }

    if (!checksumIsValid())
    {
        _type = Invalid;
    }
    return _type;
}

QString PropellerImage::imageTypeText()
{
    return _typenames[_type];
}
