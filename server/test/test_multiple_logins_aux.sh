#!/bin/bash
echo -e "capa\nuser $1\npass $1\n"
sleep $(($RANDOM % 11))
echo -e "capa\nstat\nlist\nretr 1\nnoop\ndele 1\nrset\nquit\n"
