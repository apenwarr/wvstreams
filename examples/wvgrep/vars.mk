TARGETS+=examples/wvgrep/wvgrep examples/wvgrep/wvegrep

examples/wvgrep/wvgrep: examples/wvgrep/wvgrep.o $(LIBWVSTREAMS) $(LIBWVUTILS)

examples/wvgrep/wvegrep: examples/wvgrep/wvgrep
	ln -f $< $@

