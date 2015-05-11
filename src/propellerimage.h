#pragma once

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QHash>

#include "utility.h"

/**
@class PropellerImage

@brief PropellerImage is a class for analyzing and modifying prebuilt propeller images.

- Converts between binary and EEPROM image formats
- Extract information about binaries
- Ability to customize and retarget binaries
- Can validate images for accuracy

### Propeller Application Format

The Propeller Application image consists of data blocks for initialization, program, variables, and data/stack space. The first block, initialization, describes the application's startup parameters, including the position of the other blocks within the image, as shown below.

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

To save time, PropellerManager does not download the entire application image. Instead, it downloads only the parts of the image from long 0 through the end of code (up to the start of variables) and then the Propeller chip itself writes zeros (0) to the rest of the RAM/EEPROM, after the end of code (up to 32 Kbytes), and inserts the initial call frame in the proper location. This effectively clears (initializes) all global variables to zero (0) and sets all available stack and free space to zero (0) as well.
*/

class PropellerImage
{

public:

    /**
      An enumeration containing all Propeller image types that PropellerManager supports.
      */
    enum ImageType {
        Binary,         ///< Program data-only image files (usually have a `.binary` extension)
        Eeprom          ///< Complete EEPROM images        (usually have a `.eeprom` extension)
    };

private:

    int EEPROM_SIZE;
    QByteArray  _image;
    QString     _filename;
    bool        _valid;
    QHash<quint8, QString> _clkmodesettings;
    QHash<quint8, QString> initClockModeSettings();

public:

    PropellerImage(QByteArray image, QString filename = "");

    /**
        @name Image Validation

        Apply some data integrity measures locally.
      */

    /**@{*/
    quint8      checksum();
    bool        isValid();
    /**@}*/

    /**
        @name Image Information

        Extract information about the current image.
      */

    /**@{*/
    QString     fileName();
    ImageType   imageType();

    int         imageSize();
    int         programSize();
    int         variableSize();
    int         stackSize();

    quint16     startOfCode();
    quint16     startOfVariables();
    quint16     startOfStackSpace();
    /**@}*/

    /**
        @name Low-Level Functions

        Read and write data to/from the image.
      */
    /**@{*/
    QByteArray  data();
    void        setData(QByteArray data);

    quint8      readByte(int pos);
    quint16     readWord(int pos);
    quint32     readLong(int pos);
    /**@}*/

    /**
        @name Clock Settings

        Read and write data to/from the image.
      */

    /**@{*/

    quint32     clockFrequency();                           ///< Get the current clock frequency of the image.
    void        setClockFrequency(quint32 frequency);       ///< Assign a new clock frequency to the image.
    quint8      clockMode();                                ///< Get an 8-bit integer containing the current clock mode.
    QString     clockModeText();                            ///< Get a human-readable string of the current clock mode.

    /**@}*/

};
