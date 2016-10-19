#!/bin/bash

#################################################################################
#                                                                               #
#       This is the script used to kill all running game instances.             #
#                                                                               #
#       Copyright (c) NIKHIL'S CODE                                             #
#       @authors Nikhil Gupta                  									#
#                                                                               #
#                                                                               #
#################################################################################

# Find all the qemu processes and store them in a variable
PROCESS_NUMBER=`ps aux | grep -e "python" -e "client" | awk '{print$2}'`

# Run a command to kill all the existing QEMU instances
kill -9 $PROCESS_NUMBER
