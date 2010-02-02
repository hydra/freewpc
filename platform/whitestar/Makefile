
# Root directory for the platform files
P=platform/wpc
PW=platform/whitestar

CPU := m6809
$(eval $(call have,CONFIG_PERIODIC_FIRQ))
$(eval $(call have,CONFIG_TASK))
include cpu/$(CPU)/Makefile

# Subplatform support
CONFIG_WHITESTAR ?= y
$(eval $(call have,CONFIG_BARE))
$(eval $(call have,CONFIG_PLATFORM_WHITESTAR))
$(eval $(call have,CONFIG_REMOTE_DISPLAY))
$(eval $(call have,CONFIG_REMOTE_SOUND))
$(eval $(call nohave,CONFIG_AC))
CONFIG_FONT ?= $(CONFIG_DMD)
CONFIG_ANIMATION ?= $(CONFIG_DMD)

# Additional CFLAGS required on the Whitestar platform
CFLAGS += -mdirect -fno-builtin -mcode-section=.text -mdata-section=.text -mbss-section=ram

# Don't check format strings, because we define those differently than ANSI C.
CFLAGS += -Wno-format

# Optional CFLAGS
CFLAGS += -mint8

### NOTE:  -mwpc is totally not acceptable, as Whitestar doesn't have
# the shifter hardware.  But gcc6809 is crashing on some bit shift
# statements, so for _now_, keep this here until the compiler can be
# fixed, so we can fix up other problems.  TODO
CFLAGS += -mwpc

# Optimization flags
CFLAGS += -O2 -fomit-frame-pointer -fstrength-reduce -frerun-loop-opt -Wunknown-pragmas -foptimize-sibling-calls -fstrict-aliasing -fregmove

KERNEL_SW_OBJS += $(P)/main.o
KERNEL_HW_OBJS += $(PW)/init-whitestar.o
KERNEL_ASM_OBJS += $(PW)/start.o

# The startup code needs to know the number of paged regions in the ROM
# to checksum.  The pages between BOTTOM_BANK and TOP_BANK are paged in
# and contribute to the total checksum.  TOP_BANK is fixed but
# BOTTOM_BANK varies according to the machine-specific ROM usage.
$(P)/start.o : EXTRA_ASFLAGS += -DTOP_BANK=61 -DBOTTOM_BANK=58

# TODO : as long as CONFIG_BARE is defined, we need this!
ifeq ($(CONFIG_BARE),y)
KERNEL_OBJS += $(KERNEL_BASIC_OBJS) $(KERNEL_SW_OBJS) $(KERNEL_HW_OBJS)
endif
