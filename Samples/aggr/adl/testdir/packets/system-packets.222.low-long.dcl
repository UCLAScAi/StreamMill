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

stream s2_14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s3_263 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s_extra (	
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


insert into s2_14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp2;

insert into s_extra select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp3;
insert into s3_263 select from_ip, to_ip, s_port, d_port, num_bytes, a_time from s_extra;

insert into sink1 select current_time from tcp1;
insert into sink2 select a_time from s2_14 where from_ip = 14;
insert into sink3 select a_time from s3_263 where from_ip = 263;
