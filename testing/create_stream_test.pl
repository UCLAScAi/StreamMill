#!/usr/bin/perl

use IO::Socket;
use Thread; 

$sys_port=5455;

sub open_socket
{
my $sock = new IO::Socket::INET (
                                 LocalHost => '127.0.0.1',
                                 LocalPort => $_[0],
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
      print("Closing port $_[0]\n");
      close($new_sock);
      exit;
    }
    print $_;
  }
}
}

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
  print(SOCK "quit_port_8080");
}

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

sub generate_random_string
{
	my $length_of_randomstring=shift;# the length of 
			 # the random string to generate

	my @chars=('a'..'z','A'..'Z');
	my $random_string;
	foreach (1..$length_of_randomstring) 
	{
		# rand @chars will generate a random 
		# number between 0 and scalar @chars
		$random_string.=$chars[rand @chars];
	}
	return $random_string;
}
$str = generate_random_string(5)."z";
send_message("localhost", $sys_port, "AddStreams anothersmm MytrafficNew\nstream mystream".$str."(station_id int, speed int) source 'port7000';");
print("Stream created (".$str.").\n");
