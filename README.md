# TranzMON

TranzMON is a resident monitor for the Tranz 330 POS terminals.
It turns the POS into a multi purpose Z80 based computer that can also be used as an educational tool.

It's sufficient to burn the binary into a 27C256 EPROM and put it in place of the original one at U6 on the control board.

## Supported functions

- Serial console at 19200bps 8-n-1
- External keypad/VFD input

- Port IN/OUT (keypad and console)
- Memory READ/WRITE (keypad and console)
- Jump execution to an address (keypad and console)
- Memory upload/download via XMODEM (console)
- RTC (console and keypad)

### Console commands

- O xx yy          -> Output value yy to port xx
- I xx             -> Input from port xx
- J xxxx           -> Jump @xxxx
- F xxxx yy zz     -> Fill zz bytes of RAM with yy starting @xxxx
- W xxxx yy        -> Write zz @xxxx
- R xxxx yy        -> Print yy 16b blocks of RAM starting @xxxx
- X xxxx           -> Download data via XMODEM @xxxx
- U xxxx yyyy      -> Upload yyyy bytes via XMODEM  from xxxx
- T ddMMyyhhmmssdw -> Show or set current date (dw is day of week, starts at 00 with Sunday)

## Keypad commands

## Function table


