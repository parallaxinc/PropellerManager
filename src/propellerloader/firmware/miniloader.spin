{{
    This is the mini-loader delivered via IP (Internet Protocol) packets sent to the Propeller over XBee Wi-Fi S6B.

    It is written to compile as a single unit, but for space and delivery reasons is not meant to run as, or be delivered as,
    a single unit.  IE: It can not be downloaded and executed as-is, below, in the standard fashion.
    
    The core of this application ("Mini-Loader Core" through "Constants and Variables") is delivered by the very first IP packet.
    It then runs and assists with the remainder of the download to deliver the target Propeller Application in a quick and reliable fashion.
    The remaining parts of this mini-loader application ("Finalization") are delivered when needed as "executable" packets.
    
    Any symbols in the assembly's "Constants" section labeled "[host init]" has their values set by the host before this application image is
    transmission via the first packet.
}}

CON
      _clkmode      = xtal1 + pll16x                                                                ' Standard clock mode * crystal frequency = 80 MHz
      _xinfreq      = 5_000_000

      MAX_PAYLOAD   = 1392                                                                          ' Maximum size of packet payload (in bytes)

      JMP_INST      = %010111_000                                                                   ' JMP instruction's I+E field  value
      TEST_INST     = %011000_000                                                                   ' TEST instruction's I+E field value
      #$1111_1110, HOSTVALUES, PACKET1, PACKET2, PACKET3, PACKET4, PACKET5, PACKET6

PUB Main

    coginit(0, @Loader, 0)

DAT
{{
    ## IP Loader PASM
    
    Runs in cog to receive target application at high-speed (compared to standard Propeller Download Protocol).

    Timing: Critical routine timing is shown in comments, like '4 and '6+, indication clock cycles instruction consumes.
     - `n`       : n clock cycles
     - `n+`      : n or more clock cycles
     - `n/m`     : n cycles (if jumping) or m cycles (if not jumping)
     - `x/n`     : don't care (if executed) or n cycles (if not executed)
     - `n![m+]`  : n cycles til moment of output/input, routine's iteration time is m or more cycles moment-to-moment
     - `n![m/l]` : like above except inner-loop iteration time is m cycles moment-to-moment, outer-loop is l cycles moment-to-moment

    ### MaxRxSenseError
    
    To support the FailSafe and EndOfPacket timeout feature, the :RxWait routine detects the edges of start bits with
    a polling loop.  This means there's an amount of edge detection error equal to the largest number of clock cycles
    starting from 1 cycle after input pin read to the moment the 1.5x bit period window is calculated.   This cycle
    path is indicated in the :RxWait routine's comments along with the calculated maximum error.  This value should
    be used as the MaxRxSenseError constant in the host software to adjust the 1.5x bit period downward so that the
    bit sense moment is closer to ideal in practice.

    ## Mini-Loader Core
}}
                            org     0

                            ' Initialize Tx pin

Loader                      mov     outa, TxPin
                            mov     dira, TxPin

                            ' Wait for resting RxPin

                            mov     BitDelay, BitTime       wc                                      ' Prep wait for 1/2 bit periods; clear c for first :RxWait
                            shr     BitDelay, #1
                            add     BitDelay, cnt
:RxWait     if_nc           mov     TimeDelay, #8*20                                                ' If RxPin active (c=0), reset sample count; 8 bytes * 20 samples/byte
                            waitcnt BitDelay, BitTime
                            test    RxPin, ina              wc                                      '    Check Rx state; c=0 (not resting), c=1 (resting)
                            djnz    TimeDelay, #:RxWait                                             ' Rx busy? Loop until resting 8 byte periods

                            ' Send ACK/NAK; "Ready" signal at initial baud first

Acknowledge                 mov     Bytes, #4                                                       ' Ready 1 long
:NextAckByte                mov     SByte, ExpectedID                                               ' ACK=next packet ID, NAK=previous packet ID
                            ror     ExpectedID, #8
                            and     SByte, #$FF                                                     '   Retain low byte only
                            or      SByte, #%1_0000_0000                                            '   Append stop bit; also acts as loop trigger
                            shl     SByte, #1                                                       '   Prepend start bit
                            mov     BitDelay, BitTime                                               '   Prep first bit window / ensure prev. stop bit window
                            add     BitDelay, cnt
:TxBit                      shr     SByte, #1               wc      '4                              '   Get next bit
                            waitcnt BitDelay, BitTime               '6+                             '     Wait for edge of bit window
                            muxc    outa, TxPin                     '4![18+]                        '     Output bit
                            tjnz    SByte, #:TxBit                  '4/8                            '   Loop for next bit
                            djnz    Bytes, #:NextAckByte                                            ' Loop for next byte

                            ' Set final bit period and failsafe timeout

                            mov     BitTime, FBitTime                                               ' Ensure final bit period for high-speed download
                            movs    :NextPktByte, #Failsafe         '4                              ' Reset timeout to Failsafe; restart Propeller if comm. lost between packets

                            ' Receive packet into Packet buffer

                            mov     PacketAddr, #Packet                                             ' Reset packet pointer
:NextPktLong                movd    :NextPktByte-1, PacketAddr      '4                              ' Point 'Packet{addr}' (dest field) at Packet buffer
                            movd    :BuffAddr, PacketAddr           '4
                            movd    :BuffAddr+1, PacketAddr         '4
                            mov     Bytes, #4                       '4                              '   Ready for 1 long
                            mov     Packet{addr}, #0                '4                              '   Pre-clear 1 buffer long
:NextPktByte                mov     TimeDelay, Timeout{addr}        '4                              '   Set timeout; FailSafe on entry, EndOfPacket on reentry
                            mov     BitDelay, BitTime1_5    wc      '4                              '     Prep first bit sample window; c=0 for first :RxWait
                            mov     SByte, #0                       '4
:RxWait                     muxc    SByte, #%0_1000_0000    wz      '4            ┌┐                '     Wait for Rx start bit (falling edge); Prep SByte for 8 bits
                            test    RxPin, ina              wc      '4![12/48/80]┐││                '       Check Rx state; c=0 (not resting), c=1 (resting)
              if_z_or_c     djnz    TimeDelay, #:RxWait             '4/x         └┘│                '     No start bit (z or c)? loop until timeout
              if_z_or_c     jmp     #:TimedOut                      'x/4           │                '     No start bit (z or c) and timed-out? Exit
                            add     BitDelay, cnt                   '4             ┴*23             '     Set time to...             (*See MaxRxSenseError note)
:RxBit                      waitcnt BitDelay, BitTime               '6+                             '     Wait for center of bit
                            test    RxPin, ina              wc      '4![22/x/x]                     '       Sample bit; c=0/1
                            muxc    SByte, #%1_0000_0000            '4                              '       Store bit
                            shr     SByte, #1               wc      '4                              '       Adjust result; c=0 (continue), c=1 (done)
              if_nc         jmp     #:RxBit                         '4                              '     Continue? Loop until done
:BuffAddr                   or      Packet{addr}, SByte             '4                              '     store into long (low byte first)
                            ror     Packet{addr}, #8                '4                              '     and adjust long
                            movs    :NextPktByte, #EndOfPacket      '4                              '     Replace Failsafe timeout with EndOfPacket timeout
                            djnz    Bytes, #:NextPktByte            '4/8                            '   Loop for all bytes of long
                            add     PacketAddr, #1                  '4                              '   Done, increment packet pointer for next time
                            jmp     #:NextPktLong                   '4                              ' Loop in case more arrives


                            ' Timed out; no packet?

:TimedOut                   mov     TimeDelay, :NextPktByte                                         ' Check type of timeout (no packet or end of packet)
                            and     TimeDelay, #$1FF
                            cmp     TimeDelay, #Failsafe    wz                                      '   z=no packet, nz=end of packet
TOVector      if_z          clkset  Reset                                                           '   If no packet, restart Propeller

                            ' Check packet ID

                            cmps    PacketID, ExpectedID    wz                                      ' Received expected packet? z=yes
              if_nz         jmp     #Acknowledge                                                    '   No? Acknowledge negatively (ExpectedID untouched)
                            cmps    ExpectedID, #1          wc,wr                                   '   Yes; ready next packet and check for executable (dec ExpectedID; c=execute packet; new ExpectedID < 0)
              if_c          jmp     #packetdata                                                     ' Execute packet? Run packet code just received

                            ' Copy target packet to Main RAM; ignore duplicate
                            ' Else, copy packet to Main RAM

                            sub     PacketAddr, #packetdata                                         ' Make PacketAddr into a loop counter
    :Copy                   wrlong  packetdata{addr}, MainRAMAddr                                   ' Write packet long to Main RAM
                            add     MainRAMAddr, #4                                                 '   Increment Main RAM address
                            add     :Copy, IncDest                                                  '   Increment packetdata address
                            djnz    PacketAddr, #:Copy                                              ' Loop for whole packet
                            movd    :Copy, #packetdata                                              ' Reset packetdata{addr} for next time
                            jmp     #Acknowledge                                                    ' Loop to acknowledge positively





  MainRAMAddr               long    0                                                               ' Address in Main RAM
  Zero                                                                                              ' Zero value (for clearing RAM) and
    Checksum                long    0                                                               '   Checksum (for verifying RAM)

  Reset                     long    %1000_0000                                                      ' Propeller restart value (for CLK register)
  IncDest                   long    %1_0_00000000                                                   ' Value to increment a register's destination field
  EndOfRAM                  long    $8000                                                           ' Address of end of RAM+1
  CallFrame                 long    $FFF9_FFFF                                                      ' Initial call frame value
  Interpreter               long    $0001 << 18 + $3C01 << 4 + %0000                                ' Coginit value to launch Spin interpreter
  RxPin                     long    |< 31                                                           ' Receive pin mask (P31)
  TxPin                     long    |< 30                                                           ' Transmit pin mask (P30)

' Host Initialized Values
                            long    HOSTVALUES
  BitTime                                                                                           ' Bit period (in clock cycles)
    IBitTime                long    80_000_000 / 115_200                     '[host init]           '   Initial bit period (at startup)
    FBitTime                long    80_000_000 / 230_400                     '[host init]           '   Final bit period (for download)
  BitTime1_5                long    TRUNC(1.5 * 80_000_000.0 / 230_400.0)    '[host init]           ' 1.5x bit period; used to align to center of received bits
  Timeout
    Failsafe                long    2 * 80_000_000 / (3 * 4)                 '[host init]           ' Failsafe timeout (2 seconds worth of RxWait loop iterations)
    EndOfPacket             long    2 * 80_000_000 / 230_400 * 10 * (3 * 4)  '[host init]           ' EndOfPacket timeout (2 bytes worth of RxWait loop iterations)
  ExpectedID                long    0                                        '[host init]           ' Expected Packet ID


  TimeDelay                 res     1                                                               ' Timout delay
  BitDelay                  res     1                                                               ' Bit time delay
  SByte                     res     1                                                               ' Serial Byte; received or to transmit
  Bytes                                                                                             ' Byte and
    Longs                   res     1                                                               '   long counter
  PacketAddr                res     1                                                               ' PacketAddr
  Packet                                                                                            ' Packet buffer
    PacketID                res     1                                                               '  Header:  Packet ID number
    packetdata              res     (MAX_PAYLOAD / 4) - 1                                           '   Payload: Packet data (longs); (max size in longs) - header


{{
    Finalize and Verify RAM (Executable Packet Code)
    ------------------------------------------------

    The following are routines delivered inside packets, as needed.  They are "executable" packets delivered and run from packets with ID's
    of 0 and lower.
}}

                            org     packetdata-1                                                    ' Line up executable packet code

                            long    PACKET1

                            ' Entire Application Received; clear remaining RAM

                            mov     Longs, EndOfRAM                                                 ' Determine number of registers to clear
                            sub     Longs, MainRAMAddr
                            shr     Longs, #2               wz
    :Clear    if_nz         wrlong  Zero, MainRAMAddr                                               ' Clear register
              if_nz         add     MainRAMAddr, #4                                                 '   Increment Main RAM address
              if_nz         djnz    Longs, #:Clear                                                  ' Loop until end; Main RAM Addr = $8000

                            ' Insert initial call frame

                            rdword  Longs, #5<<1                                                    ' Get next stack address
                            sub     Longs, #4                                                       ' Adjust to previous stack address
                            wrlong  CallFrame, Longs                                                ' Store initial call frame
                            sub     Longs, #4
                            wrlong  CallFrame, Longs

                            ' Verify RAM; calculate checksum                                        ' (Checksum = 0, MainRAMAddr = $8000)

    :Validate               sub     MainRAMAddr, #1                                                 ' Decrement Main RAM Address
                            rdbyte  Bytes, MainRAMAddr                                              '   Read next byte from Main RAM
                            add     Checksum, Bytes                                                 '   Adjust checksum
                            tjnz    MainRAMAddr, #:Validate                                         ' Loop for all RAM
                            neg     ExpectedID, Checksum                                            ' Set ExpectedID to negative checksum
                            jmp     #Acknowledge                                                    ' ACK=Proper -Checksum, NAK=Improper Checksum


{{
    Prep for Validation and Launch (Executable Packet Code)
    -------------------------------------------------------

    The following code "may" be executed twice (by design) if the final launch packet code (next section below) is not received. The first launch packet
    delivers this code which modifies the Timeout feature to execute the packet code (then next packet code normally, or this code again in the case noted
    above) and then it sends positive acknowledgement of the first launch packet to the host. The second pass of this code only happens if the final launch
    packet is not received- it will effectively launch the target application without further communication with the host. However, if the final launch
    packet is received, it replaces this code with a simpler version (next section below) meant to execute just once, without further communication with
    the host.
}}

                            org     packetdata-1                                                    ' Line up executable packet code

                            long    PACKET2

  LaunchStart               movi    TOVector, #JMP_INST                                             ' We can safely launch upon timeout now; replace timeout vector's
                            movs    TOVector, #packetdata+3                                         '   restart instruction with jump past acknowledgement, below
                            jmp     #Acknowledge                                                    ' Jump (pass 1) to send acknowledgement

                            rdword  MainRAMAddr, #3<<1                                              ' Else (pass 2), check program base address
                            cmp     MainRAMAddr, #$10       wz                                      ' nz=Invalid
              if_nz         clkset  Reset                                                           ' Invalid?  Reset Propeller

                            coginit interpreter                                                     ' Relaunch with Spin Interpreter


{{
    Validate and Launch (Executable Packet Code)
    --------------------------------------------

    The following code "may" never be received if the final launch packet is not received. This is an acceptable "soft" error. If this code isn't received,
    the first launch packet (previous section above) will launch the target application anyway without further communication with the host.
}}

                            org     packetdata-1                                                    ' Line up executable packet code

                            long    PACKET3

LaunchFinal                 rdword  MainRAMAddr, #3<<1                                              ' Check program base address
                            cmp     MainRAMAddr, #$10       wz                                      ' nz=Invalid
              if_nz         clkset  Reset                                                           ' Invalid?  Reset Propeller

                            coginit interpreter                                                     ' Relaunch with Spin Interpreter
