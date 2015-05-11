#include "propellerimage.h"

PropellerImage::PropellerImage(QByteArray image, QString filename)
{
    EEPROM_SIZE = 32768;

    _image  = image;
    _filename = filename;
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
        _valid = false;
    }
    else
    {
        _valid = true;
    }
}

int PropellerImage::checksum()
{
    int checksum = 0;
    foreach (unsigned char c, _image)
    {
        checksum += c;
    }

    // Add value of initial call frame
    if (imageType() == Binary)
        checksum += 2 * (0xff + 0xff + 0xff + 0xf9);

    checksum &= 0xff;

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

int PropellerImage::programSize()   /** start of stack space pointer (DBASE) */
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

int PropellerImage::startOfCode()
{
    int start = readWord(6);

    if (start != 0x0010)
        qDebug() << "Code start is invalid!";

    return start;
}

int PropellerImage::startOfVariables()
{
    return readWord(8);
}

int PropellerImage::startOfStackSpace() // (DBASE)
{
    return readWord(10);
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
    if (imageSize() > programSize())
        return Eeprom;
    else
        return Binary;
}
