#---------------------------------------------------------------------------------
# Based on devkitPro/nds-examples templates/arm9/Makefile (libnds 2 / calico).
# ds_rules supplies the ARM7 binary, the ndstool rule and the default icon.
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

GAME_TITLE     := DSi SNES
GAME_SUBTITLE1 := SNES emulator
GAME_SUBTITLE2 := DSi only

include $(DEVKITARM)/ds_rules

TARGET   := dsisnes
BUILD    := build
SOURCES  := source
INCLUDES := include

ARCH     := -march=armv5te -mtune=arm946e-s
CFLAGS   := -g -Wall -O2 -ffunction-sections -fdata-sections \
            $(ARCH) $(INCLUDE) -DARM9
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS  := -g $(ARCH)
LDFLAGS   = -specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

# order is important
LIBS     := -lfat -lnds9

LIBDIRS  := $(LIBNDS) $(PORTLIBS)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT  := $(CURDIR)/$(TARGET)
export VPATH   := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))

export LD      := $(CC)
export OFILES  := $(CFILES:.c=.o)

export INCLUDE := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifneq (,$(wildcard icon.bmp))
export GAME_ICON := $(CURDIR)/icon.bmp
endif

.PHONY: $(BUILD) clean

$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds

else

$(OUTPUT).nds: $(OUTPUT).elf $(GAME_ICON)
$(OUTPUT).elf: $(OFILES)

-include $(DEPSDIR)/*.d

endif
