#!/bin/bash

for i in {6..500}
do
    ./test_multiple_logins_aux.sh $i | nc -C localhost 8888 &
done
