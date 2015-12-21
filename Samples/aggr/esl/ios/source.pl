#!/usr/bin/perl
use FileHandle;
use Socket;

$usage = "source.pl host port sourcename
 where
    host = The name or IP address of the TelegraphCQ host to connect to
    port = The port number on which the TCQ host is listening

";

if ($#ARGV<1) {
        print "Missing one or more parameters.\n Usage: $usage";
        exit;
}
$host = $ARGV[0];
$port = $ARGV[1];




for $f (@ARGV) {
$i++;
if ($f eq "-h" || $f eq "--help") { print "Usage: $usage"; exit; }
}

#open the socket
$iaddr = inet_aton($host);
$paddr = sockaddr_in($port,$iaddr);
$proto = getprotobyname('tcp');



socket(SOCK, PF_INET, SOCK_STREAM, $proto) || die "Couldn't open socket: $!";
connect(SOCK, $paddr) || die "connect: $!";
autoflush SOCK,1;
#print SOCK "abc";

while (<STDIN>) {
    $str = $_;
    print($str);
    print(SOCK  $str);
    $count++;
}
close(SOCK);

exit;
