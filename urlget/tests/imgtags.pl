#!/usr/bin/perl -w
# Parse an html file and grab all the urls requested by IMG tags.
#
$urlbase=$ARGV[0];

@all = <STDIN>;
$_ = join("", @all);

@l = /<img\s+[^>]*src="([^"]*)"[^>]*>/gis;

foreach $u (@l)
{
    if ($u =~ m{^https?://}) {
	print "$u\n";
    } else {
	print "$urlbase$u\n";
    }
}

