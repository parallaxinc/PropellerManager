#include "gpio.h"

#include <QFile>
#include <QString>
#include <QTextStream>
#include <QDebug>
#include <QThread>

#include "logging.h"

Gpio::Gpio(int pin, Gpio::Direction dir)
{
    this->pin = pin;
    this->dir = dir;
    Gpio::Export(pin);
    Gpio::setDirection(pin, dir);
}

Gpio::~Gpio()
{
    Gpio::Unexport(pin);
}

int Gpio::Read()
{
    return Gpio::Read(pin);
}

int Gpio::Write(int value)
{
    return Gpio::Write(pin, value);
}

int Gpio::Export(int pin)
{
    QFile file("/sys/class/gpio/export");
    if (!file.open(QIODevice::WriteOnly))
    {
        qCDebug(pgpio) << "Failed to open export for writing.";
        return -1;
    }

    QTextStream stream(&file);
    stream << pin << endl;

    QThread::msleep(100);

    if (stream.status() != 0)
    {
        qCDebug(pgpio) << "Failed to write export";
    }
    return 0;
}

int Gpio::Unexport(int pin)
{
    QFile file("/sys/class/gpio/unexport");
    if (!file.open(QIODevice::WriteOnly))
    {
        qCDebug(pgpio) << "Failed to open unexport for writing.";
        return -1;
    }

    QTextStream stream(&file);
    stream << pin << endl;

    if (stream.status() != 0)
    {
        qCDebug(pgpio) << "Failed to write unexport";
    }
    return 0;
}

int Gpio::setDirection(int pin, Gpio::Direction dir)
{
    QString filename = QString("/sys/class/gpio/gpio%1/direction").arg(pin);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        qCDebug(pgpio) << "Failed to open" << filename << "for writing.";
        return -1;
    }

    QTextStream stream(&file);
    if (dir == Gpio::Out)
    {
        stream << "out" << endl;
    }
    else
    {
        stream << "in" << endl;
    }

    if (stream.status() != 0)
    {
        qCDebug(pgpio) << "Failed to write GPIO direction";
    }
    return 0;
}

int Gpio::Read(int pin)
{
    QString filename = QString("/sys/class/gpio/gpio%1/value").arg(pin);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCDebug(pgpio) << "Failed to open" << filename << "for reading.";
        return -1;
    }

    int value = 0;
    QTextStream stream(&file);
    stream >> value;

    if (stream.status() != 0)
    {
        qCDebug(pgpio) << "Failed to read GPIO value";
        return -1;
    }
    return 0;
}

int Gpio::Write(int pin, int value)
{
    QString filename = QString("/sys/class/gpio/gpio%1/value").arg(pin);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        qCDebug(pgpio) << "Failed to open" << filename << "for writing.";
        return -1;
    }

    QTextStream stream(&file);
    stream << value;

    if (stream.status() != 0)
    {
        qCDebug(pgpio) << "Failed to write GPIO value";
        return -1;
    }

    return 0;
}
