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

- **O** xx yy          -> Output value yy to port xx
- **I** xx             -> Input from port xx
- **J** xxxx           -> Jump execution @xxxx
- **F** xxxx yy zz     -> Fill zz bytes of RAM with yy starting @xxxx
- **W** xxxx yy        -> Write zz @xxxx
- **R** xxxx yy        -> Print yy 16b blocks of RAM starting @xxxx
- **X** xxxx           -> Download data via XMODEM @xxxx
- **U** xxxx yyyy      -> Upload yyyy bytes via XMODEM  from xxxx
- **T** ddMMyyhhmmssD  -> Show or set current date (D is day of week, starts at 0 with Sunday)

## Keypad commands

- **1**: Memory inspection/edit mode
- **2**: Port input mode
- **3**: Port output mode
- **4**: Jump mode
- **5**: RTC set mode

## Function table

Starting at a fixed address of 0x7F00, the monitor provides an address table of common functions that reside in ROM code, and that can be called from code uploaded in RAM.

**TODO**: Describe the table


