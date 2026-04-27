#!/usr/bin/perl
use strict;
use utf8;
use warnings;
use open qw(:std :utf8);
use File::HomeDir;
use Getopt::Std;

my $dict_path = '/usr/share/dict/';
my @english_dicts = ('american-english', 'british-english');
my @german_dicts = ('ogerman', 'ngerman');
my $splitchar = ':';
my $homedir = File::HomeDir->my_home;

$/ = "";                        # Enable paragraph mode.

my %wordcount;
my %ignore;
my $verbose = 0;
our ($opt_d, $opt_e, $opt_n, $opt_v, $opt_w);

my $a_umlaut = "\x{c3}\x{a4}";
my $o_umlaut = "\x{c3}\x{b6}";
my $u_umlaut = "\x{c3}\x{bc}";
my $A_umlaut = "\x{c3}\x{84}";
my $O_umlaut = "\x{c3}\x{96}";
my $U_umlaut = "\x{c3}\x{9c}";
my $sz_char  = "\x{c3}\x{9f}";
my $right_single_quotation_mark = "\x{e2}\x{80}\x{99}";

sub read_ignorefile
{
    my $filename = shift(@_);
    print STDERR "  reading in words from $filename\n" if $verbose;
    open(FILE, "<:encoding(UTF-8)", $filename) || die "Can't open file $filename: $!\n";
    while (<FILE>) {
        s/^\N{BOM}//;               # Remove BOM.
        tr/A-Z/a-z/;                # Canonicalize to lowercase.
        s/$A_umlaut/$a_umlaut/g;
        s/$O_umlaut/$o_umlaut/g;
        s/$U_umlaut/$u_umlaut/g;
        s/Ä/ä/g;
        s/Ö/ö/g;
        s/Ü/ü/g;
        my @words = split /\s+/m;
        foreach (@words) {
            s/^['`’-]+//;
            s/['`’-]+$//;
            #tr/`’/''/;
            $ignore{$_}++ unless /^[0-9'`’-]*$/;
        }
    }
    close FILE;
}

sub read_dicts
{
    read_ignorefile($dict_path . $_) foreach (@_);
}

getopts('denvw:');

$verbose = 1 if $opt_v;
if ($opt_d) {
    print STDERR "German mode\n" if $verbose;
    read_dicts(@german_dicts);
}
if ($opt_e) {
    print STDERR "English mode\n" if $verbose;
    read_dicts(@english_dicts);
}
if ($opt_w) {
    print STDERR "Processing ignore files\n" if $verbose;
    for (split /$splitchar/, $opt_w) {
        s/^~/$homedir/;
        read_ignorefile($_);
    }
}

print STDERR "Now processing files: @ARGV\n" if $verbose;

while (<>) {
    #binmode ARGV, ':utf8';
    s/^\N{BOM}//;               # Remove BOM.
    s/-\n//;                    # Dehyphenate hyphenations.
    tr/A-Z/a-z/;                # Canonicalize to lowercase.
    s/$A_umlaut/$a_umlaut/g;
    s/$O_umlaut/$o_umlaut/g;
    s/$U_umlaut/$u_umlaut/g;
    s/${right_single_quotation_mark}/'/g;
    s/Ä/ä/g;
    s/Ö/ö/g;
    s/Ü/ü/g;
    s/[’´]/'/g;
    #s/[^a-z${a_umlaut}${o_umlaut}${u_umlaut}${sz_char}0-9' \n-]/ /gu;
    s/[,;:.?!"″()[\]{}+*\/~=<>€\$£&|“”„‘»«‚—…_#@°%^\\]/ /g;
    my @words = split /\s+/m;
    foreach (@words) {
        s/^['`’-]+//;
        s/['`’-]+$//;
        s/^[0-9]+′[0-9]+$//;
        #s/[`’]/'/;  # TODO
        next if $ignore{$_} || /^$/ || /^–$/;
        $wordcount{$_}++ unless /^[0-9'`’-]*$/;
    }
}

my @sorted_keys = $opt_n
    ? sort { $wordcount{$b} <=> $wordcount{$a} || $a cmp $b } keys(%wordcount)
    : sort keys(%wordcount);
foreach my $word (@sorted_keys) {
    printf "%5d %s\n", $wordcount{$word}, $word;
}
