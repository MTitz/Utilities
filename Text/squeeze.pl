#!/usr/bin/perl -w

$/ = undef;
while (<>) {
    s/\n\n\n/\n\n/g;
    print;
}
