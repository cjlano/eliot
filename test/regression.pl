#!/usr/bin/perl

use strict;
use warnings;

use Cwd 'abs_path';

my $root_path = $0;
$root_path =~ s/regression.pl/../;
$root_path = abs_path($root_path);

# Change to the test/ directory, because some scenarii expect
# to find saved games in there
chdir("$root_path/test");

my $driver_file = "driver";
# Look for ~/ods5.dawg
my $ods = "$ENV{HOME}/ods5.dawg";

# File extensions
my $input_ext = ".input";
my $ref_ext = ".ref";
my $run_ext = ".run";


# Find the dictionary
if (not -f $ods)
{
    die "Cannot find dictionary $ods: $!";
}

# Find the text interface
my $eliottxt;
if (-x "$root_path/utils/eliottxt")
{
    $eliottxt = "$root_path/utils/eliottxt";
}
elsif (-x "$root_path/utils/eliottxt.exe")
{
    $eliottxt = "$root_path/utils/eliottxt.exe";
}
else
{
    die "Cannot find the text interface executable"
}


# Fill a map of (scenario --> randseed) from the driver file.
# Also fill the list of scenarios in the order of the driver file.
my %scenario_map;
my @all_scenarios;
open(DRIVER, $driver_file) or die "Cannot open the scenario list: $!";
while(<DRIVER>)
{
    chomp;
    my $line = $_;
    $line =~ s/#.*//;
    if ($line =~ /^\s*(\w+)\s+(\d+)\s*$/)
    {
        $scenario_map{$1} = $2;
        push(@all_scenarios, $1);
    }
}
close(DRIVER);


# Select the scenarios to play: if there was no argument in the commandline
# we play all the scenarios, otherwise we play only the specified ones
my @scenarios_to_play;
if (@ARGV == 0)
{
    @scenarios_to_play = @all_scenarios;
}
else
{
    # Remove known file extensions in case they are present
    foreach my $item (@ARGV)
    {
        $item =~ s/$input_ext$|$ref_ext$|$run_ext$//;
        push(@scenarios_to_play, $item);
    }
}


# Actually play the selected scenarios
my @errors;
foreach my $scenario (@scenarios_to_play)
{
    print "Scenario: $scenario\n";
    my $input_file = $scenario . $input_ext;
    my $ref_file   = $scenario . $ref_ext;
    my $run_file   = $scenario . $run_ext;
    my $randseed = $scenario_map{$scenario};

    # Check that the needed files exist
    if (not -f $input_file)
    {
        print "--> Error: Missing file: $input_file\n";
        push(@errors, $scenario);
        next;
    }
    if (not -f $ref_file)
    {
        print "--> Error: Missing file: $ref_file\n";
        push(@errors, $scenario);
        next;
    }

    # OK, let's do the actual stuff
    unlink $run_file;
    my $rc = `$eliottxt $ods $randseed < $input_file > $run_file 2>&1`;
    if (not $rc eq "")
    {
        print "--> Error: Execution of scenario failed (return value: $rc)\n";
        push(@errors, $scenario);
        next;
    }

    # Is the output file different from the reference file?
    my $diff = `diff $ref_file $run_file`;
    if (not $diff eq "")
    {
        print "--> Error: found differences:\n";
        print $diff;
        push(@errors, $scenario);
    }
}


# Display the results
print "\nSummary: ";
if (@errors == 0)
{
    print "Everything was OK.\n";
}
else
{
    my $errorsCount = @errors;
    print "$errorsCount error(s). The following scenario(s) have failed:\n";
    print "@errors\n"
}
