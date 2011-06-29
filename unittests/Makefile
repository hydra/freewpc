
DDD := unittests
# the test runner and library
UNITTEST_OBJS := $(DDD)/seatest.o $(DDD)/testrunner.o 

# the unit tests
UNITTEST_OBJS += $(DDD)/switches_tests.o $(DDD)/combos_tests.o

# Additional defines
ifeq ($(CONFIG_UNITTEST),y)
# among other things this will force sim/main.c's main() method to be ignored and use the one in unittest/testrunner.c instead
CFLAGS += -I$(DDD) -DCONFIG_UNITTEST
endif
