
libwvstreams.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard crypto/*.cc))
libwvstreams.so-OBJECTS += $(patsubst %.c,%.o,$(wildcard crypto/*.c))

include $(call doinclude,vars.mk,crypto)

