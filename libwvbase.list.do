redo-ifchange config.od
. ./config.od

objs=$(cat <<-EOF
	streams/wvconstream.o
	streams/wvfdstream.o
	streams/wvfile.o
	streams/wvistreamlist.o
	streams/wvlog.o
	streams/wvstream.o
	streams/wvstreamclone.o
	uniconf/uniconf.o
	uniconf/uniconfgen.o
	uniconf/uniconfkey.o
	uniconf/uniconfroot.o
	uniconf/unihashtree.o
	uniconf/uniinigen.o
	uniconf/unilistiter.o
	uniconf/unimountgen.o
	uniconf/unitempgen.o
	utils/wvattrs.o
	utils/wvbackslash.o
	utils/wvbuffer.o
	utils/wvbufferstore.o
	utils/wvcont.o
	utils/wvcrashbase.o
	utils/wvencoder.o
	utils/wverror.o
	utils/wvfork.o
	utils/wvhash.o
	utils/wvhashtable.o
	utils/wvlinklist.o
	utils/wvmoniker.o
	utils/wvregex.o
	utils/wvscatterhash.o
	utils/wvsorter.o
	utils/wvstreamsdebugger.o
	utils/wvstring.o
	utils/wvstringcache.o
	utils/wvstringlist.o
	utils/wvstringmask.o
	utils/wvstrutils.o
	utils/wvtask.o
	utils/wvtclstring.o
	utils/wvtimeutils.o
	xplc-cxx/factory.o
	xplc-cxx/getiface.o
	xplc-cxx/strtouuid.o
	xplc-cxx/uuidtostr.o
	xplc-cxx/xplc.o
	xplc/category.o
	xplc/catiter.o
	xplc/catmgr.o
	xplc/loader.o
	xplc/moduleloader.o
	xplc/modulemgr.o
	xplc/monikers.o
	xplc/new.o
	xplc/servmgr.o
	xplc/statichandler.o
EOF
)

{
    echo "$objs"
    [ -z "$_WIN32" ] || dirs=win32 . ./objlist.od
} |
sort |
{
    if [ -n "$_WIN32" ]; then
    	redo-ifchange not-win32.list
        comm -2 -3 - not-win32.list
    else
        cat
    fi
} >$3
