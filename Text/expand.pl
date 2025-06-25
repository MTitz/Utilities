#!/usr/bin/perl -w

$/ = undef;
while (<>) {
    s/\n([^\n])/\n\n$1/g;
    print;
}
