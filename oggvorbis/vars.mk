
libwvoggvorbis.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard oggvorbis/*.cc))

include $(call doinclude,vars.mk,oggvorbis)

