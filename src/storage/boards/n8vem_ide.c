#include "n8vem_ide.h"

#define IDE_BASE_ADDR 0x30

// IDE I/O ports
static __sfr __at (IDE_BASE_ADDR+0x03) IDE_Ctrl;
static __sfr __at (IDE_BASE_ADDR+0x00) IDE_PortA;
static __sfr __at (IDE_BASE_ADDR+0x01) IDE_PortB;
static __sfr __at (IDE_BASE_ADDR+0x02) IDE_PortC;

// DEFINES:
// SHD Config
// IDE SHD register structure
// LBA -> Enables LBA mode
// DEV -> Device select
// hea -> Head number
// obs | LBA | obs | DEV | hea | hea | hea | hea
//#define IDE_SHD_CFG			0b11100000
#define IDE_SHD_CFG				0b10100000

// 8255 I/O Modes
#define MODE_8255_INPUT		0b10010010
#define MODE_8255_OUTPUT	0b10000000

// IDE lines
#define IDE_LINE_A0		0x01
#define IDE_LINE_A1		0x02
#define IDE_LINE_A2		0x04
#define IDE_LINE_CS0	0x08
#define IDE_LINE_CS1	0x10
#define IDE_LINE_WR		0x20
#define IDE_LINE_RD		0x40
#define IDE_LINE_RST	0x80

// IDE REGISTERS
#define IDE_REG_DATA	(IDE_LINE_CS0)
#define IDE_REG_ERR		(IDE_LINE_CS0 | IDE_LINE_A0)
#define IDE_REG_SECCNT	(IDE_LINE_CS0 | IDE_LINE_A1)
#define IDE_REG_SHD		(IDE_LINE_CS0 | IDE_LINE_A1 | IDE_LINE_A2)
#define IDE_REG_SEC		(IDE_LINE_CS0 | IDE_LINE_A0 | IDE_LINE_A1)
#define IDE_REG_CYLL	(IDE_LINE_CS0 | IDE_LINE_A2)
#define IDE_REG_CYLH	(IDE_LINE_CS0 | IDE_LINE_A0 | IDE_LINE_A2)
#define IDE_REG_CMD		(IDE_LINE_CS0 | IDE_LINE_A0 | IDE_LINE_A1 | IDE_LINE_A2)
#define IDE_REG_STAT	(IDE_LINE_CS0 | IDE_LINE_A0 | IDE_LINE_A1 | IDE_LINE_A2)
#define IDE_REG_CTRL	(IDE_LINE_CS1 | IDE_LINE_A1 | IDE_LINE_A2)
#define IDE_REG_ASTAT	(IDE_LINE_CS1 | IDE_LINE_A0 | IDE_LINE_A1 | IDE_LINE_A2)

// IDE COMMANDS
#define IDE_CMD_RECAL	0x10
#define IDE_CMD_RD		0x20
#define IDE_CMD_WR		0x30
#define IDE_CMD_INIT	0x91
#define IDE_CMD_ID		0xEC
#define IDE_CMD_SPDOWN	0xE0
#define IDE_CMD_SPUP	0xE1

uint8_t n8vem_ide_block_rd(uint8_t *dest);
uint8_t n8vem_ide_waitNotBusy(void);
uint8_t n8vem_ide_waitDRQ(void);
void n8vem_ide_setLBAAddr(uint8_t sect, uint8_t head, uint8_t cyll, uint8_t cylh);

uint8_t n8vem_ide_init(void) {
	uint8_t delay = 0xFF;

	// Set 8255 to input
	IDE_Ctrl = MODE_8255_INPUT;

	// Reset the drive
	IDE_PortC = IDE_LINE_RST;

	while(delay--) { 
		__asm
			nop
		__endasm;
	};
	delay = 0xFF;
	
	IDE_PortC = 0;

	n8vem_ide_reg_wr(IDE_REG_SHD, IDE_SHD_CFG);
	
	while(delay--) {
		__asm
			nop
		__endasm;
		if (!(n8vem_ide_reg_rd(IDE_REG_STAT) & 0x80)) return 0; // Check the busy flag is off
	}

	// If we got here, the init timed out
	return 0xFF;
}

uint8_t n8vem_ide_reg_rd(uint8_t reg) {
	uint8_t reg_val = 0;

	IDE_PortC = reg;
	IDE_PortC = reg | IDE_LINE_RD;

	reg_val = IDE_PortA;

	IDE_PortC = reg;
	IDE_PortC = 0;

	return reg_val;
}

void n8vem_ide_reg_wr(uint8_t reg, uint8_t val) {
	IDE_Ctrl = MODE_8255_OUTPUT;
	
	IDE_PortA  = val;

	IDE_PortC = reg;
	IDE_PortC = reg | IDE_LINE_WR;
	IDE_PortC = reg;
	IDE_PortC = 0;

	IDE_Ctrl = MODE_8255_INPUT;
}

uint8_t n8vem_ide_block_rd(uint8_t *dest) {
	uint8_t count = 0xFF;

	while(count--) {
		IDE_PortC = IDE_REG_DATA;
		IDE_PortC = IDE_REG_DATA | IDE_LINE_RD;

		dest[1] = IDE_PortB;
		dest[0] = IDE_PortA;

		dest+=2;
	}
		
	IDE_PortC = IDE_REG_DATA;
	IDE_PortC = 0;

	if(n8vem_ide_reg_rd(IDE_REG_STAT) & 0x01) return 0xFF;
	else return 0;
}

uint8_t n8vem_ide_waitNotBusy(void) {
	uint8_t retries = 0xFF;

	while(retries--) {
		__asm
			nop
		__endasm;
		// Check DRIVE READY and DRIVE BUSY bits
		if((n8vem_ide_reg_rd(IDE_REG_STAT) & 0xC0) == 0x40) return 0;
	}

	return 0xFF;
}

uint8_t n8vem_ide_waitDRQ(void) {
	uint8_t retries = 0xFF;

	while(retries--) {
		__asm
			nop
		__endasm;
		// Check DATA REQUEST and DRIVE BUSY bits
		if((n8vem_ide_reg_rd(IDE_REG_STAT) & 0x88) == 0x08) return 0;
	}

	return 0xFF;
}

void n8vem_ide_setLBAAddr(uint8_t sect, uint8_t head, uint8_t cyll, uint8_t cylh) {
	n8vem_ide_reg_wr(IDE_REG_SEC, sect);
	n8vem_ide_reg_wr(IDE_REG_SHD, IDE_SHD_CFG | head);
	n8vem_ide_reg_wr(IDE_REG_CYLL, cyll);
	n8vem_ide_reg_wr(IDE_REG_CYLH, cylh);
	n8vem_ide_reg_wr(IDE_REG_SECCNT, 1);
}

uint8_t n8vem_ide_read(uint8_t *dest, uint8_t sect, uint8_t head, uint8_t cyll, uint8_t cylh) {
	n8vem_ide_setLBAAddr(sect, head, cyll, cylh);

	if(n8vem_ide_waitNotBusy()) return 0xFF;

	n8vem_ide_reg_wr(IDE_REG_CMD, IDE_CMD_RD);
	
	if(n8vem_ide_waitDRQ()) return 0xFF;

	return n8vem_ide_block_rd(dest);
}
