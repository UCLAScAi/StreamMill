#!/usr/bin/perl
use FileHandle;
use Socket;
use Time::HiRes qw( usleep);

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


while(1) {
  system("ps | grep esl_start > tempFile");
  #system("wc -l tempFile > tempFile2");
  open(READER, "tempFile") || die "Can't open word count file!\n";
  #$log_line = <READER>;
  #print("first $log_line");
  $restart = 0;
  $cnt = 0;
  while ($log_line = <READER>) {
    ($a1,$a2,$a3,$a4,$status) = split(/\s+/, $log_line);
    $cnt = $cnt+1;
    print("a1 $a1, a2 $a2, a3 $a3, a4 $a4, stat $status, cnt $cnt\n");
    if($status eq "<defunct>") {
      $restart = 1;
    }
    if($a4 eq "<defunct>") {
      $restart = 1;
    }
    if($a3 eq "<defunct>") {
      $restart = 1;
    }
  }
  if($restart == 1 || $cnt < 2) {
    #output it again (Why did I parse it in the first place? I don't know)
    #print ("lines- $lines, name- $name\n");
    print("System restarted because: restart $restart, cnt $cnt\n");
    #  @timeData = localtime(time);
    #  print join(' ', @timeData);
    #  print("\n");
    system("killall esl_start");
    system("./esl_start -p 5444&");

    #next activating streams and queries
    #sleep(60);
    #send_message("stream", "5431", "ActivateStream demoTraffic a131_179_64_232_56638");
    #sleep(2);
    #printf("Activated stream\n");
    #send_message("stream", "5431", "ActivateQuery demoTraffic a131_179_64_232_280664");
    #sleep(2);
    #printf("Activated query 1\n");
    #send_message("stream", "5431", "ActivateQuery demoTraffic a131_179_64_232_312653");
    #printf("Activated query 2\n");
    #system("./ss-always.pl stream 5433 1 0 &");
  }
  close(SENSOR);
  close(READER);
  sleep(3);
}
exit;
