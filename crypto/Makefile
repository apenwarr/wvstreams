TOPDIR=../..
include ${TOPDIR}/rules.mk

# if you compile SSLeay yourself, change this to the base directory of the
# package (e.g. /usr/src/SSLeay)
SSLDIR=SILLY/ssleay

# List of include directories for SSLeay.  If none of these is yours, add
# it to the list.
SSLINC= /usr/include/ssl /usr/include/ssleay /usr/local/include/ssl \
	/usr/local/ssl/include /usr/local/ssleay/include /usr/include/openssl

# List of library directories for SSLeay.  Add yours to the list.
# Don't forget the -L before each directory name!
SSLLIB= -L/usr/lib/ssl -L/usr/lib/ssleay -L/usr/local/lib/ssleay \
	-L/usr/local/ssl -L/usr/local/ssl/lib -L/usr/local/ssleay

default: all cryptotest ssltest
all: crypto.libs

LIBS += -L${SSLDIR} ${SSLLIB} -lssl -lcrypto
LIBS += ${EFENCE}
#LIBS += /usr/local/mpr/libmpr.a -static
XPATH=.. ../utils ../streams ../configfile ../ipstreams ../netmap ../netguide \
	${SSLDIR}/include ${SSLINC}

crypto.libs: wvcrypto.o wvsslstream.o

crypto.a: crypto.libs

cryptotest ssltest: crypto.a \
	 ../netguide/netguide.a ../netmap/netmap.a ../libwvstreams.a

clean:
	rm -f cryptotest ssltest
