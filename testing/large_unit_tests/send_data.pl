#!/usr/bin/perl
use FileHandle;
use Socket;
use Time::HiRes qw( usleep);

$usage = "ss.pl username__streamname host port times delay 
 where
    username__streamname = username and stream name connected by a double '_'
    host = The name or IP address of the host to connect to
    port = The port number on which the  host is listening
    times = Number of times the data should be read
    delay = Delay between the iterations
";

if ($#ARGV<4) {
  print "Missing one or more parameters.\n Usage: $usage";
  exit;
}
$usr_strm = $ARGV[0];
$host = $ARGV[1];
$port = $ARGV[2];
$times = $ARGV[3];
$sens = $ARGV[4];

if ($times <= 0 || $sens < 0) {
  print "Missing one or more parameters.\n Usage: $usage";
  exit;
}

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
  system("cat data_to_send.txt > sensor-counts");
  open(SENSOR, "sensor-counts") || die "Can't open sensor data file: $!\n";
  $count=0;
  while (<SENSOR>) {
    #parse the line
    ($stationid,$speed) = split(/,/);
    #output it again (Why did I parse it in the first place? I don't know)
    $str = "$usr_strm,$stationid,$speed";
    if($count < 1000) {
      print(SOCK  "$str");
      $count++;
      #usleep(10000);
    }
  }
  close(SENSOR);
  print ("Done sending to server.  $count tuples sent\n");
  if($sens > 0) {
    sleep $sens;
  }
}
close(SOCK);
exit;
