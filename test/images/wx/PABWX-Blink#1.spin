CON
        _clkmode = xtal1 + pll16x                                               'Standard clock mode * crystal frequency = 80 MHz
        _xinfreq = 5_000_000

PUB Main
  DIRA[26]~~
  repeat
    !OUTA[26]
    waitcnt(clkfreq / 2 + cnt)
        