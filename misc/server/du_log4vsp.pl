#!/usr/bin/perl
#
#  A Smokin'Guns Duel Log Fix for VSP stat.
#  Copyright (c) 2007-2009 Gilbert Cao (hika)
#  All rights reserved.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
################################################################################
#
# v1.0
# The program is basically a fix in the log file provided by Smokin'Guns Duel.
# - It remove "Warmup:" lines.
# - It add "ClientBegin:" lines when a player switch from spectator to ingame.
#

use strict;

sub usage {
	print STDOUT "usage : " . $0 . " -in <file> [-out <file>]\n";
}

my($in, $out, $i, $argc);
$argc = scalar(@ARGV);
################################################################################
# Parse arguments
for($i = 0; $i < $argc; $i++) {
	if (($ARGV[$i] eq "-in")
	&& ($i < $argc-1)
	&& (-f $ARGV[$i+1])) {
		$in = $ARGV[$i+1];
		$i++;
	}
	elsif (($ARGV[$i] eq "-out")
	&& ($i < $argc-1)) {
		$out = $ARGV[$i+1];
		$i++;
	}
}
################################################################################

if (!defined($in)) {
	usage();
	exit(0);
}

if (!open(IN, "<" . $in)) {
	print STDERR $in . ": " . $! . "\n";
	exit(-1);
}
if (defined($out)) {
	if (!open(OUT, ">" . $out)) {
		print STDERR $out . ": " . $! . "\n";
		exit(-1);
	}
}
else {
	if (!open(OUT, ">&=STDOUT")) {
		print STDERR "STDOUT: " . $! . "\n";
		exit(-1);
	}
}

my($valueInfo, $line, $userinfo, $clientNum, $spec, $digit1, $digit2);
$userinfo = 0; $spec = 1;
while ($line = <IN>) {
	chomp($line);

	if ($userinfo && !$spec) {
	# The previous line was a "ClientUserinfoChanged"
	# and the player was not a spectator.
		if ($line !~ /^\ *\d+:\d+\ *ClientBegin/) {
		# No ClientBegin ? So we have to add it manually
			my($whitespace, $l);
			$l = length($digit1);
			if ($l == 1) {
				$whitespace = "  ";
			}
			elsif ($l == 2) {
				$whitespace = " ";
			}
			else {
				$whitespace = "";
			}

			# Add an "fake" "ClientBegin line
			print OUT $whitespace . $digit1 . ":" . $digit2 . " ClientBegin: " . $clientNum . "\n";
		}
		$userinfo = 0;
		$spec = 1;
	}

	if ($line !~ /^\ *(\d+):(\d+)\ *Warmup:/) {
		print OUT $line . "\n";
	}

	if ($line =~ /^\ *(\d+):(\d+)\ *ClientUserinfoChanged:\ (\d+)\ (.+)$/) {
	# A line like 
	#  3:10 ClientUserinfoChanged: 0 n\Tuco\t\3\model\wq_male3/blue\hc\70\w\0\l\0\skill\2.000000\tt\0\tl\0  3:10 ClientUserinfoChanged: 0 n\Tuco\t\3\model\wq_male3/blue\hc\70\w\0\l\0\skill\2.000000\tt\0\tl\0
	# match the pattern.
	# Get the the clientNum and the spectator status.
	# If != 3, then the player is not a spectator.
		$digit1 = $1;
		$digit2 = $2;
		$clientNum = $3;
		$valueInfo = $4;
		$userinfo = 1;

		my(@vars) = split(/\\+/, $valueInfo);
		if (scalar(@vars) >= 4) {
			if ($vars[3] ne "3") {
			# Not a spectator
				$spec = 0;
			}
		}
	}
}

close(IN);
close(OUT);

