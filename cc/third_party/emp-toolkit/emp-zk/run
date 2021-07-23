#!/bin/bash
if [ "$1" == "-p1" ] 
then
   shift
   perf record  $1 1 12345 & (sleep 0.1;  $1 2 12345)
elif [ "$1" == "-p2" ] 
then
   shift
   (sleep 0.1; $1 1 12345) & (perf record $1 2 12345)

elif [ "$1" == "-m1" ] 
then
   shift
   valgrind --leak-check=full $1 1 12345 & $1 2 12345
elif [ "$1" == "-m2" ] 
then
   shift
   $1 1 12345 & valgrind --leak-check=full $1 2 12345
elif [ "$1" == "-t1" ] 
then
   shift
   time $1 1 12345 & $1 2 12345
elif [ "$1" == "-t2" ] 
then
   shift
   $1 1 12345 & time $1 2 12345

else 
  (sleep 0.05; $1 1 12345) &  $1 2 12345
fi
