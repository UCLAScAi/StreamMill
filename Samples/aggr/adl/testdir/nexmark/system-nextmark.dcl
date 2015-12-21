stream person(person_id int, name char(20), email char(20), credit_card int, city char(20), state char(2), reg_time timestamp) source 'port5556' order by reg_time;

stream bid(auction_id int, price int, bidder_id int, bid_time timestamp) source 'port5557' order by bid_time;

stream auction(auction_id int, item_name char(10), seller_id int, initial_price int, category_id int, expire_date timestamp, input_time timestamp) source 'port5555' order by input_time;

stream query3(tuple_type int, auction_id int, seller_id int, initial_price int, person_id int, name char(20), city char(20), state char(2), in_time timestamp);

stream query4(tuple_type int, auction_id int, initial_price int, category_id int, seller_id int, expire_date timestamp, b_auc_id int, bid_price int, in_time timestamp);
