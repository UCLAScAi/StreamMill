#!/usr/bin/perl
use FileHandle;
use Socket;
use Time::HiRes qw( usleep);

$sys_port=5444;

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

system("rm __queries__");
system("rm __declares__");
system("rm __aggregates__");

system("killall esl_start");
system(3);
system("./esl_start -p 5444&");
sleep(40);

send_message("stream", $sys_port, "AddStreams traffic traffic\nstream traffic(station_id int, speed int) source 'port5551';");
printf("Added stream\n");
sleep(3);

send_message("stream", $sys_port, "AddQueries traffic qa\nselect station_id, speed from traffic;");
printf("Added query\n");
sleep(3);

send_message("stream", $sys_port, "AddQueries traffic qb\nselect station_id, sum(speed) over(partition by station_id ROWS 2 preceding) from traffic;");
printf("Added query\n");
sleep(3);

send_message("stream", $sys_port, "AddStreams demo trainflowers\nstream trainflowers(id int, SL real, SW real, PL real, PW real, isSetosa int) source 'port5552';");
printf("Added stream\n");
sleep(3);

send_message("stream", $sys_port, "AddStreams demo testflowers\nstream testflowers(id int, SL real, SW real, PL real, PW real, isSetosa int) source 'port5553';");
printf("Added stream\n");
sleep(3);


send_message("stream", $sys_port, "AddStreams demo trainVert\nstream trainVert(id int, col int, val int, cLbl int);");
printf("Added stream\n");
sleep(3);

send_message("stream", $sys_port, "AddStreams demo testVert\nstream testVert(id int, col int, val int, cLbl int);");
printf("Added stream\n");
sleep(3);

send_message("stream", $sys_port, "AddTables demo classStat\ntable classStat(classLbl int, cnt real) memory;");
printf("Added table\n");
sleep(3);

send_message("stream", $sys_port, "AddTables demo colStat\ntable colStat(col int, val int, classLbl int, cnt real) memory;");
printf("Added table\n");
sleep(3);

send_message("stream", $sys_port, "AddAggregate demo aggregate dissembleFlowers(id int, v1 Int, v2 Int, v3 Int, v4 Int, classLbl Int):(id int, col Int, val Int, classLbl Int)
{
 initialize: iterate: {

  INSERT INTO RETURN VALUES (id, 1, v1, classLbl), (id, 2, v2, classLbl), (id, 3, v3, classLbl), (id, 4, v4, classLbl);
 }
};");
printf("Added aggregate\n");
sleep(4);

send_message("stream", $sys_port, "AddAggregate demo Aggregate learnClassifier(coli int, vali int, classLbli int, numColsi int):(a int) {
 initialize: iterate: {
   update classStat set cnt = cnt + 1 where classLbl = classLbli and coli = numColsi;
   insert into classStat values (classLbli, 1) where SQLCODE <> 0 and coli = numColsi;

   update colStat set cnt = cnt + 1
     where col = coli and val = vali and classLbl = classLbli;
   insert into colStat values (coli, vali, classLbli, 1) where SQLCODE <> 0;
 }
};");
printf("Added aggregate\n");
sleep(4);

send_message("stream", $sys_port, "AddAggregate demo Aggregate evaluateClassifier(idi int, coli int, vali int, numColsi int, classLbli int):(ido int, classLblo int, classo int) {
 table classes(cls int, p real) memory;

 Aggregate updateP(clsi int, coli int, vali int):(o real) {
   initialize: iterate: {
     update classes set p = p * (select cnt from colStat where col = coli and val = vali and classLbl = clsi)/(select cnt from classStat where classLbl = clsi)          where exists (select cnt from colStat where col = coli and val = vali and classLbl = clsi) and cls = clsi;

     /* if this combo of coli, vali, classLbli is not seen before then
        not clear what to do? (options)
        - set p = 0*
        - devide p by total number of classes
     */      update classes set p = 0 where cls = clsi and SQLCODE <> 0;
   }
 };

 Aggregate mymax(a real):(b real) {
   table t(m real) memory;    initialize: {
     insert into t values(a);
   }    iterate: {
     update t set m = a where m < a;
   }
   terminate: {
     insert into return select m from t;
   }
 }

 initialize: iterate: {       insert into classes select distinct classLbl, 0 from classStat where coli = 1;    update classes set p = (select cnt from classStat where classLbl = classes.cls)
                               /(select sum(cnt) from classStat)
     where coli = 1;
   select updateP(cls, coli, vali)    from classes;
   insert into return select idi, classLbli, cls from classes where coli = numColsi and p = (select maxr(p) from classes) and p > 0;    insert into return values(idi, classLbli, -1) where coli = numColsi and SQLCODE <> 0;

   delete from classes where coli = numColsi;
 }
};");
printf("Added aggregate\n");
sleep(5);

send_message("stream", $sys_port, "AddQueries demo trainVert\ninsert into trainVert select dissembleFlowers(id, SL, SW, PL, PW, isSetosa) from trainflowers;");
printf("Added query\n");
sleep(3);

send_message("stream", $sys_port, "AddQueries demo testVert\ninsert into testVert select dissembleFlowers(id, SL, SW, PL, PW, isSetosa) from testflowers;");
printf("Added query\n");
sleep(3);

send_message("stream", $sys_port, "AddQueries demo learnUDA\nselect learnClassifier(col, val, cLbl, 4) from trainVert;");
printf("Added query\n");
sleep(3);

send_message("stream", $sys_port, "AddQueries demo classifyUDA\nselect evaluateClassifier(id, col, val, 4, cLbl) from testVert;");
printf("Added query\n");
sleep(3);

printf("Done!!\n");
exit;
