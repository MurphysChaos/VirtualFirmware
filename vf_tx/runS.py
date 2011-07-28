#!/usr/bin/python
# Kabiru Ahmed
# 07/08/11
# The scrip is created to run parallel with runS.py. It is used to send a buffer and runS.py will receive it
# Run runS.py first before running this script
#
# Usage: ./runC.py
# 

import os, sys, time

argument = sys.argv.pop()

# This is where the total running time of the script is set
totalRunTime = 2.0

startTime = time.time()

command = "./sendrcv -s"
i = 0
# Forever Loop
while True:
	i += 1
	print "=========Test ", i
	os.system(command)
	if time.time() - startTime >  totalRunTime:
		break
