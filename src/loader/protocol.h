#pragma once

#include <QByteArray>

/**
@class PropellerProtocol

@brief The PropellerProtocol class contains algorithms for interacting with the Propeller firmware.

This class implements the Propeller LFSR handshake sequence, the bit stream translator,
and other functions needed to implement the Propeller download sequence.
*/

namespace Command {
    enum Command {
        Shutdown,
        Run,
        Write,
        WriteRun
    };
};

namespace Propeller {

const int _max_data_size = 1392;

const int request_size = 69;
const quint8 request[69] = {
    0x49,
    0xAA,0x52,0xA5,0xAA,0x25,0xAA,0xD2,0xCA,0x52,0x25,0xD2,0xD2,0xD2,0xAA,0x49,0x92,
    0xC9,0x2A,0xA5,0x25,0x4A,0x49,0x49,0x2A,0x25,0x49,0xA5,0x4A,0xAA,0x2A,0xA9,0xCA,
    0xAA,0x55,0x52,0xAA,0xA9,0x29,0x92,0x92,0x29,0x25,0x2A,0xAA,0x92,0x92,0x55,0xCA,
    0x4A,0xCA,0xCA,0x92,0xCA,0x92,0x95,0x55,0xA9,0x92,0x2A,0xD2,0x52,0x92,0x52,0xCA,
    0xD2,0xCA,0x2A,0xFF
};


const int reply_size = 125;
const quint8 reply[reply_size] = {
    0xEE,0xCE,0xCE,0xCF,0xEF,0xCF,0xEE,0xEF,0xCF,0xCF,0xEF,0xEF,0xCF,0xCE,0xEF,0xCF,
    0xEE,0xEE,0xCE,0xEE,0xEF,0xCF,0xCE,0xEE,0xCE,0xCF,0xEE,0xEE,0xEF,0xCF,0xEE,0xCE,
    0xEE,0xCE,0xEE,0xCF,0xEF,0xEE,0xEF,0xCE,0xEE,0xEE,0xCF,0xEE,0xCF,0xEE,0xEE,0xCF,
    0xEF,0xCE,0xCF,0xEE,0xEF,0xEE,0xEE,0xEE,0xEE,0xEF,0xEE,0xCF,0xCF,0xEF,0xEE,0xCE,
    0xEF,0xEF,0xEF,0xEF,0xCE,0xEF,0xEE,0xEF,0xCF,0xEF,0xCF,0xCF,0xCE,0xCE,0xCE,0xCF,
    0xCF,0xEF,0xCE,0xEE,0xCF,0xEE,0xEF,0xCE,0xCE,0xCE,0xEF,0xEF,0xCF,0xCF,0xEE,0xEE,
    0xEE,0xCE,0xCF,0xCE,0xCE,0xCF,0xCE,0xEE,0xEF,0xEE,0xEF,0xEF,0xCF,0xEF,0xCE,0xCE,
    0xEF,0xCE,0xEE,0xCE,0xEF,0xCE,0xCE,0xEE,0xCF,0xCF,0xCE,0xCF,0xCF
};

/**
  Take Propeller Application image (InImage) and generate Propeller Download stream
  (OutImage) in an optimized format (3, 4, or 5 bits per byte; 7 to 11 bytes per long).

Note: for every 5 contiguous bits in Propeller Application Image (LSB first) 3, 4, or
5 bits can be translated to a byte.  The process requires 5 bits input (ie: indexed
into the array) and gets a byte out that contains the first 3, 4, or 5 bits encoded in
the Propeller Download stream format. If less than 5 bits were translated, the
remaining bits leads the next 5 bit translation unit input to the translation process.}
 **/

// Propeller Download Stream Translator array.  Index into this array using the "Binary Value" (usually 5 bits) to translate,
// the incoming bit size (again, usually 5), and the desired data element to retrieve (dtTx = translation, dtBits = bit count
// actually translated.

// Binary    Incoming    Translation
// Value,    Bit Size,   or Bit Count
const quint8 translator[32][5][2] = {
    //  ***  1-BIT  ***        ***  2-BIT  ***        ***  3-BIT  ***        ***  4-BIT  ***        ***  5-BIT  ***
    { /*%00000*/ {0xFE, 1},  /*%00000*/ {0xF2, 2},  /*%00000*/ {0x92, 3},  /*%00000*/ {0x92, 3},  /*%00000*/ {0x92, 3} },
    { /*%00001*/ {0xFF, 1},  /*%00001*/ {0xF9, 2},  /*%00001*/ {0xC9, 3},  /*%00001*/ {0xC9, 3},  /*%00001*/ {0xC9, 3} },
    {            {0,    0},  /*%00010*/ {0xFA, 2},  /*%00010*/ {0xCA, 3},  /*%00010*/ {0xCA, 3},  /*%00010*/ {0xCA, 3} },
    {            {0,    0},  /*%00011*/ {0xFD, 2},  /*%00011*/ {0xE5, 3},  /*%00011*/ {0x25, 4},  /*%00011*/ {0x25, 4} },
    {            {0,    0},             {0,    0},  /*%00100*/ {0xD2, 3},  /*%00100*/ {0xD2, 3},  /*%00100*/ {0xD2, 3} },
    {            {0,    0},             {0,    0},  /*%00101*/ {0xE9, 3},  /*%00101*/ {0x29, 4},  /*%00101*/ {0x29, 4} },
    {            {0,    0},             {0,    0},  /*%00110*/ {0xEA, 3},  /*%00110*/ {0x2A, 4},  /*%00110*/ {0x2A, 4} },
    {            {0,    0},             {0,    0},  /*%00111*/ {0xFA, 3},  /*%00111*/ {0x95, 4},  /*%00111*/ {0x95, 4} },
    {            {0,    0},             {0,    0},             {0,    0},  /*%01000*/ {0x92, 3},  /*%01000*/ {0x92, 3} },
    {            {0,    0},             {0,    0},             {0,    0},  /*%01001*/ {0x49, 4},  /*%01001*/ {0x49, 4} },
    {            {0,    0},             {0,    0},             {0,    0},  /*%01010*/ {0x4A, 4},  /*%01010*/ {0x4A, 4} },
    {            {0,    0},             {0,    0},             {0,    0},  /*%01011*/ {0xA5, 4},  /*%01011*/ {0xA5, 4} },
    {            {0,    0},             {0,    0},             {0,    0},  /*%01100*/ {0x52, 4},  /*%01100*/ {0x52, 4} },
    {            {0,    0},             {0,    0},             {0,    0},  /*%01101*/ {0xA9, 4},  /*%01101*/ {0xA9, 4} },
    {            {0,    0},             {0,    0},             {0,    0},  /*%01110*/ {0xAA, 4},  /*%01110*/ {0xAA, 4} },
    {            {0,    0},             {0,    0},             {0,    0},  /*%01111*/ {0xD5, 4},  /*%01111*/ {0xD5, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%10000*/ {0x92, 3} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%10001*/ {0xC9, 3} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%10010*/ {0xCA, 3} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%10011*/ {0x25, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%10100*/ {0xD2, 3} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%10101*/ {0x29, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%10110*/ {0x2A, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%10111*/ {0x95, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%11000*/ {0x92, 3} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%11001*/ {0x49, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%11010*/ {0x4A, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%11011*/ {0xA5, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%11100*/ {0x52, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%11101*/ {0xA9, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%11110*/ {0xAA, 4} },
    {            {0,    0},             {0,    0},             {0,    0},             {0,    0},  /*%11111*/ {0x55, 5} }
};
};

// Generate Propeller Loader Download Stream from adjusted LoaderImage (above); Output delivered to LoaderStream and LoaderStreamSize.

// It should be noted that the Propeller Protocol is **Little-Endian**

class PropellerProtocol
{

private:
    QByteArray _reply;
    QByteArray _request;

    int lfsr(int * seed);
    QList<char> buildLfsrSequence(int size);

public:
    PropellerProtocol();
    QByteArray buildRequest(Command::Command command = Command::Shutdown);
    static QByteArray encodeData(QByteArray image);
    static QByteArray encodeLong(quint32 value);
    static QByteArray packLong(quint32 value);

    QByteArray reply();
    QByteArray request();

};
