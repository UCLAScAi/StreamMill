stream  tcp1 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time    timestamp
) order by current_time source 'port5600';

stream  tcp2 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) order by current_time source 'port5610';

stream  tcp3 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) order by current_time source 'port5620';

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
) order by current_time source 'port5630';

stream  tcp5 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) order by current_time source 'port5640';

stream  tcp6 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) order by current_time source 'port5650';

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
) order by current_time source 'port5660';

stream  tcp8 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) order by current_time source 'port5670';

stream  tcp9 (
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
	a_time 		timestamp,
	current_time	timestamp
) order by current_time source 'port5680';

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
