# This Makefile is used to control a P-ROC build, where FreeWPC runs
# on a PC and connects to an external FPGA-based board for I/O.

N := platform/native
P := platform/proc

CONFIG_PROC := y
CONFIG_REMOTEIO := y
CONFIG_DMD := y
CONFIG_PTH := y
CONFIG_ANIMATION ?= $(CONFIG_DMD)
CONFIG_FONT ?= $(CONFIG_DMD)

# Common native mode CPU configuration
CPU := native
include cpu/$(CPU)/Makefile

# Platform dependent files
KERNEL_HW_OBJS += $(P)/init-proc.o
KERNEL_HW_OBJS += $(P)/serial.o
KERNEL_HW_OBJS += $(P)/procdmd.o

# Additional defines
CFLAGS += -I$(P) -DCONFIG_PLATFORM_PROC -DCONFIG_REMOTEIO

# Import images that are common to all games.
IMAGE_MAP += $(P)/../wpc/wpc.ild

FIRST_BANK=0

