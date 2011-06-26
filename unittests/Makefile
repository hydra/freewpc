
DDD := unittests
$(UNITTEST_OBJS) : CFLAGS += -DNATIVE_SYSTEM

# the test runner and library
UNITTEST_OBJS := $(DDD)/seatest.o $(DDD)/testrunner.o 

# the unit tests
UNITTEST_OBJS += $(DDD)/switches_tests.o $(DDD)/combos_tests.o

# Additional defines
CFLAGS += -I$(DDD)

# force sim/main.c's main() method to be ignored.
$(NATIVE_OBJS) : CFLAGS += -DCONFIG_UNITTEST



