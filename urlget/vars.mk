
libwvstreams.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard urlget/*.cc))

include $(call doinclude,vars.mk,urlget)

