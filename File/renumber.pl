#!/usr/bin/perl
#
# Martin Titz, 2023
#
# Rename files with numbers in filenames, so that all numbers
# in the filenames have same length including leading zeros.
#

use strict;
use warnings;

$0 =~ s#.*/##g;
my $usage = "Usage: $0 [-n] [-nr] [-r] [-v] [dirpart] file_begin file_end digits\n";
my $justPrint = 0;
my $recursive = 1;
my $verbose = 0;

my $dirs = 0;
my $files = 0;
my $matches = 0;
my $changes = 0;

my $file_begin;
my $file_end;
my $format;

sub processFilename {
    my ($dirpart, $filepart) = @_;
    my $old = "$dirpart/$filepart";
    warn("$0: File $old doesn't exist\n"), return if !-e $old;
    $files++;
    $_ = $filepart;

    /^($file_begin)(\d+)($file_end)$/ or return;
    $_ = $1.sprintf($format, $2).$3;
    $matches++;

    if ($filepart ne $_) {
        $_ = "$dirpart/$_";
        warn("$0: File $_ already exists, can't rename $old\n"), return if -e;
        unless ($justPrint) {
            unless (rename($old, $_)) { warn "$0: Can't rename $old to $_: $!\n"; return; }
        }
        print "  $old -> $_\n" if $verbose;
        $changes++;
        return;
    }
}

sub processDirectory {
    my $dir = shift(@_);
    my $dh;
    unless (opendir($dh, $dir)) { warn "$0: Cannot open directory $dir: $!"; return; }
    print "Processing directory $dir\n" if $verbose;
    $dirs++;
    while (defined(my $file = readdir $dh)) {
        next if $file eq "." || $file eq "..";
        my $filename = "$dir/$file";
        stat($filename);
        if (-f _) {
            processFilename($dir, $file);
        } elsif ($recursive && -d _) {
            processDirectory($filename);
        }
    }
    closedir $dh;
}

while (defined $ARGV[0] && $ARGV[0] =~ /^-/) {
    $_ = shift;
    if (/^-h$/) {
        die $usage;
    } elsif (/^-n$/) {
        $justPrint = 1;
    } elsif (/^-nr$/) {
        $recursive = 0;
    } elsif (/^-r$/) {
        $recursive = 1;
    } elsif (/^-v$/) {
        $verbose = 1;
    } else {
        die "Unrecognized switch: $_\n";
    }
}

die $usage if $#ARGV < 2 || $#ARGV > 3;
my $dirbase = $#ARGV == 3 ? shift : '.';
chop($dirbase) if $dirbase ne '/' && $dirbase =~ /\/$/;
die "Missing directory: $dirbase\n" unless -d $dirbase;

$file_begin = shift;
$file_end = shift;
$_ = shift;
die "Last argument must be a positive integer.\n" unless /^\+?\d+$/ && $_ > 0;
$format = '%0' . $_ . 'd';

processDirectory($dirbase);

print "\n" if $verbose;
printf "Processed directories:%8d\n", $dirs;
printf "Processed files:      %8d\n", $files;
printf "Filenames matching:   %8d\n", $matches;
printf "%-22s%8d\n", $justPrint ? "Would rename:" : "Renamed:", $changes;
