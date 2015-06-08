OBJ
    pin : "Pinout"

CON
    LED_PIN = pin#LED
    LED_PERIOD = 1000

PUB Main
    dira[LED_PIN]~~
    repeat
        outa[LED_PIN]~
        repeat LED_PERIOD

        outa[LED_PIN]~~

        repeat LED_PERIOD
