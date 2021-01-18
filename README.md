# TranzMON

## Supported functions

- Port IN/OUT
- Memory READ/WRITE
- Jump execution to an address
- XModem data upload

## Monitor commands

- **X**_aaaa_		- Loads a binary file to memory at address "aaaa". EG: X5000
- **I**_aa_		- INputs a byte from port address "aa". EG: I01
- **O**_aa_**,**_dd_	- OUTputs byte "dd" to port "aa". EG: O01,AA
- **R**_aaaa_		- Reads a byte from memory address "aaaa". EG: R5000
- **W**_aaaa_**,**_dd_	- Writes byte "dd" to memory address "aaaa". EG: W5000,04
- **J**_aaaa_		- Jumps execution to address "aaaa". EG: J3000

## Function table

At the end of the EPROM, there is an address table for some useful functions that can be reused by other code:

