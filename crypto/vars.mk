
libwvstreams.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard crypto/*.cc))
libwvstreams.so-OBJECTS += $(patsubst %.c,%.o,$(wildcard crypto/*.c))

CPPFLAGS += -I/usr/include/openssl

include $(call doinclude,vars.mk,crypto)

