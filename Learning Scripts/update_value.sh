#!/bin/bash

g++ -std=c++11 -Ofast -march=native -o value update.cpp

./value $1 $2 $3 $4
