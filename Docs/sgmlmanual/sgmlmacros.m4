changequote(--<<-,->>--)

define(skip_include, )

define(ZAPPY, )

undefine(--<<-format->>--)

define(WVEXACTLY, --<<-
	--AUTOPUNC--
	--SNIP--
WVCOMMAS($*)
	--SNIP--
	--AUTOPUNC--
->>--)

define(WVEXAMPLE, --<<-
	<informalexample>
	 <programlisting>
	  WVEXACTLY(include($1))
 	 </programlisting>
	</informalexample>
->>--)

define(WVCOMMAS, --<<-ifelse($#, 1, $1, --<<-$1, WVCOMMAS(shift($*))->>--)->>--)

define(WVPREFACE, --<<-<preface><title>$1</title>
WVCOMMAS(shift($*))

</preface>
->>--)

define(WVPART, --<<-<part id="$1"><title>$2</title>
WVCOMMAS(shift(shift($*)))

</part>
->>--)

define(WVPARTINTRO, --<<-<partintro><title>$1</title>
WVCOMMAS(shift($*))

</partintro>
->>--)

define(WVCHAPTER, --<<-<chapter id="$1"><title>$2</title>
WVCOMMAS(shift(shift($*)))

</chapter>
->>--)

define(WVSECT1, --<<-<sect1 id="$1"><title>$2</title>
WVCOMMAS(shift(shift($*)))

</sect1>
->>--)

define(WVSECT2, --<<-<sect2 id="$1"><title>$2</title>
WVCOMMAS(shift(shift($*)))

</sect2>
->>--)

define(WVSECT3, --<<-<sect3 id="$1"><title>$2</title>
WVCOMMAS(shift(shift($*)))

</sect3>
->>--)

define(WVCMD, --<<-
	<synopsis>WVEXACTLY($*)</synopsis>
->>--)
