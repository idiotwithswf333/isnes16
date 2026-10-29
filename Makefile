ifeq ($(strip $(DEVKITARM)),)
$(error "DEVKITARM is not set (use the devkitpro/devkitarm image)")
endif
ifeq ($(strip $(DEVKITPRO)),)
$(error "DEVKITPRO is not set")
endif

TARGET   := dsisnes
SOURCES  := $(wildcard source/*.c)
OBJECTS  := $(SOURCES:.c=.o)

LIBNDS   ?= $(DEVKITPRO)/libnds
CALICO   ?= $(DEVKITPRO)/calico

# ARM7 side: stock libnds/calico ARM7 binary (audio will move here later).
# Override on the command line if the auto-detect picks the wrong file.
ARM7ELF  ?= $(firstword $(wildcard $(CALICO)/bin/ds7_maxmod.elf $(CALICO)/bin/ds7.elf $(LIBNDS)/default.elf))
ICON     ?= $(firstword $(wildcard icon.bmp $(LIBNDS)/icon.bmp))

# DSi-mode header flags for ndstool (same ones TWiLight Menu++/nds-bootstrap use).
DSI_FLAGS ?= -z 80040000 -u 00030004

ARCH     := -march=armv5te -mtune=arm946e-s
CC       := arm-none-eabi-gcc
CFLAGS   := -g -Wall -O2 -ffunction-sections -fdata-sections $(ARCH) \
            -DARM9 -D__NDS__ -Isource -I$(LIBNDS)/include -I$(CALICO)/include
LDFLAGS  := -specs=$(CALICO)/share/ds9.specs -g $(ARCH) -Wl,-Map,$(TARGET).map
LIBS     := -L$(LIBNDS)/lib -L$(CALICO)/lib -lfat -lnds9

all: $(TARGET).nds

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

$(TARGET).nds: $(TARGET).elf
	ndstool -c $@ -9 $< -7 $(ARM7ELF) -b $(ICON) "DSi SNES;SNES emulator;DSi only" \
	        -g SNSD 01 "DSISNES" $(DSI_FLAGS)

clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).nds $(TARGET).map

.PHONY: all clean
