// GPIO.cpp is based on Guillermo A. Amaral's blink.c example as
// featured in p1load

/* blink.c
*
* Raspberry Pi GPIO example using sysfs interface.
* Guillermo A. Amaral B. <g@maral.me>
*/

#include "GPIO.h"

extern "C"
{
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
}

#define IN   0
#define OUT  1

#define LOW  0
#define HIGH 1

#define PIN  24 /* P1-18 */
#define POUT 4  /* P1-07 */


GPIO::GPIO(int pin, int dir)
{
    this->pin = pin;
    this->dir = dir;
    GPIO::Export(pin);
    GPIO::Direction(pin, dir);
}

GPIO::~GPIO()
{
    GPIO::Unexport(pin);
}

int GPIO::Read()
{
    GPIO::Read(pin);
}

int GPIO::Write(int value)
{
    return GPIO::Write(pin, value);
}

int GPIO::Export(int pin)
{
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open export for writing!\n");
        return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(0);
}

int GPIO::Unexport(int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(0);
}

int GPIO::Direction(int pin, int dir)
{
    static const char s_directions_str[]  = "in\0out";

#define DIRECTION_MAX 35
    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio direction for writing!\n");
        return(-1);
    }

    if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
        fprintf(stderr, "Failed to set direction!\n");
        return(-1);
    }

    close(fd);
    return(0);
}

int GPIO::Read(int pin)
{
#define VALUE_MAX 30
    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        return(-1);
    }

    if (-1 == read(fd, value_str, 3)) {
        fprintf(stderr, "Failed to read value!\n");
        return(-1);
    }

    close(fd);

    return(atoi(value_str));
}

int GPIO::Write(int pin, int value)
{
    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for writing!\n");
        return(-1);
    }

    if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
        fprintf(stderr, "Failed to write value!\n");
        return(-1);
    }

    close(fd);
    return(0);
}

int main(int argc, char *argv[])
{
    int repeat = 9;

    /*
     * Enable GPIO pins
     */
    if (-1 == GPIO::Export(POUT) || -1 == GPIO::Export(PIN))
        return(1);

    /*
     * Set GPIO directions
     */
    if (-1 == GPIO::Direction(POUT, OUT) || -1 == GPIO::Direction(PIN, IN))
        return(2);

    do {
        /*
         * Write GPIO value
         */
        if (-1 == GPIO::Write(POUT, repeat % 2))
            return(3);

        /*
         * Read GPIO value
         */
        printf("I'm reading %d in GPIO %d\n", GPIO::Read(PIN), PIN);

        usleep(500 * 1000);
    }
    while (repeat--);

    /*
     * Disable GPIO pins
     */
    if (-1 == GPIO::Unexport(POUT) || -1 == GPIO::Unexport(PIN))
        return(4);

    return(0);
}

