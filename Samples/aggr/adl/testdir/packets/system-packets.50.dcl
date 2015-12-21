stream  tcp1 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5600' order by current_time;

stream  tcp2 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5610' order by current_time;

stream  tcp3 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5620' order by current_time;

stream s1_extra (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s1 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s2_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink1 (	
        a_time 		timestamp
) order by a_time;

stream sink2 (	
        a_time 		timestamp
) order by a_time;

stream sink3 (	
        a_time 		timestamp
) order by a_time;

insert into s1_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp1;
insert into s1 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s1_extra;

insert into s2_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp2;

insert into sink1 select a_time from s1 where from_ip > 0;
insert into sink2 select a_time from s2_14 where from_ip = 14;
insert into sink3 select current_time from tcp3 where from_ip = 263;

stream  tcp4 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5630' order by current_time;

stream  tcp5 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5640' order by current_time;

stream  tcp6 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5650' order by current_time;

stream s5_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s6_48 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s6_extra (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink4 (	
        a_time 		timestamp
) order by a_time;

stream sink5 (	
        a_time 		timestamp
) order by a_time;

stream sink6 (	
        a_time 		timestamp
) order by a_time;

insert into s5_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp5;

insert into s6_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp6;
insert into s6_48 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s6_extra;

insert into sink4 select current_time from tcp4;
insert into sink5 select a_time from s5_14 where from_ip = 14;
insert into sink6 select a_time from s6_48 where from_ip = 48;

stream  tcp7 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5660' order by current_time;

stream  tcp8 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5670' order by current_time;

stream  tcp9 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5680' order by current_time;

stream s7 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s8_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s9_15 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink7 (	
        a_time 		timestamp
) order by a_time;

stream sink8 (	
        a_time 		timestamp
) order by a_time;

stream sink9 (	
        a_time 		timestamp
) order by a_time;

insert into s7 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp7;

insert into s8_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp8;

insert into s9_15 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp9;

insert into sink7 select a_time from s7 where from_ip > 0;
insert into sink8 select a_time from s8_14 where from_ip = 14;
insert into sink9 select a_time from s9_15 where from_ip = 15;

stream  tcp10 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5690' order by current_time;

stream s10_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink10 (	
        a_time 		timestamp
) order by a_time;

insert into s10_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp10;
insert into sink10 select a_time from s10_14 where from_ip = 14;
 
stream  tcp11 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5700' order by current_time;

stream  tcp12 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5710' order by current_time;

stream  tcp13 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5720' order by current_time;

stream s11_extra (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s11 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s12_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink11 (	
        a_time 		timestamp
) order by a_time;

stream sink12 (	
        a_time 		timestamp
) order by a_time;

stream sink13 (	
        a_time 		timestamp
) order by a_time;

insert into s11_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp11;
insert into s11 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s11_extra;

insert into s12_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp12;

insert into sink11 select a_time from s11 where from_ip > 0;
insert into sink12 select a_time from s12_14 where from_ip = 14;
insert into sink13 select current_time from tcp13 where from_ip = 263;

stream  tcp14 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5730' order by current_time;

stream  tcp15 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5740' order by current_time;

stream  tcp16 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5750' order by current_time;

stream s15_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s16_48 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s16_extra (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink14 (	
        a_time 		timestamp
) order by a_time;

stream sink15 (	
        a_time 		timestamp
) order by a_time;

stream sink16 (	
        a_time 		timestamp
) order by a_time;

insert into s15_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp15;

insert into s16_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp16;
insert into s16_48 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s16_extra;

insert into sink14 select current_time from tcp14;
insert into sink15 select a_time from s15_14 where from_ip = 14;
insert into sink16 select a_time from s16_48 where from_ip = 48;

stream  tcp17 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5760' order by current_time;

stream  tcp18 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5770' order by current_time;

stream  tcp19 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5780' order by current_time;

stream s17 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s18_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s19_15 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink17 (	
        a_time 		timestamp
) order by a_time;

stream sink18 (	
        a_time 		timestamp
) order by a_time;

stream sink19 (	
        a_time 		timestamp
) order by a_time;

insert into s17 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp17;

insert into s18_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp18;

insert into s19_15 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp19;

insert into sink17 select a_time from s17 where from_ip > 0;
insert into sink18 select a_time from s18_14 where from_ip = 14;
insert into sink19 select a_time from s19_15 where from_ip = 15;

stream  tcp20 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5790' order by current_time;

stream s20_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink20 (	
        a_time 		timestamp
) order by a_time;

insert into s20_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp20;
insert into sink20 select a_time from s20_14 where from_ip = 14;
 
stream  tcp21 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5800' order by current_time;

stream  tcp22 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5810' order by current_time;

stream  tcp23 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5820' order by current_time;

stream s21_extra (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s21 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s22_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink21 (	
        a_time 		timestamp
) order by a_time;

stream sink22 (	
        a_time 		timestamp
) order by a_time;

stream sink23 (	
        a_time 		timestamp
) order by a_time;

insert into s21_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp21;
insert into s21 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s21_extra;

insert into s22_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp22;

insert into sink21 select a_time from s21 where from_ip > 0;
insert into sink22 select a_time from s22_14 where from_ip = 14;
insert into sink23 select current_time from tcp23 where from_ip = 263;

stream  tcp24 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5830' order by current_time;

stream  tcp25 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5840' order by current_time;

stream  tcp26 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5850' order by current_time;

stream s25_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s26_48 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s26_extra (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink24 (	
        a_time 		timestamp
) order by a_time;

stream sink25 (	
        a_time 		timestamp
) order by a_time;

stream sink26 (	
        a_time 		timestamp
) order by a_time;

insert into s25_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp25;

insert into s26_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp26;
insert into s26_48 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s26_extra;

insert into sink24 select current_time from tcp24;
insert into sink25 select a_time from s25_14 where from_ip = 14;
insert into sink26 select a_time from s26_48 where from_ip = 48;

stream  tcp27 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5860' order by current_time;

stream  tcp28 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5870' order by current_time;

stream  tcp29 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5880' order by current_time;

stream s27 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s28_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s29_15 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink27 (	
        a_time 		timestamp
) order by a_time;

stream sink28 (	
        a_time 		timestamp
) order by a_time;

stream sink29 (	
        a_time 		timestamp
) order by a_time;

insert into s27 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp27;

insert into s28_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp28;

insert into s29_15 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp29;

insert into sink27 select a_time from s27 where from_ip > 0;
insert into sink28 select a_time from s28_14 where from_ip = 14;
insert into sink29 select a_time from s29_15 where from_ip = 15;

stream  tcp30 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5890' order by current_time;

stream s30_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink30 (	
        a_time 		timestamp
) order by a_time;

insert into s30_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp30;
insert into sink30 select a_time from s30_14 where from_ip = 14;
 
stream  tcp31 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5900' order by current_time;

stream  tcp32 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5910' order by current_time;

stream  tcp33 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5920' order by current_time;

stream s31_extra (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s31 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s32_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink31 (	
        a_time 		timestamp
) order by a_time;

stream sink32 (	
        a_time 		timestamp
) order by a_time;

stream sink33 (	
        a_time 		timestamp
) order by a_time;

insert into s31_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp31;
insert into s31 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s31_extra;

insert into s32_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp32;

insert into sink31 select a_time from s31 where from_ip > 0;
insert into sink32 select a_time from s32_14 where from_ip = 14;
insert into sink33 select current_time from tcp33 where from_ip = 263;

stream  tcp34 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5930' order by current_time;

stream  tcp35 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5940' order by current_time;

stream  tcp36 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5950' order by current_time;

stream s35_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s36_48 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s36_extra (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink34 (	
        a_time 		timestamp
) order by a_time;

stream sink35 (	
        a_time 		timestamp
) order by a_time;

stream sink36 (	
        a_time 		timestamp
) order by a_time;

insert into s35_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp35;

insert into s36_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp36;
insert into s36_48 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s36_extra;

insert into sink34 select current_time from tcp34;
insert into sink35 select a_time from s35_14 where from_ip = 14;
insert into sink36 select a_time from s36_48 where from_ip = 48;

stream  tcp37 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port5960' order by current_time;

stream  tcp38 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5970' order by current_time;

stream  tcp39 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5980' order by current_time;

stream s37 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s38_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s39_15 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink37 (	
        a_time 		timestamp
) order by a_time;

stream sink38 (	
        a_time 		timestamp
) order by a_time;

stream sink39 (	
        a_time 		timestamp
) order by a_time;

insert into s37 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp37;

insert into s38_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp38;

insert into s39_15 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp39;

insert into sink37 select a_time from s37 where from_ip > 0;
insert into sink38 select a_time from s38_14 where from_ip = 14;
insert into sink39 select a_time from s39_15 where from_ip = 15;

stream  tcp40 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port5990' order by current_time;

stream s40_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink40 (	
        a_time 		timestamp
) order by a_time;

insert into s40_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp40;
insert into sink40 select a_time from s40_14 where from_ip = 14;
 
stream  tcp41 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port6000' order by current_time;

stream  tcp42 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port6010' order by current_time;

stream  tcp43 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port6020' order by current_time;

stream s41_extra (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s41 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s42_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink41 (	
        a_time 		timestamp
) order by a_time;

stream sink42 (	
        a_time 		timestamp
) order by a_time;

stream sink43 (	
        a_time 		timestamp
) order by a_time;

insert into s41_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp41;
insert into s41 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s41_extra;

insert into s42_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp42;

insert into sink41 select a_time from s41 where from_ip > 0;
insert into sink42 select a_time from s42_14 where from_ip = 14;
insert into sink43 select current_time from tcp43 where from_ip = 263;

stream  tcp44 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port6030' order by current_time;

stream  tcp45 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port6040' order by current_time;

stream  tcp46 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port6050' order by current_time;

stream s45_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s46_48 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s46_extra (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink44 (	
        a_time 		timestamp
) order by a_time;

stream sink45 (	
        a_time 		timestamp
) order by a_time;

stream sink46 (	
        a_time 		timestamp
) order by a_time;

insert into s45_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp45;

insert into s46_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp46;
insert into s46_48 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s46_extra;

insert into sink44 select current_time from tcp44;
insert into sink45 select a_time from s45_14 where from_ip = 14;
insert into sink46 select a_time from s46_48 where from_ip = 48;

stream  tcp47 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) source 'port6060' order by current_time;

stream  tcp48 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port6070' order by current_time;

stream  tcp49 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port6080' order by current_time;

stream s47 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s48_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s49_15 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink47 (	
        a_time 		timestamp
) order by a_time;

stream sink48 (	
        a_time 		timestamp
) order by a_time;

stream sink49 (	
        a_time 		timestamp
) order by a_time;

insert into s47 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp47;

insert into s48_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp48;

insert into s49_15 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp49;

insert into sink47 select a_time from s47 where from_ip > 0;
insert into sink48 select a_time from s48_14 where from_ip = 14;
insert into sink49 select a_time from s49_15 where from_ip = 15;

stream  tcp50 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) source 'port6090' order by current_time;

stream s50_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sink50 (	
        a_time 		timestamp
) order by a_time;

insert into s50_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp50;
insert into sink50 select a_time from s50_14 where from_ip = 14;
 
