#! /usr/bin/env perl
use CondorTest;

Condor::DebugOff();

#$cmd = 'job_condorc_ab_van.cmd';
$cmd = $ARGV[0];

print "Submit file for this test is $cmd\n";
print "looking at env for condor config\n";
system("printenv | grep CONDOR_CONFIG");

$condor_config = $ENV{CONDOR_CONFIG};
print "CONDOR_CONFIG = $condor_config\n";

$testname = 'Condor-C A & B test - vanilla U';

$aborted = sub {
	my %info = @_;
	my $done;
	print "Abort event not expected!\n";
	#die "Want to see only submit, release and successful completion events for periodic release test\n";
};

$held = sub {
	my %info = @_;
	my $cluster = $info{"cluster"};
	my $holdreason = $info{"holdreason"};

	print "Held event not expected: $holdreason \n";
	system("condor_status -any -l");
	system("ps auxww | grep schedd");
	system("cat `condor_config_val LOG`/SchedLog");
	system("cat `condor_config_val LOG`/MasterLog");

	exit(1);
};

$executed = sub
{
	my %args = @_;
	my $cluster = $args{"cluster"};

	print "Start test timer from execution time\n";
	CondorTest::RegisterTimed($testname, $timed, 1800);
};

$timed = sub
{
	die "Test took too long!!!!!!!!!!!!!!!\n";
};

$success = sub
{
	my %info = @_;

	# Verify that output file contains expected "Done" line
	$output = $info{"output"};
	open( OUTPUT, "< $output" );
	@output_lines = <OUTPUT>;
	close OUTPUT;
	if( !grep(/Done/,@output_lines) ) {
	    die "Output file $output is missing expected output!\n";
	}

	# Verify that output file contains the contents of the
	# input file.
	$input = $info{"input"};
	open( INPUT, "< $input" );
	@input_lines = <INPUT>;
	close INPUT;
	for my $input_line (@input_lines) {
	    if( !grep($_ eq $input_line,@output_lines) ) {
		die "Output file is missing echoed input!\n";
	    }
	}

	# Verify expected output in outfile1.
	$output1 = "outfile1";
	open( OUTPUT1, "< $output1" );
	@output1_lines = <OUTPUT1>;
	close OUTPUT1;
	if( !grep(/Output to file 2/,@output1_lines) ) {
	    die "Output file $output1 is missing expected output!\n";
	}

	print "Success: ok\n";
};

$release = sub
{
	print "Release expected.........\n";
	my @adarray;
	my $status = 1;
	my $cmd = "condor_reschedule";
	$status = CondorTest::runCondorTool($cmd,\@adarray,2);
	if(!$status)
	{
		print "Test failure due to Condor Tool Failure<$cmd>\n";
		exit(1)
	}
};

CondorTest::RegisterExitedSuccess( $testname, $success);
CondorTest::RegisterExecute($testname, $executed);
CondorTest::RegisterRelease( $testname, $release );
CondorTest::RegisterHold( $testname, $held );

if( CondorTest::RunTest($testname, $cmd, 0) ) {
	print "$testname: SUCCESS\n";
	exit(0);
} else {
	die "$testname: CondorTest::RunTest() failed\n";
}

