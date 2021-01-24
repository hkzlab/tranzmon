;; Custom CRT0 for Tranz 330 Monitor
;; Taken from SDCC

	.module crt0
	
	.globl	_main

	;; Console I/O functions to export
	.globl  _putchar
	.globl  _getchar

    ;; ISR to export
    .globl _pio_isr
    .globl _dart_isr
    .globl _ctc_isr_0
    .globl _ctc_isr_1
    .globl _ctc_isr_2
    .globl _ctc_isr_3
            
    ;; Data to export
    .globl _str_appname

	.globl  l__INITIALIZER
	.globl  s__INITIALIZED
 	.globl  s__INITIALIZER

	.area	_HEADER (ABS)

    ;; Strings and other static data
    .org    0x7D00
_str_appname:
    .asciz  "TranzMON v0.4"

	;; Setup the function pointers at the end of the EPROM
	.org	0x7F00
	.dw		#_getchar
	.dw		#_putchar
	
	.org	0x7FFE ; Add a dummy word here just to make sure the eprom is filled up to 32k
	.dw	0xAAAA

	;; Reset vector
	.org 	0x0000
	jp  init
	
	;; Interrupt vector table
	.org    0x0010
inttbl:
	.dw     #_pio_isr   ;; 0x10
	.dw     #_dart_isr  ;; 0x12
	.dw     0           ;; 0x14
	.dw     0           ;; 0x16
	.dw     #_ctc_isr_0 ;; 0x18
	.dw     #_ctc_isr_1 ;; 0x1A
	.dw     #_ctc_isr_2 ;; 0x1C
	.dw     #_ctc_isr_3 ;; 0x1E
	
	
	.org    0x0040
init:
	;; Stack at the top of memory.
	;;ld	sp,#0xFFFF
	ld sp,#0x87FF ;; Reserve 2kb of ram for the monitor

    ;; Setup the interrupts: http://www.z80.info/1653.htm
    ;; In mode 2, when interrupting, a device will automatically place a vector address (8 bits) on the data bus
    ;; this will be combined with the value in register 'i' to build a vector, where 'i' is the higher part and the data on bus the lower
    ;;
    ;; This vector will point to a location in memory where another 16-bit address will be found, this address is the start of the interrupt handler routine
    im 2  ;; Interrupt mode 2
    xor a ;; Interrupt vectors page 0
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

gsinit:
	ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	jr	Z, gsinit_next
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
	ldir
gsinit_next:
    ret
    
_exit::
    halt

	;; Ordering of segments for the linker.
	.area	_HOME
    .area   _CODE
    .area   _INITIALIZER
	.area	_DATA
    .area   _INITIALIZED
	.area   _BSEG
	.area   _BSS
	.area   _HEAP



