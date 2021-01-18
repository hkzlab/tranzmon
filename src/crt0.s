;; Custom CRT0 for ITHACA AUDIO Z80 CPU monitor
;; Taken from SDCC

	.module crt0
	
	.globl	_main

	;; Console I/O functions to export
	.globl  _putchar
	.globl  _getchar

	;; IDE functions to export
	;;.globl	_n8vem_ide_init
	;;.globl	_n8vem_ide_read
	;;.globl 	_n8vem_ide_reg_rd
	;;.globl	_n8vem_ide_reg_wr

	.globl  l__INITIALIZER
	.globl  s__INITIALIZED
 	.globl  s__INITIALIZER

	.area	_HEADER (ABS)

	;; Setup the function pointers at the end of the EPROM
	.org	0x7F00
	;;.dw		#_n8vem_ide_init
	;;.dw		#_n8vem_ide_reg_rd
	;;.dw		#_n8vem_ide_reg_wr
	;;.dw		#_n8vem_ide_read
	;;.dw		#_getchar
	;;.dw		#_putchar
	
	.org	0x7FFE ; Add a dummy word here just to make sure the eprom is filled up to 32k
	.dw	0xAAAA

	;; Reset vector
	.org 	0x0000
init:
	;; Stack at the top of memory.
	ld	sp,#0xFFFF

    	;; Initialise global variables
    	call	gsinit
	call	_main
	jp		_exit

	;; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
	.area	_INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL

	.area	_DATA
	.area	_INITIALIZED
	.area	_BSEG
	.area   _BSS
	.area   _HEAP

	.area   _CODE

_exit::
	halt

	.area   _GSINIT
gsinit::
	ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	jr	Z, gsinit_next
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
	ldir
gsinit_next:

	.area   _GSFINAL
	ret

