#!/usr/bin/python
# Kabiru Ahmed
# 06/28/11
# Script to each of the file in the tests folder as an argument to the program
#
# Usage: ./test.py
# 

import os, sys
from thread import start_new_thread

argument = sys.argv.pop()
path = "../test/messages/"
testFiles = os.listdir(path)

oneFileFolder= "../test/one/"
oneFiles = os.listdir(oneFileFolder)

buffLenFolder = "../test/bufferLen/"
buffLenFiles = os.listdir(buffLenFolder)

compileCommand = "make sendrcv"
cleanUpCommand = "make clean"
firstCommand = "./sendrcv -c "+ path
secCommand = "./sendrcv -c "+ oneFileFolder

command = "./sendrcv -s"


#compile the files
os.system(compileCommand)
i = 0
#run the program against all the files in the tests folder
for file in testFiles:
	i += 1
	print "=========Test ", i
	print "Testing file: " + file
	os.system(firstCommand + file)
	start_new_thread(os.system,(firstCommand + file,))

print("===============================================================")
for file1 in oneFiles:
	for file2 in buffLenFiles:
		i += 1
		print "=========Test ", i
		print "Running: ./sendrcv -s " + file1 +" "+buffLenFolder+""+file2
		os.system(secCommand + file1 +" "+buffLenFolder+""+file2)
		

print("===============================================================")
		
			
# cleanup the mess
os.system(cleanUpCommand)
