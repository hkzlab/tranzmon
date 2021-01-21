# Prefixes
COMPILER_PREFIX = $(SDCC_PREFIX)
COMPILER_LIBS = $(COMPILER_PREFIX)/share/sdcc/lib/z80/

# Options
QUIET = @

# SDCC commands
CCC = $(COMPILER_PREFIX)/bin/sdcc-sdcc
CAS = $(COMPILER_PREFIX)/bin/sdcc-sdasz80
CLD = $(COMPILER_PREFIX)/bin/sdcc-sdldz80

# Misc local commands
ECHO = echo
COPY = cp
MOVE = mv
H2B = objcopy

ROMADDR = 0x0000

# Tool flags
H2B_FLAGS = --input-target=ihex --output-target=binary

# Project directories
SRC_DIR = src/
BIN_DIR = bin/

INCLUDES = -I$(SRC_DIR)/include -I$(SRC_DIR)

CLOC = 0x0100
CSIZ = 0x7C00
DLOC = 0x8000

# Compilation / Assembly / Linking flags
CUST_DEFINES = 
#CUST_DEFINES += -D__SERIAL_CONSOLE__
CCC_FLAGS = --opt-code-size -mz80 -D__SDCC__=1 -D__ROMADDR__=$(ROMADDR) -D__CLOC__=$(CLOC) -D__DLOC__=$(DLOC) $(CUST_DEFINES) $(INCLUDES)
CAS_FLAGS = -plosff
CLD_FLAGS = --code-loc $(CLOC) --data-loc $(DLOC) --code-size $(CSIZ) --no-std-crt0 --out-fmt-ihx

# Here begins the actual creation of destination files
TARGET = monitor

all: $(BIN_DIR)/$(TARGET).bin

$(BIN_DIR)/$(TARGET).bin:	$(BIN_DIR)/$(TARGET).hex
	$(QUIET)$(ECHO) Converting $(TARGET).hex to bin
	$(H2B) $(H2B_FLAGS) $(BIN_DIR)/$(TARGET).hex $(BIN_DIR)/$(TARGET).bin

$(BIN_DIR)/$(TARGET).hex:	$(BIN_DIR)/$(TARGET).ihx
	$(QUIET)$(ECHO) Generating $(TARGET).ihx
	$(QUIET)$(COPY)	$(BIN_DIR)/$(TARGET).ihx $(BIN_DIR)/$(TARGET).hex

$(BIN_DIR)/$(TARGET).ihx:	$(BIN_DIR)/crt0.rel $(BIN_DIR)/main.rel \
							$(BIN_DIR)/pio.rel \
							$(BIN_DIR)/ctc.rel \
							$(BIN_DIR)/dart.rel \
							$(BIN_DIR)/utilities.rel
	$(CCC) $(CLD_FLAGS) $(CCC_FLAGS) $(BIN_DIR)/crt0.rel $(BIN_DIR)/main.rel \
		$(BIN_DIR)/pio.rel \
		$(BIN_DIR)/ctc.rel \
		$(BIN_DIR)/dart.rel \
		$(BIN_DIR)/utilities.rel \
		-o $(BIN_DIR)/$(TARGET).ihx

$(BIN_DIR)/crt0.rel: $(SRC_DIR)/crt0.s
	$(CAS) $(CAS_FLAGS) -c -o $(BIN_DIR)/crt0.rel $(SRC_DIR)/crt0.s

$(BIN_DIR)/main.rel: $(SRC_DIR)/main.c
	$(CCC) $(CCC_FLAGS) -c -o $(BIN_DIR) $(SRC_DIR)/main.c
	
$(BIN_DIR)/utilities.rel: $(SRC_DIR)/utilities.c
	$(CCC) $(CCC_FLAGS) -c -o $(BIN_DIR) $(SRC_DIR)/utilities.c

$(BIN_DIR)/pio.rel: $(SRC_DIR)/hardware/pio.c
	$(CCC) $(CCC_FLAGS) -c -o $(BIN_DIR) $(SRC_DIR)/hardware/pio.c

$(BIN_DIR)/ctc.rel: $(SRC_DIR)/hardware/ctc.c
	$(CCC) $(CCC_FLAGS) -c -o $(BIN_DIR) $(SRC_DIR)/hardware/ctc.c

$(BIN_DIR)/dart.rel: $(SRC_DIR)/hardware/dart.c
	$(CCC) $(CCC_FLAGS) -c -o $(BIN_DIR) $(SRC_DIR)/hardware/dart.c

clean:
	rm $(BIN_DIR)/*
