#!/usr/bin/perl
#
# Martin Titz, 2013, 2014, 2019, 2025
#
# Statistics by file extensions
#

use strict;

my $NONE = "<noext>";  # text for no extension

my %extensions;
my %basename2extensions;
my $all = 0;
my $filenameStatistics = 0;
my $recursive = 0;
my $verbose = 0;

sub processFilename {
    my ($dirpart, $filepart) = @_;
    my $dotIndex = rindex $filepart, ".";
    my $extension = $dotIndex == -1 ? "" : substr $filepart, $dotIndex+1;
    ++$extensions{$extension eq "" ? $NONE : $extension};
    if ($filenameStatistics) {
        my $basename = $dotIndex == -1
            ? $filepart
            : substr $filepart, 0, $dotIndex;
        if (! defined $basename2extensions{$basename}) {
            $basename2extensions{$basename} = [];
        }
        push @{$basename2extensions{$basename}}, $extension;
    }
}

sub processDirectory {
    my $dir = shift(@_);
    print "Processing directory $dir\n" if $verbose;
    opendir(my $dh, $dir) or warn "Cannot open directory $dir: $!";
    while (defined(my $file = readdir $dh)) {
        next if $file eq "." || $file eq "..";
        next if substr($file, 0, 1) eq "." && !$all;
        my $filename = "$dir/$file";
        stat($filename);
        if (-d _) {
            processDirectory($filename) if $recursive;
        } elsif (-f _) {
            processFilename($dir, $file);
        } else {
            print "  Ignoring special file $filename\n" if $verbose;
        }
    }
    closedir $dh;
}

sub processFinal {
    foreach my $key (sort(keys %extensions)) {
        printf "%-10s%6d\n", $key, $extensions{$key};
    }
    if ($filenameStatistics) {
        printf("\n");
        for my $bn (sort(keys %basename2extensions)) {
            print "$bn";
            for my $ext (sort @{ $basename2extensions{$bn} }){
                $ext = $NONE if $ext eq "";
                print " ", $ext;
            }
            print "\n";
        }
    }
}

$0 =~ s#.*/##g;
my $usage = "Usage: $0 [-a] [-f] [-r] [-v] file...\n";
while (defined $ARGV[0] && $ARGV[0] =~ /^-/) {
    $_ = shift;
    if (/^-a$/) {
        $all = 1;
    } elsif (/^-h$/) {
        die $usage;
    } elsif (/^-f$/) {
        $filenameStatistics = 1;
    } elsif (/^-r$/) {
        $recursive = 1;
    } elsif (/^-v$/) {
        $verbose = 1;
    } else {
        die "Unrecognized switch: $_\n";
    }
}
@ARGV = (".") unless @ARGV;
for (@ARGV) {
    if (-f $_) {
        processFilename($_);
    } elsif (-d $_) {
        processDirectory($_);
    } else {
        print "$_ does not exist.\n";
    }
}
print "\n" if $verbose;
processFinal();
