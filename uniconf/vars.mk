
libwvstreams.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard uniconf/*.cc))

include $(call doinclude,vars.mk,uniconf)

