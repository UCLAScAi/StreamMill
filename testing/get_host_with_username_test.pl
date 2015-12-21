#!/usr/bin/perl

use IO::Socket;
use Thread; 

$sys_port=5455;

sub send_message
{
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

send_message("localhost", $sys_port, "GetHostWithUsername smm");
print("Asked for host with username smm.\n");
send_message("localhost", $sys_port, "GetHostWithUsername nlaptev");
print("Asked for host with username smm2.\n");
send_message("localhost", $sys_port, "GetHostWithUsername smm3 pad");
print("Asked for host with username smm3.\n");
send_message("localhost", $sys_port, "GetHostWithUsername smm4 pad");
print("Asked for host with username smm4.\n");
send_message("localhost", $sys_port, "GetHostWithUsername smm7 pad");
print("Asked for host with username smm7.\n");
