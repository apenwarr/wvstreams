
libwvstreams.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard linuxstreams/*.cc))

include $(call doinclude,vars.mk,ipstreams)

