#!/bin/bash
#
# An "alarm" script that can verify whether there is a process
# running for your favorite program.
#
# For test purposes set sleep to a small value
# Actual script would probably run silently as
# long as the process was alive (and not echo
# the "all is well" message).
#
#
while [ 1 ]
do
  sleep 1
  PIDOFZ=$(./esl_start -p 5444)
  if [ x$PIDOFZ == x"" ] 
  then
      echo "This is where you would log the event or whatever..."
      exit
  else
      # For debug purposes print the heartbeat
      echo "Process ID = $PIDOFZ, all is well."
  fi
done
