
libwvstatic.a: $(call objects,win32)

# object files that we replace completely for win32
OBJREPLACED=\
	utils/wvtask.o \
	
# object files that we can't use in win32 for now, but which we should
# definitely fix
OBJFIXME=\
	utils/wvsubprocqueue.o \
	utils/wvsystem.o \
	utils/wvregex.o \
	utils/wvglob.o \
	utils/wvglobdiriter.o \
	utils/wvsubproc.o \
	\
	streams/wvatomicfile.o \
	streams/wvlogrotator.o \
	streams/wvpipe.o \
	streams/wvwatcher.o \
	\
	ipstreams/wvunixsocket.o \
	\
	uniconf/unifilesystemgen.o \
	
# object files that we probably just shouldn't include in win32 libraries
OBJSKIP=$(OBJREPLACED) $(OBJFIXME) \
	utils/strcrypt.o \
	utils/wvfork.o \
	utils/wvmagiccircle.o \
	utils/wvshmzone.o \
	utils/wvprociter.o \
	\
	streams/wvlockdev.o \
	streams/wvlockfile.o \
	streams/wvmagicloopback.o \
	streams/wvmodem.o \
	streams/wvsyslog.o \
	streams/wvsubprocqueuestream.o \
	\
	ipstreams/wvipraw.o \
	ipstreams/wvunixdgsocket.o \
	\
	uniconf/unigenhack.o \
	uniconf/daemon/uniconfd.o \
	
TOBJFIXME=\
	utils/t/wvsubprocqueue.t.o \
	utils/t/wvsystem.t.o \
	utils/t/wvpushdir.t.o \
	\
	streams/t/wvatomicfile.t.o \
	streams/t/wvstreamsdaemon.t.o \
	streams/t/wvpipe.t.o \
	\
	uniconf/t/uniconfd.t.o \
	uniconf/t/uniconfgen-sanitytest.o \
	uniconf/t/unicachegen.t.o \
	uniconf/t/uniinigen.t.o \
	uniconf/t/unireplicategen.t.o \
	uniconf/t/uniretrygen.t.o \
	uniconf/t/uniclientgen.t.o \
	uniconf/t/uniunwrapgen.t.o \
	uniconf/t/unitransactiongen.t.o \
	uniconf/t/unicallbackgen.t.o \
	uniconf/t/unimountgen.t.o \
	uniconf/t/unisubtreegen.t.o \
	uniconf/t/unipermgen.t.o \
	uniconf/t/unidefgen.t.o \
	uniconf/t/unifastregetgen.t.o \
	uniconf/t/unitempgen.t.o \
	uniconf/t/unireadonlygen.t.o \
	uniconf/t/uniautogen.t.o \
	uniconf/t/unilistgen.t.o \
	
TOBJSKIP=$(TOBJFIXME) \
	utils/t/strcrypt.t.o \
	utils/t/wvondiskhash.t.o \
	utils/t/wvregex.t.o \
	utils/t/wvglob.t.o \
	utils/t/wvglobdiriter.t.o \
	utils/t/wvprociter.t.o \
	\
	streams/t/wvmagicloopback.t.o \
	streams/t/wvlogrotator.t.o \
	streams/t/wvsubprocqueuestream.t.o \
	streams/t/wvlockfile.t.o \
	\
	ipstreams/t/wvunixdgsocket.t.o \
	ipstreams/t/wvunixsocket.t.o \
	\
	linuxstreams/t/wvpty.t.o \
	\
	uniconf/t/unitempgenvsdaemon.t.o \
	
PROGSKIP=\
	ipstreams/tests/unixtest \
	examples/wvgrep/wvgrep \
	examples/wvgrep/wvegrep \
	utils/tests/buffertest \
	utils/tests/crashtest \
	utils/tests/crashtest-nofd \
	utils/tests/forktest \
	utils/tests/magiccircletest \
	utils/tests/proctest \
	utils/tests/rateadjtest \
	utils/tests/serializetest \
	utils/tests/tasktest \
	utils/tests/testtest \
	streams/tests/logfiletest \
	streams/tests/looptest \
	streams/tests/modemtest \
	streams/tests/pamtest \
	streams/tests/pipetest \
	streams/tests/syslogtest \
	ipstreams/tests/iptest \
	ipstreams/tests/ip2test \
	ipstreams/tests/udglistentest \
	ipstreams/tests/ulistentest \
	ipstreams/tests/unixdgtest \
	ipstreams/tests/xplctest \
	crypto/tests/cryptotest \
	linuxstreams/tests/aliastest \
	linuxstreams/tests/dspechotest \
	linuxstreams/tests/dsptest \
	linuxstreams/tests/ifctest \
	linuxstreams/tests/routetest \
	uniconf/tests/unimem \
	
WV_EXCLUDES += $(OBJSKIP) $(TOBJSKIP) $(PROGSKIP)
