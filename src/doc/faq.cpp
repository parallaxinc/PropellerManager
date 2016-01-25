/** @page faq Frequently Asked Questions
 *
### Why doesn't PropellerManager doesn't work with _____ board?

If you have difficulty getting your board to work with PropellerManager, there 
are a few things to try.

First, upgrade to the latest version of PropellerManager.

If you have a Saleae logic analyzer, and it's at all possible, collecting the 
output of the analyzer on pins 31, 30, and RESn while attempting a download are 
important to getting this to work. Also helpful would be a schematic of the 
board.

Verify that the board uses an FTDI chip and DTR for reset (a rare crop of 
boards use RST or miscellaneous USB chips for some reason). If you are on 
Linux, you can find out pretty quickly what serial device it is by running 
'dmesg -w' and then reattaching the Chameleon. You'll see something like this, 
which I'd like you to send me.

    [10448.650872] usb 2-1.4.4: new full-speed USB device number 7 using ehci-pci
    [10448.749401] usb 2-1.4.4: New USB device found, idVendor=0403, idProduct=6001
    [10448.749406] usb 2-1.4.4: New USB device strings: Mfr=1, Product=2, SerialNumber=3
    [10448.749408] usb 2-1.4.4: Product: FT232R USB UART
    [10448.749410] usb 2-1.4.4: Manufacturer: FTDI
    [10448.749412] usb 2-1.4.4: SerialNumber: A2003Eh9
    [10448.752089] ftdi_sio 2-1.4.4:1.0: FTDI USB Serial Device converter detected
    [10448.752124] usb 2-1.4.4: Detected FT232RL
    [10448.752654] usb 2-1.4.4: FTDI USB Serial Device converter now attached to ttyUSB0

If you really want to explore this, you can download and build PropellerManager 
yourself. Building it is easy: install Qt, then:

    $ qmake
    $ make -j

A command-line app will be built at:

    bin/propman

Use this to test whether your changes work.

Investigate the PropellerDevice::reset() function found in 
src/device/propellerdevice.cpp. The reset pulse is very quick, then an 80ms 
wait. You can hold the reset longer by adding QThread::msleep(milliseconds) 
between where setDataTerminalReady is set and unset.

If you can tweak the values and find something that works, send a pull request 
with your changes and we will review them.

*/
