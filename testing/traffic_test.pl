#!/usr/bin/perl

# This script creates a simple query and a stream and sends traffic readings to the query.

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
  connect(SOCK, $paddr) || return "connect: $!";
  autoflush SOCK,1;

  for ($count = 0; $count < 100; $count++) {
    $mul = $count*2;
    $str = "smm__traffic, $count, $mul\n";
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

  socket(SOCK, PF_INET, SOCK_STREAM, $proto) || return "Couldn't open socket: $!";
  connect(SOCK, $paddr) || return "connect: $!";
  autoflush SOCK,1;

  print(SOCK  "$stringToSend");
  close(SOCK);
}
send_message("localhost", $sys_port, "AddStreams smm traffic\nstream traffic(station_id int, speed int) source 'port5551';");
printf("Created stream traffic\n");

send_message("localhost", $sys_port, "ActivateStreamModuleByName smm traffic");
printf("Activated stream module traffic\n");

send_message("localhost", $sys_port, "ActivateQueryModuleByName smm qa");
printf("Activated query module traffic\n");

send_message("localhost", $sys_port, "MonitorAllOfIP smm 127_0_0_1 o 8080");
printf("Monitoring buffers\n");

send_data("localhost", "5551");
printf("Data sent to port 5551\n");

# send_message("localhost", $sys_port, "UnMonitorAllOfIP smm 127_0_0_1 o 8080");
# printf("UnMonitoring buffers\n");

# Wait until all of the data is sent
sleep(3);

send_message("localhost", 8080, "quit_port_8080");
printf("Sent kill signal\n");
exit;


