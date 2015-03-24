class GPIO
{
private:
    int pin;
    int dir;

public:
    GPIO(int pin, int dir);
    ~GPIO();

    int Read();
    int Write(int value);

    static int Export(int pin);
    static int Unexport(int pin);
    static int Direction(int pin, int dir);
    static int Read(int pin);
    static int Write(int pin, int value);
};

