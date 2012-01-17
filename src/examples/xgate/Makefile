#!/usr/bin/make -f


#
# Project files
#
PROG = test
SRCS = xgate.s
OBJS = $(SRCS:.s=.o)

# Assembler flags used by default to compile a program
ASFLAGS = --gstabs

#LDFLAGS = -m mc9xgateelfb --defsym _start=0
LINKOPTS = -Wl,-defsym,vectors_addr=0xF710,-m,mc9xgateelfb,-T,hc9s12xdp512elfb.x
#
# Options to creates the .s19 or .b files from the elf
#
OBJCOPY_FLAGS_TEMP = --remove-section=page0 \
#                --remove-section=.page0
# If using the below option for paged flash, the ADDRESS_TYPE
# field in the freeEMSloader script should be updated to match.

# TODO figure out why it wont recognize any other sections besides .data and .text

#	--change-section-lma .text+0xFEC000 \
#       --change-section-lma .data+0xFEC000 \



OBJCOPY_FLAGS  = --output-target=srec \
		--change-section-lma .fixedconf1+0xFD4000 \
		--change-section-lma .fixedconf2+0xFD4000 \
		     --change-section-lma .text1+0xFD4000 \
		      --change-section-lma .text=0xF70800 \
		      --change-section-lma .data=0xF70800 \
		   --change-section-lma .vectors+0xFEC000 \
		    --change-section-lma .rodata+0xFEC000 \
		--only-section=.fixedconf1 \
		--only-section=.fixedconf2 \
		--only-section=.text \
		--only-section=.text1 \
		--only-section=.data \
		--only-section=.vectors \
		--only-section=.rodata \
		--change-section-lma .xgateF8=0xF80000 \
		--change-section-lma .ppageF8=0xF88000 \
		--change-section-lma .fpageF9=0xF98000 \
		--change-section-lma .dpageF9=0xF9A000 \
		--change-section-lma .ppageFE=0xFE8000 \
		--only-section=.ppageF8 \
		--only-section=.fpageF9 \
		--only-section=.dpageF9 \
		--only-section=.ppageFE \
		--change-section-lma .fpageFA=0xFA8000 \
		--change-section-lma .dpageFA=0xFAA000 \
		--change-section-lma .fpageFB=0xFB8000 \
		--change-section-lma .dpageFB1=0xFBA000 \
		--change-section-lma .dpageFB2=0xFBA400 \
		--change-section-lma .dpageFB3=0xFBA800 \
		--change-section-lma .dpageFB4=0xFBAC00 \
		--change-section-lma .dpageFB5=0xFBB000 \
		--change-section-lma .dpageFB6=0xFBB400 \
		--change-section-lma .dpageFB7=0xFBB800 \
		--change-section-lma .dpageFB8=0xFBBC00 \
		--change-section-lma .fpageFC=0xFC8000 \
		--change-section-lma .dpageFC=0xFCA000 \
		--only-section=.dpageFA \
		--only-section=.fpageFA \
		--only-section=.dpageFB1 \
		--only-section=.dpageFB2 \
		--only-section=.dpageFB3 \
		--only-section=.dpageFB4 \
		--only-section=.dpageFB5 \
		--only-section=.dpageFB6 \
		--only-section=.dpageFB7 \
		--only-section=.dpageFB8 \
		--only-section=.fpageFB \
		--only-section=.dpageFC \
		--only-section=.fpageFC
#		--change-section-lma .ppageFD=0xFD8000 \
#		--change-section-lma .ppageFF=0xFF8000 \
#		--only-section=.ppageFD \	This is left here for clarity only (equivalent to text1 but paged)
#		--only-section=.ppageFF		This is left here for clarity only (equivalent to text but paged)
# Changes to the above options should be reflected in memory.x and memory.h also!

# The following flash areas can only be used once Sean has written his loader app :
#		--only-section=.ppageE0 \
#		--only-section=.ppageE1 \
#		--only-section=.ppageE2 \
#		--only-section=.ppageE3 \
#		--only-section=.ppageE4 \
#		--only-section=.ppageE5 \
#		--only-section=.ppageE6 \
#		--only-section=.ppageE7 \
#		--only-section=.ppageE8 \
#		--only-section=.ppageE9 \
#		--only-section=.ppageEA \
#		--only-section=.ppageEB \
#		--only-section=.ppageEC \
#		--only-section=.ppageED \
#		--only-section=.ppageEE \
#		--only-section=.ppageEF \
#		--only-section=.ppageF0 \
#		--only-section=.ppageF1 \
#		--only-section=.ppageF2 \
#		--only-section=.ppageF3 \
#		--only-section=.ppageF4 \
#		--only-section=.ppageF5 \
#		--only-section=.ppageF6 \
#		--only-section=.ppageF7 \
#		--change-section-lma .ppageE0=0xE08000 \
#		--change-section-lma .ppageE1=0xE18000 \
#		--change-section-lma .ppageE2=0xE28000 \
#		--change-section-lma .ppageE3=0xE38000 \
#		--change-section-lma .ppageE4=0xE48000 \
#		--change-section-lma .ppageE5=0xE58000 \
#		--change-section-lma .ppageE6=0xE68000 \
#		--change-section-lma .ppageE7=0xE78000 \
#		--change-section-lma .ppageE8=0xE88000 \
#		--change-section-lma .ppageE9=0xE98000 \
#		--change-section-lma .ppageEA=0xEA8000 \
#		--change-section-lma .ppageEB=0xEB8000 \
#		--change-section-lma .ppageEC=0xEC8000 \
#		--change-section-lma .ppageED=0xED8000 \
#		--change-section-lma .ppageEE=0xEE8000 \
#		--change-section-lma .ppageEF=0xEF8000 \
#		--change-section-lma .ppageF0=0xF08000 \
#		--change-section-lma .ppageF1=0xF18000 \
#		--change-section-lma .ppageF2=0xF28000 \
#		--change-section-lma .ppageF3=0xF38000 \
#		--change-section-lma .ppageF4=0xF48000 \
#		--change-section-lma .ppageF5=0xF58000 \
#		--change-section-lma .ppageF6=0xF68000 \
#		--change-section-lma .ppageF7=0xF78000 \


####################################
# This part should not be modified #
####################################
#
# Programs location
#
DEVC_PREFIX=mc9xgate-elf-
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
