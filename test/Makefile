
TEST_OBJS += test/window.o

TEST2_OBJS += test/format.o
TEST2_OBJS += test/preset.o
ifeq ($(CONFIG_DMD),y)
ifeq ($(CONFIG_PLATFORM_WPC),y)
TEST2_OBJS += test/swtest.o
endif
endif
TEST2_OBJS += $(if $(CONFIG_DMD),test/dmdtest.o)
TEST2_OBJS += test/adjust.o
TEST2_OBJS += test/timestamp.o
TEST2_OBJS += test/stress.o
TEST2_OBJS += $(if $(CONFIG_WPC), test/wire.o)
TEST2_OBJS += test/pricing.o
TEST2_OBJS += test/burnin.o
