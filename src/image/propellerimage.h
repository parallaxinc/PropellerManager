#pragma once

#include <QByteArray>
#include <QString>
#include <QHash>
#include <QStringList>

/**
@class PropellerImage image/propellerimage.h PropellerImage

@brief The PropellerImage class encapsulates Propeller binary images.

- Converts between binary and EEPROM image formats
- Extract information about binaries
- Ability to customize and retarget binaries
- Can validate images for accuracy

### Propeller Application Format

The Propeller Application image consists of data blocks for initialization,
program, variables, and data/stack space. The first block, initialization,
describes the application's startup parameters, including the position of the 
other blocks within the image, as shown below.

| data size | address  | description      |
|-----------|----------|------------------|
| long      | 0        | Clock Frequency |
| byte      | 4        | Clock Mode |
| byte      | 5        | Checksum (this value causes additive checksum of bytes 0 to ImageLimit-1 to equal 0) |
| word      | 6        | Start of Code pointer (must always be $0010) |
| word      | 8        | Start of Variables pointer |
| word      | 10       | Start of Stack Space pointer |
| word      | 12       | Current Program pointer (points to first public method of object) |
| word      | 14       | Current Stack Space pointer (points to first run-time usable space of stack) |

### What Gets Downloaded

To save time, PropellerManager does not download the entire application image. Instead, it
downloads only the parts of the image from long 0 through the end of code (up
to the start of variables) and then the Propeller chip itself writes zeros (0)
to the rest of the RAM/EEPROM, after the end of code (up to 32 Kbytes), and
inserts the initial call frame in the proper location. This effectively clears
(initializes) all global variables to zero (0) and sets all available stack
and free space to zero (0) as well.
*/

/**
@example imageinfo/main.cpp

Here we extract various pieces of information from a Propeller binary image.
*/


class PropellerImage
{
    enum ImageFormat
    {
        _long_clockfrequency = 0,
        _byte_clockmode = 4,
        _byte_checksum = 5,
        _word_code = 6,
        _word_variables = 8,
        _word_stackspace = 10
    };

    struct ClockMode
    {
        quint8 value;
        QString name;
    };

public:

    /**
      An enumeration containing all Propeller image types that PropellerManager supports.
      */
    enum ImageType
    {
        Invalid,        ///< Not a valid image file
        Binary,         ///< Program data-only image files (usually have a `.binary` extension)
        Eeprom          ///< Complete EEPROM images        (usually have a `.eeprom` extension)
    };

private:

    QByteArray  _image;
    QString     _filename;
    ImageType   _type;
    QHash<ImageType, QString> _typenames;

    QList<ClockMode> _clkmodesettings;
    QStringList _clockmodes;
    QList<ClockMode> initClockModeSettings();
    ClockMode createClockMode(quint8 value, QString name);

public:

    PropellerImage( const QByteArray & image = QByteArray(),
                    const QString & filename = "");

    /**
        @name Image Validation
      */

    /**@{*/
    quint8      checksum();
    bool        checksumIsValid();
    bool        isValid();
    bool        recalculateChecksum();
    /**@}*/

    /**
        @name Image Information
      */

    /**@{*/
    QString     fileName();
    ImageType   imageType();
    QString     imageTypeText();

    quint32     imageSize();
    quint32     eepromSize();
    quint32     programSize();

    quint32     codeSize();
    quint32     variableSize();
    quint32     stackSize();

    quint16     startOfCode();
    quint16     startOfVariables();
    quint16     startOfStackSpace();
    /**@}*/

    /**
        @name Low-Level Functions
      */

    /**@{*/
    QByteArray  data();
    void        setData(QByteArray image);

    quint8  readByte( int pos);
    quint16 readWord( int pos);
    quint32 readLong( int pos);

    void    writeByte(int pos, quint8 value);
    void    writeWord(int pos, quint16 value);
    void    writeLong(int pos, quint32 value);

    /**@}*/

    /**
        @name Clock Settings
      */

    /**@{*/

    quint32     clockFrequency();
    void        setClockFrequency(quint32 frequency);

    quint8      clockMode();
    QString     clockModeText();
    QString     clockModeText(quint8 value);
    quint8      clockModeValue(QString name);
    bool        setClockMode(quint8 value);
    QStringList listClockModes();
    ClockMode   clockModeExists(quint8 value);
    ClockMode   clockModeExists(QString name);

    /**@}*/

};
