#include "propellerimage.h"

#include "../util/logging.h"

PropellerImage::PropellerImage(QByteArray image, QString filename)
{
    EEPROM_SIZE = 4096 * 8;

    _image  = image;
    _filename = filename;
    _clkmodesettings = initClockModeSettings();
    _type = imageType();

    _typenames[Invalid] = "Invalid";
    _typenames[Binary] = "Binary";
    _typenames[Eeprom] = "EEPROM";
}

/**
    Returns the checksum of the image.

    A valid checksum has a value of zero.
    */

quint8 PropellerImage::checksum()
{
    quint8 sum = 0;
    foreach (quint8 c, _image)
        sum += c;

    if (_type == Binary)
        sum += 2 * (0xff + 0xff + 0xff + 0xf9);

    return sum;
}

/**
    Returns whether the checksum of the image is valid.

    \returns true if valid, otherwise false.
    */

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

/**
    Returns the raw binary data of the image.
    */

QByteArray PropellerImage::data()
{
    return _image;
}

/**
    Sets the raw binary data of the image.
    */

void PropellerImage::setData(QByteArray data) {
    _image = data;
}

/**
    Returns whether the image appears to be valid.

    \returns true if valid, otherwise returns false.
    */

bool PropellerImage::isValid()
{
    return (_type != Invalid);
}

/**
Total size of stored image file on disk.
    */

quint32 PropellerImage::imageSize()
{
    return _image.size();
}

/**
Total size of application in memory. This value will be larger than the total file size for PropellerImage::Binary images.

This value is equivalent to startOfStackSpace().
    */

quint32 PropellerImage::programSize()
{
    return startOfStackSpace();
}

/**
Size of the application code.

This value is equivalent to startOfVariables() + 8.
    */

quint32 PropellerImage::codeSize()
{
    return startOfVariables() - startOfCode();
}

/**
Returns the size in bytes of image data used for variables.
    */

quint32 PropellerImage::variableSize()
{
    return startOfStackSpace() - startOfVariables();
}

/**
Size in bytes of portion of image data used for stack space, or otherwise free.
    */

quint32 PropellerImage::stackSize()
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
        qCDebug(pimage) << "Code start is invalid!";

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
    return readWord(_word_stackspace) - 8; // subtract the initial stack frame
}

/**
Read a byte from address pos of the image.

@return an 8-bit unsigned value.
*/

quint8  PropellerImage::readByte(int pos)
{
    return (quint8) _image.at(pos);
}

/**
Read a word from address pos of the image.

\note This function does not perform word-align on the address passed.

@return a 16-bit unsigned value.
*/

quint16 PropellerImage::readWord(int pos)
{
    return  (readByte(pos)) +
        (readByte(pos+1) << 8);
}

/**
Read a long from address pos of the image.

\note This function does not perform long-align on the address passed.

@return a 32-bit unsigned value.
*/

quint32 PropellerImage::readLong(int pos)
{
    return  (readByte(pos)) +
        (readByte(pos+1) << 8) +
        (readByte(pos+2) << 16) +
        (readByte(pos+3) << 24);
}

/**
Write a byte-sized value to the image at pos.
    */

void PropellerImage::writeByte(int pos, quint8 value)
{
    _image[pos] = value;
}

/**
Write a word-sized value to the image at pos.

\note This function does not perform word-align on the address passed.
    */

void PropellerImage::writeWord(int pos, quint16 value)
{
    for (int i = 0; i < 2; i++)
        writeByte(pos+i, (value >> (i * 8)) & 0xFF);
}

/**
Write a long-sized value to the image at pos.

\note This function does not perform long-align on the address passed.
    */

void PropellerImage::writeLong(int pos, quint32 value)
{
    for (int i = 0; i < 4; i++)
        writeByte(pos+i, (value >> (i * 8)) & 0xFF);
}


/**
Sets a new clock frequency for the image.
*/

void PropellerImage::setClockFrequency(quint32 frequency)
{
    writeLong(_long_clockfrequency, frequency);
}

/**
Sets a new clock mode for the image.
*/

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
Returns the current clock frequency of the image.

@return a 32-bit unsigned value containing the clock frequency in Hertz.
    */

quint32 PropellerImage::clockFrequency()
{
    return readLong(_long_clockfrequency);
}

/**
Get an 8-bit integer containing the current clock mode.
    */

quint8 PropellerImage::clockMode()
{
    quint8 clkmode = readByte(_byte_clockmode);
    if (_clkmodesettings.contains(clkmode))
        return clkmode;
    else
        return 0x80;
}

/**
Get a human-readable string of the current clock mode.
    */

QString PropellerImage::clockModeText()
{
    return clockModeText(clockMode());
}

/**
Get a human-readable string of any known clock mode.
    */

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

/**
Returns the filename associated with this image.
    */

QString PropellerImage::fileName()
{
    return _filename;
}

/**
Returns the ImageType of the image, or ImageType::Invalid.
    */

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

/**
Returns a human-readable string of the ImageType.
    */

QString PropellerImage::imageTypeText()
{
    return _typenames[_type];
}
