
libwvfft.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard fft/*.cc))

include $(call doinclude,vars.mk,fft)

