#!/usr/bin/perl

use IO::Socket;

sub open_socket
{
my $sock = new IO::Socket::INET (
                                 LocalHost => '127.0.0.1',
                                 LocalPort => '8080',
                                 Proto => 'tcp',
                                 Listen => 1,
                                 Reuse => 1,
                                );
die "Could not create socket: $!\n" unless $sock;
print "Opening port\n";
while ($new_sock = $sock->accept()) {
  while($input = scalar<$new_sock>){
    if ($input =~ /^quit_port_8080/i) {
      print("Terminating...\n");
      print("Closing port 8080\n");
      close($new_sock);
      exit;
    }
    print $_;
  }
}
}

open_socket();
