
libwvstreams.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard ipstreams/*.cc))

include $(call doinclude,vars.mk,ipstreams)

