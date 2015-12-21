#!/usr/bin/perl

# This script tests multi-user support of SMM.

use FileHandle;
use Socket;
use Time::HiRes qw( usleep);
use IO::Socket;

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


send_message("localhost", $sys_port, "AddNewUser anothersmm anothersmm anothersmm\@smm.com smmsmm");
printf("Added user anothersmm\n");


send_message("localhost", $sys_port, "AddStreams anothersmm trafficNew\nstream trafficNew(station_id int, speed int) source 'port5552';");
printf("Added stream\n");

send_message("localhost", $sys_port, "AddQueries anothersmm qaNew\nselect station_id, speed from trafficNew;");
printf("Added query\n");

send_message("localhost", $sys_port, "ActivateStreamModuleByName smm traffic");
printf("Activated stream module traffic\n");

send_message("localhost", $sys_port, "ActivateStreamModuleByName anothersmm trafficNew");
printf("Activated stream module trafficNew\n");

send_message("localhost", $sys_port, "ActivateQueryModuleByName smm qa");
printf("Activated query module traffic\n");

send_message("localhost", $sys_port, "ActivateQueryModuleByName anothersmm qaNew");
printf("Activated query module trafficNew\n");

send_message("localhost", $sys_port, "MonitorAllOfIP smm 127_0_0_1 o 8080");
printf("Monitoring All Buffers of user smm\n");

send_message("localhost", $sys_port, "MonitorAllOfIP anothersmm 127_0_0_1 o 8080");
printf("Monitoring All Buffers of user anothersmm\n");

system("/usr/bin/perl /research/workspace/StreamMill/testing/open_port.pl&");
printf("Opening local port 8080\n");

send_data("localhost", "5551");
# send_data("localhost", "5552");
# printf("Data sent to port 5551\n");
exit;
