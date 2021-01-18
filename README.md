# MINOS

MINOS (stands for MINi OS) is a small ROM Monitor I wrote for the Ithaca Audio Z80 CPU card inside my S-100 computer.
The monitor is coded in C (SDCC compiler) and assembly and is very small (under 2K) to fit on a 2716 EPROM.
The MONITOR is meant to reside at address _0xF000_ and by default it communicates through Serial port A of N8VEM SerialIO card, 19200bps, HW flow control.

## Supported functions

- Port IN/OUT
- Memory READ/WRITE
- Jump execution to an address
- XModem data upload
- N8VEM IDE board booting

## Monitor commands

- **X**_aaaa_		- Loads a binary file to memory at address "aaaa". EG: X5000
- **I**_aa_		- INputs a byte from port address "aa". EG: I01
- **O**_aa_**,**_dd_	- OUTputs byte "dd" to port "aa". EG: O01,AA
- **R**_aaaa_		- Reads a byte from memory address "aaaa". EG: R5000
- **W**_aaaa_**,**_dd_	- Writes byte "dd" to memory address "aaaa". EG: W5000,04
- **J**_aaaa_		- Jumps execution to address "aaaa". EG: J3000
- **B**			- Copies in RAM (0x1000)  the content of sector 0 from the first drive in N8VEM IDE board then jumps the execution to it.

## Function table

At the end of the EPROM, there is an address table for some useful functions that can be reused by other code:
- **0xF7F4** n8vem\_ide\_init
- **0xF7F6** n8vem\_ide\_reg\_rd
- **0xF7F8** n8vem\_ide\_reg\_wr
- **0xF7FA** n8vem\_ide\_read
- **0xF7FC** getchar
- **0xF7FE** putchar

