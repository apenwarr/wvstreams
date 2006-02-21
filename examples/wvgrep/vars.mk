TARGETS+=examples/wvgrep/wvgrep examples/wvgrep/wvegrep

examples/wvgrep/wvgrep: $(LIBWVSTREAMS) $(LIBWVUTILS)

examples/wvgrep/wvegrep: examples/wvgrep/wvgrep
	ln -f $< $@

