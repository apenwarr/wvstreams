
libwvutils.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard utils/*.cc))

include $(call doinclude,vars.mk,utils)

