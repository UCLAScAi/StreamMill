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

send_message("localhost", $sys_port, "GetLeastLoadedHost smm padd");
print("User smm asked for the least loaded server.\n");
