TOPDIR=../..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../../include

# if you compile SSLeay yourself, change this to the base directory of the
# package (e.g. /usr/src/SSLeay)
SSLDIR=SILLY/ssleay

# List of include directories for SSLeay.  If none of these is yours, add
# it to the list.
SSLINC= /usr/include/ssl /usr/include/ssleay /usr/local/include/ssl \
	/usr/local/ssl/include /usr/local/ssleay/include \
	/usr/include/openssl

# List of library directories for SSLeay.  Add yours to the list.
# Don't forget the -L before each directory name!
SSLLIB= -L/usr/lib/ssl -L/usr/lib/ssleay -L/usr/local/lib/ssleay \
	-L/usr/local/ssl -L/usr/local/ssl/lib -L/usr/local/ssleay

XPATH=$(SSLINC) $(SSLDIR)/include

LIBS += -L../.. -lwvutils -lwvstreams -lssl
# cryptotest-LIBS += -lssl
# ssltest-LIBS += -lssl
# sslsrvtest-LIBS += -lssl
# certtest-LIBS += -lssl

default: all

all: cryptotest ssltest sslsrvtest certtest reqtest md5test

crypto.a: ../crypto.libs

cryptotest ssltest sslsrvtest certtest reqtest md5test: crypto.a

clean:
	rm -f cryptotest ssltest sslsrvtest certtest reqtest md5test testcert.pem

