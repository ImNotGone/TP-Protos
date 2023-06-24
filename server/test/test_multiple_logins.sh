#!/bin/bash

for i in {6..500}
do
    echo -e "user $i\npass $i" | nc -C localhost 8888 &
done
