
libwvstreams.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard configfile/*.cc))

include $(call doinclude,vars.mk,configfile)

