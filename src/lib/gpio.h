#pragma once

class Gpio
{
private:
    int pin;
    int dir;

public:
    enum Direction {
        In,
        Out
    };

    enum Level {
        Low,
        High
    };

    Gpio(int pin, Gpio::Direction dir);
    ~Gpio();


    int Read();
    int Write(int value);

    static int Export(int pin);
    static int Unexport(int pin);
    static int setDirection(int pin, Gpio::Direction dir);
    static int Read(int pin);
    static int Write(int pin, int value);
};

