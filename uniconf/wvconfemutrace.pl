#! /usr/bin/perl -w

use strict;

my %entries;
my $binary = shift(@ARGV);

while(my $line = <>) {
  next unless $line =~ m/^TRACE:/;
  chomp($line);
  my(undef, $type, $section, $key, $cookie, @backtrace) = split(/:/, $line);

  if($type eq 'add') {
    $entries{$section} = {} if !defined($entries{$section});
    $entries{$section}{$key} = {} if !defined($entries{$section}{$key});

    $entries{$section}{$key}{$cookie} = \@backtrace;
  } elsif($type eq 'del') {
    $entries{$section}{$key}{$cookie} = undef;
  } else {
    die("unknown type");
  }
}

foreach my $section (sort(keys(%entries))) {
  foreach my $key (sort(keys(%{$entries{$section}}))) {
    foreach my $cookie (sort(keys(%{$entries{$section}{$key}}))) {
      if(defined($entries{$section}{$key}{$cookie})) {
	print "[$section]$key -> $cookie:\n";
	shift(@{$entries{$section}{$key}{$cookie}});
	foreach my $frame (@{$entries{$section}{$key}{$cookie}}) {
	  $frame =~ s/\+0x[0-9a-f]+\)/)/;
	  $frame =~ s/\) \[(.*)\]$/)/;
	  #my $line = `addr2line -e $binary $1`;
	  #chomp($line);

	  print "\t$1: $frame\n", ;
	}
      }
    }
  }
}

