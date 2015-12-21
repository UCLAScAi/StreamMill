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

stream s2 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream s14 (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

stream sall (	
	from_ip  	int,
	to_ip   	int,
	s_port 		int,
	d_port		int,
	num_bytes 	int,
        a_time 		timestamp
) order by a_time;

insert into s2 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp1 where from_ip = 2;

insert into s14 select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp2 where from_ip = 14;

insert into sall select from_ip, to_ip, s_port, d_port, num_bytes, current_time from tcp3;

select a_time, from_ip, to_ip from s2;
select a_time, from_ip, to_ip from s14;
select a_time, from_ip, to_ip from sall;
