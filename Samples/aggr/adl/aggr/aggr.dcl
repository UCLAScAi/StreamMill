aggregate demo$dissembleFlowers (id int, v1 Int, v2 Int, v3 Int, v4 Int, classLbl Int):(id int, col Int, val Int, classLbl Int)
{
 initialize: iterate: {

  INSERT INTO RETURN VALUES (id, 1, v1, classLbl), (id, 2, v2, classLbl), (id, 3, v3, classLbl), (id, 4, v4, classLbl);
 }
};
Aggregate demo$learnClassifier (coli int, vali int, classLbli int, numColsi int):(a int) {
 initialize: iterate: {
   update classStat set cnt = cnt + 1 where classLbl = classLbli and coli = numColsi;
   insert into classStat values (classLbli, 1) where SQLCODE <> 0 and coli = numColsi;

   update colStat set cnt = cnt + 1
     where col = coli and val = vali and classLbl = classLbli;
   insert into colStat values (coli, vali, classLbli, 1) where SQLCODE <> 0;
 }
};
Aggregate demo$evaluateClassifier (idi int, coli int, vali int, numColsi int, classLbli int):(ido int, classLblo int, classo int) {
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
};
AGGREGATE hamidmou$Topk      (Next INT):
REAL
{
 TABLE state (max Int);
 TABLE count (id Int, val Real, cnt int);
INITIALIZE:
{
 INSERT INTO state VALUES (1);
 INSERT INTO count VALUES (1, Next, 0);
}

ITERATE:
{
 UPDATE state SET  max=max+1;
 
DELETE FROM count 
  WHERE id+1000 = (SELECT max FROM state);
 
INSERT INTO count VALUES ((SELECT max FROM state), Next, 0);
 
UPDATE count SET cnt=cnt+1
  WHERE 
   val <= Next and (id <> (SELECT max FROM state));

 DELETE FROM count
  WHERE cnt >= 20;
 
INSERT INTO return SELECT count(id) from count;
}

TERMINATE:
{
}

};

