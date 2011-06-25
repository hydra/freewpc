# This Makefile is used to control a native build, where the code
# runs on the same machine as the build.  It is slightly different
# because of toolchain differences, and also simulation files need
# to be linked in.

DDD := unittests

UNITTEST_OBJS := $(DDD)/seatest.o $(DDD)/testrunner.o
#UNITTEST_OBJS += 

# Additional defines
CFLAGS += -I$(DDD)

$(NATIVE_OBJS) : CFLAGS += -DCONFIG_UNITTEST
$(UNITTEST_OBJS) : CFLAGS += -DNATIVE_SYSTEM


