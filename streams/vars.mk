
libwvstreams.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard streams/*.cc))

include $(call doinclude,vars.mk,streams)

