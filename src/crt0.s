;; Custom CRT0 for Tranz 330 Monitor
;; Taken from SDCC

	.module crt0
	
	.globl	_main

	;; Console I/O functions to export
	;;.globl  _putchar
	;;.globl  _getchar

	;; IDE functions to export
	;;.globl	_n8vem_ide_init
	;;.globl	_n8vem_ide_read
	;;.globl 	_n8vem_ide_reg_rd
	;;.globl	_n8vem_ide_reg_wr

    ;; ISR to export
    .globl _pio_isr

    ;; Data to export
    .globl _str_appname

	.area	_HEADER (ABS)

    ;; Strings and other static data
    .org    0x7D00
_str_appname:
    .asciz  "TRANZMON V0.1"

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
	jp  init
	
	;; ISR
	.org    0x0018
	.dw     #_pio_isr
	
	.org    0x0040
init:
	;; Stack at the top of memory.
	ld	sp,#0xFFFF

    ;; Setup the interrupts
    im 2  ;; Interrupt mode 2
    xor a ;; Interrupt vectors in page 0
    ld i,a
    
    di ;; Disable the interrupts for now

    ;; Clear the RAM
    xor a           ;; Clear register 'a'
    ld bc,#0x8000   ;; Times that the ldir command will repeat
    ld hl,#0x8000   ;; Point 'hl' to the start of RAM
    ld de,#0x8001   ;; Put location of RAM+1 here
    ld (hl),a       ;; Clear the location
    ldir            ;; Repeat

    ;; Initialise global variables
    call	gsinit
	call	_main
	jp		_exit

gsinit::
    ret
    
_exit::
    halt

	;; Ordering of segments for the linker.
	.area	_HOME
    .area   _CODE
	.area	_DATA
	.area   _BSEG
	.area   _BSS
	.area   _HEAP


