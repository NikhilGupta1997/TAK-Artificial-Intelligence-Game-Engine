#!/bin/bash

## This is to run the first program main1.cpp

if [ -e "code" ]; then
    ./code $1
else
    echo "First run compile.sh to compile the code"
fi
