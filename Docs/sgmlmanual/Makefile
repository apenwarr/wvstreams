
default: wvstreams.verify

all: wvstreams.verify wvstreams.html wvstreams.ps wvstreams.pdf

html: wvstreams.html
ps: wvstreams.ps
pdf: wvstreams.pdf

IMGFILES=$(shell [ -d img ] && find img -type f)

%.verify: %.sgml
	# Syntax check
	nsgmls $< >/dev/null
	# Syntax check passed

%.sgml: %.wvsgml *.m4
	@echo Preprocessing $< to $@...
	@m4 sgmlmacros.m4 $< | awk '			\
		BEGIN {					\
			paropen = 0;			\
			parready = 0;			\
		}					\
							\
		parready && !paropen && /^[ 	]*[-A-Za-z0-9]/ {	\
			print "<para>";			\
			print;				\
			parready = 0;			\
			paropen = 1;			\
			next;				\
		}					\
							\
		(paropen) && (!parready) && /^[ 	]*$$/ {	\
			print "</para>";		\
			print;				\
			parready = 1;			\
			paropen = 0;			\
			next;				\
		}					\
							\
		/^[ 	]*$$/ {				\
			parready = 1;			\
		}					\
							\
		{ print; }				\
		' | cat -s >$@

%.html: %.sgml HTML.dsl ${IMGFILES}
	${MAKE} dirimages
	rm -rf $@ $*.html.tmp
	mkdir -p $*.html.tmp
	# Copy only the required .gif files
	set -e; \
	for d in $$(grep '"[^"]*\.gif"' $*.sgml \
			| sed 's,^.*"img/\([^"]*\.gif\)".*$$,\1,'); \
	do \
		mkdir -p "$*.html.tmp/img/$$(dirname $$d)"; \
		${MAKE} img.tmp/$$d; \
		cp -v img.tmp/$$d $*.html.tmp/img/$$d; \
	done
	# Generate html
	cd $*.html.tmp && jade -t sgml -d ../HTML.dsl ../$*.sgml
	mv $*.html.tmp $@

%.tex: %.sgml
	rm -f $@
	jade -t tex -o $*.tex.tmp \
		-d /usr/lib/dsssl/stylesheets/docbook/print/docbook.dsl $<
	cat $*.tex.tmp | sed 's,{img/\(.*\)\.[^.]*},{img.tmp/\1.eps},g' >$@
	rm -f $*.tex.tmp

%.dvi: jadetex.fmt %.tex ${IMGFILES}
	${MAKE} teximages
	jadetex $*.tex </dev/null >/dev/null # generate .aux files for xrefs
	jadetex $*.tex </dev/null >/dev/null # actually generate output
	jadetex $*.tex </dev/null >/dev/null # actually generate output

%.ps: %.dvi
	dvips -q -o $@ $< </dev/null

%.pdf: %.ps
	ps2pdf $<

img.tmp:
	rm -rf img.tmp
	mkdir img.tmp

img.tmp/%: img/% img.tmp
	cp $< $@
	
img.tmp/%.eps: img.tmp/%.ps
	cp $< $@

img.tmp/%.pnm: img.tmp/%.gif
	giftopnm $< >$@

img.tmp/%.eps img.tmp/%.ps: img.tmp/%.pnm
	pnmtops -dpi 100 -width 350 $< >$@

img.tmp/%.gif: img.tmp/%.ppm
	ppmtogif -interlace $< >$@

img.tmp/%.ppm: img.tmp/%.ps
	gs -q -dNOPAUSE -sDEVICE=ppm -r75x75 -sOutputFile=$@ $< -c quit
	
images: htmlimages teximages

dirimages:
	mkdir -p . $$([ -d img ] && find img -type d | sed 's,^img/,img.tmp/,')

# create .eps from all image types
teximages: dirimages
	${MAKE} null $$([ -d img ] && find img -type f \
			| sed 's,img/\(.*\)\.[^.]*$$,img.tmp/\1.eps,')

# create .gif from all image types			
htmlimages: dirimages
	${MAKE} null $$([ -d img ] && find img -type f \
			| sed 's,img/\(.*\)\.[^.]*$$,img.tmp/\1.gif,')

null:

jadetex.fmt:
	tex -ini -progname=jadetex \&latex jadetex.ini </dev/null >/dev/null


clean:
	rm -f *~ *.fmt *.tex *.ps *.pdf *.dvi *.aux *.log HTML.manifest
	rm -rf *.html *.tmp
	rm -f $$(/bin/ls *.wvsgml | sed 's/\.wvsgml/.sgml/')
