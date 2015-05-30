CON
        _clkmode = xtal1 + pll16x                                               'Standard clock mode * crystal frequency = 80 MHz
        _xinfreq = 5_000_000

PUB Main
  DIRA[7..0]~~
  repeat
    !OUTA[0]
    waitcnt(clkfreq / 12 + cnt)
    !OUTA[1]
    waitcnt(clkfreq / 12 + cnt)
    !OUTA[2]
    waitcnt(clkfreq / 12 + cnt)
    !OUTA[3]
    waitcnt(clkfreq / 12 + cnt)
    !OUTA[4]
    waitcnt(clkfreq / 12 + cnt)
    !OUTA[5]
    waitcnt(clkfreq / 12 + cnt)
    !OUTA[6]
    waitcnt(clkfreq / 12 + cnt)
    !OUTA[7]
    waitcnt(clkfreq / 12 + cnt)

        