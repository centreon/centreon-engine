#!/usr/bin/perl

use strict;
use warnings;

my $type = $ARGV[0];
my $service = $ARGV[1];

if (!defined $service) {
  $service = "Unknown";
}

my $now = time();

my $val = 5 * sin($now / 1000.0);
my $high_threshold = 4.0;
my $low_threshold = 3.0;

if ($type eq "service") {
  my ($state, $STATE);
  if ($val > $high_threshold) {
    $state = 2;
    $STATE = "CRITICAL: ";
  } elsif ($val > $low_threshold) {
    $state = 1;
    $STATE = "WARNING: ";
  } else {
    $state = 0;
    $STATE = "OK: ";
  }

  printf("${STATE}active-check $service | metric=%.2f;$low_threshold;$high_threshold;-5;5\n", $val);
  exit $state
}
else {
  if ($val < 4.5) {
    print("UP: active-check $service\n");
    exit 0;
  }
  else {
    print("DOWN: active-check $service\n");
    exit 1;
  }
}
