#!/usr/bin/perl

# This script creates a UDA with a null pointer exception and makes sure that
# the SMM system does not crash.

use FileHandle;
use Socket;
use Time::HiRes qw( usleep);

$sys_port=5455;

sub send_data
{
  $host = $_[0];
  $port = $_[1];

  $iaddr = inet_aton($host);
  $paddr = sockaddr_in($port,$iaddr);
  $proto = getprotobyname('tcp');

  socket(SOCK, PF_INET, SOCK_STREAM, $proto) || die "Couldn't open socket: $!";
  connect(SOCK, $paddr) || die "connect: $!";
  autoflush SOCK,1;

  for ($count = 0; $count < 100; $count++) {
    $mul = $count*2;
    $str = "$count, $mul\n";
    print ("sending : $str");
    print(SOCK "$str");
  }
}

sub send_message
{
  #num args = $#_;
  #vals $_[]
  $machine = $_[0];
  $port = $_[1];
  $stringToSend = $_[2];

  $iaddr = inet_aton($machine);
  $paddr = sockaddr_in($port,$iaddr);
  $proto = getprotobyname('tcp');

  socket(SOCK, PF_INET, SOCK_STREAM, $proto) || die "Couldn't open socket: $!";
  connect(SOCK, $paddr) || die "connect: $!";
  autoflush SOCK,1;

  print(SOCK  "$stringToSend");
  close(SOCK);
}

# Create a new user smm with password smmsmm
send_message("localhost", $sys_port, "AddAggregate smm caggregate crashMePlease(next int):int
{
@
@
@
double test;
@
initialize: {
@
@
} 
iterate: {
@
int* k = NULL;
k[100] = 100; // crash
@
 }
};");
printf("Added aggregate\n");

send_message("localhost", $sys_port, "AddQueries smm myQ\nselect crashMePlease(speed) from traffic;");
printf("Added query\n");

send_message("localhost", $sys_port, "ActivateStreamModuleByName smm traffic");
printf("Activated stream module traffic\n");

send_message("localhost", $sys_port, "ActivateQueryModuleByName smm myQ");
printf("Activated query module myQ\n");

send_message("localhost", $sys_port, "MonitorBuffer smm stdout_a127_0_0_1_363483 o 8080");
printf("Monitoring buffers for stdout_a127_0_0_1_396602 o 8080\n");

send_data("localhost", "5551");
printf("Data sent to port 5551\n");

exit;
