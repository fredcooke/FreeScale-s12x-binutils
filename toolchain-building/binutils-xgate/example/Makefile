#!/usr/bin/make -f

########################################
# Makefile for 68HC11 projects         #
# GNU copyright 2002 by Aurelien Jarno #
########################################

#
# Project files
#
PROG = test
SRCS = main.s
OBJS = $(SRCS:.s=.o)

# Assembler flags used by default to compile a program
ASFLAGS = --gstabs

LDFLAGS = -m m68hc11elfb --defsym _start=0

#
# Options to creates the .s19 or .b files from the elf
#
OBJCOPY_FLAGS = --remove-section=page0 \
                --remove-section=.page0


####################################
# This part should not be modified #
####################################
#
# Programs location
#
DEVC_PREFIX=m68hc11-
AS=$(DEVC_PREFIX)as
SIZE=$(DEVC_PREFIX)size
OBJCOPY=$(DEVC_PREFIX)objcopy
OBJDUMP=$(DEVC_PREFIX)objdump
LD=$(DEVC_PREFIX)ld

#
# Main rules
#
all:	$(PROG).elf $(PROG).s19 $(PROG).b

$(PROG).elf: $(OBJS) memory.x
	$(LD) $(LDFLAGS) -o $(PROG).elf $(OBJS)

clean:
	rm -f *.o *.elf *.s19 *.b

#
# Some useful rules
#
dump:   $(PROG).elf
	$(OBJDUMP) -d $(PROG).elf

size:   $(PROG).s19
	$(SIZE) $(PROG).s19

#
# Implicit rules
#
# .elf is for the simulator and gdb
# .s19 is for some downloader and the simulator
# .b   is a binary dump
#
.SUFFIXES: .elf .s19 .b

.elf.s19:
	$(OBJCOPY) --output-target=srec $(OBJCOPY_FLAGS) $< $*.s19

.elf.b:
	$(OBJCOPY) --output-target=binary --gap-fill=255 $(OBJCOPY_FLAGS) $< $*.b
