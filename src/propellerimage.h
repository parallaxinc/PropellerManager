#pragma once

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QHash>

#include "utility.h"

/**
  \class PropellerImage

  PropellerImage is a class for analyzing and modifying prebuilt
  propeller binary and EEPROM images.

  - Converts between binary and EEPROM image formats
  - Deep information about binary
  - Ability to customize and retarget binaries
  - Can validate images for accuracy
*/
class PropellerImage
{

public:

    enum Type {
        Binary,
        Eeprom
    };

private:

    Type        _type;
    QByteArray  _image;
    bool        _valid;
    QHash<quint8, QString> _clkmodesettings;
    QHash<quint8, QString> initClockModeSettings();

public:

    PropellerImage(QByteArray image, Type type);

    /**
        @name Image Validation

        Apply some data integrity measures locally.
      */

    /**@{*/
    int         checksum();
    bool        isValid();
    /**@}*/

    /**
        @name Image Information

        Extract information about the current image.
      */

    /**@{*/
    int         imageSize();
    int         stackSize();
    int         programSize();
    int         variableSize();
    int         freeSpace();
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

    /** Get the current clock frequency of the image. */
    quint32     clockFrequency();

    /** Assign a new clock frequency to the image. */
    void        setClockFrequency(int frequency);

    /** Get an 8-bit integer containing the current clock mode. */
    quint8      clockMode();

    /** Get a human-readable string of the current clock mode. */
    QString     clockModeText();

    /**@}*/

//    void setType(PropellerImage::Type type)

};
