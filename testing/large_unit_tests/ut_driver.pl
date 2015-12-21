#!/usr/bin/perl

require large_ut;

print "Testing adding a large number of users...\n";
large_ut::test_add_multiple_users();
print "(OK) Done testing adding a large number of users...\n";

print "Testing adding a large number of streams for each user...\n";
large_ut::test_add_multiple_streams();
print "(OK) Done testing adding a large number of streams for each user...\n";

print "Testing adding a large number of queries for each user...\n";
large_ut::test_add_multiple_queries();
print "(OK) Done testing adding a large number of queries for each user...\n";

print "Testing activating a large number of streams for each user...\n";
large_ut::test_activate_streams();
print "(OK) Done testing activating a large number of streams for each user...\n";

print "Testing activating a large number of queries for each user...\n";
large_ut::test_activate_queries();
print "(OK) Done testing activating a large number of queries for each user...\n";

print "Testing monitoring all buffers for each user...\n";
# large_ut::test_monitor_all_ip();
print "(OK) Done testing monitoring all buffers for each user...\n";

print "Testing sending data...\n";
# large_ut::test_send_data();
print "(OK) Done testing sending data...\n";
