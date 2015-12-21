#!/usr/bin/perl

package large_ut;

require utility;

our @users = ();
our %user_streams = ();
our %user_queries = ();
our $load_level = 1; # max: 9

sub get_added_users {
	foreach (@users) {
		print $_."\n";
	}
}

sub get_added_streams {
	foreach (keys %user_streams) {
		$k = $_;
		my @arr = @{$user_streams{$k}};
		foreach (@arr) {
			print $k." --> ".$_."\n";
		}
	}
}

sub get_added_queries {
	foreach (keys %user_queries) {
		$k = $_;
		my @arr = @{$user_queries{$k}};
		foreach (@arr) {
			print $k." --> ".$_."\n";
		}
	}
}

sub test_add_multiple_users
{
	for ($i = 0; $i < $load_level; $i++) {
		$usr = "u".$i.utility::generate_random_string();
		push(@users, $usr);
		utility::send_message("localhost", utility::get_port(), "AddNewUser ".$usr." ".$usr." ".$usr."\@email.com ".$usr);
		printf("Added user ".$usr."\n");
	}
}

sub test_add_multiple_streams
{
	foreach (@users) {
		my @u_streams = ();
		for ($i = 0; $i < $load_level; $i++) {
			$str = utility::generate_random_string();
			push(@u_streams, $str);
			print "Adding stream ".$str." for user ".$_."\n";
			utility::send_message("localhost", utility::get_port(), "AddStreams ".$_." ".$str."\nstream ".$str."(station_id int, speed int) source 'port555".$i."';");
		}
		$user_streams{$_} =  [@u_streams];
	}
}

sub large_ut::test_add_multiple_queries
{
	foreach (keys %user_streams) {
		$usr = $_;
		my @arr = @{$user_streams{$usr}};
		@u_queries = ();
		foreach (@arr) {
			$str = $_;
			for ($i = 0; $i < $load_level; $i++) {
				$qry = utility::generate_random_string();
				push(@u_queries, $qry);
				print "Adding query ".$qry." for user ".$usr."\n";
				utility::send_message("localhost", utility::get_port(), "AddQueries ".$usr." ".$qry."\nselect station_id, speed from ".$str.";");
			}
		}
		$user_queries{$usr} =  [@u_queries];
	}
}

sub test_activate_streams
{
	foreach (keys %user_streams) {
		$k = $_;
		my @arr = @{$user_streams{$k}};
		foreach (@arr) {
			utility::send_message("localhost", utility::get_port(), "ActivateStreamModuleByName ".$k." ".$_);
			printf("Activated stream module ".$_." for user ".$k."\n");
		}
	}
}

sub test_activate_queries
{
	foreach (keys %user_streams) {
		$k = $_;
		my @arr = @{$user_queries{$k}};
		foreach (@arr) {
			utility::send_message("localhost", utility::get_port(), "ActivateQueryModuleByName ".$k." ".$_);
			printf("Activated query module ".$_." for user ".$k."\n");
		}
	}
}

sub test_monitor_all_ip
{
	foreach (@users) {
		utility::send_message("localhost", utility::get_port(), "MonitorAllOfIP ".$_." 127_0_0_1 o 8080");
		printf("Monitoring All Buffers of user ".$_."\n");
	}
}

sub test_send_data
{
	# system("/usr/bin/perl /research/workspace/StreamMill/testing/open_port.pl&");
	utility::send_data("localhost", "5551");
}
1;
