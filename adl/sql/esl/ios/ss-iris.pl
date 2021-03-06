#!/usr/bin/perl
use FileHandle;
use Socket;
use Time::HiRes qw( usleep);

$usage = "ss.pl host port times delay
 where
    user__streamname = username followed by two underscores followed by a stream name. 
    file = name of the file where data is
    host = The name or IP address of the host to connect to
    port = The port number on which the  host is listening
    times = Number of times the data should be read
    delay = Delay between the iterations
";

if ($#ARGV<5) {
  print "Missing one or more parameters.\n Usage: $usage";
  exit;
}
$usr_stream = $ARGV[0];
$filename = $ARGV[1];
$host = $ARGV[2];
$port = $ARGV[3];
$times = $ARGV[4];
$sens = $ARGV[5];



if ($times <= 0 || $sens <= 0) {
  print "Missing one or more parameters.\n Usage: $usage";
  exit;
}

print "Polling sensor data every $sens for $times times.\n";

#open the socket
$iaddr = inet_aton($host);
$paddr = sockaddr_in($port,$iaddr);
$proto = getprotobyname('tcp');


socket(SOCK, PF_INET, SOCK_STREAM, $proto) || die "Couldn't open socket: $!";
connect(SOCK, $paddr) || die "connect: $!";
autoflush SOCK,1;

$iter = 0;
while ($iter < $times) {
  $iter = $iter + 1;
  print ("Sending data to Server\n");
  open(POINTS, $filename) || die "Can't open sensor data file: $!\n";
  $count=0;
  while (<POINTS>) {
    #parse the line
    ($id,$sl,$sw,$pl,$pw,$isset) = split(/,/);
    #output it again (Why did I parse it in the first place? I don't know)
    $str = "$usr_stream,$id,$sl,$sw,$pl,$pw,$isset";
    #if($count < 600) {
    print ("ct $count:$str");
    print(SOCK "$str");
    #usleep(10000);
    $count++;
    #}
  }
  close(POINTS);
  print ("Done sending to server.  $count tuples sent\n");
  sleep $sens;
}


print ("Closing connection to server.\n");
close(SOCK);
exit;
