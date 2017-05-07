#!/usr/bin/python

# Student: Joaquin Saldana 
# Class: CS344 - Operating Systems 1
# Assignment: Python Exploration 

# Description: This is the code for the python assignment 
# where we are are supposed to create/write 3 files with random 
# lowercase characters and then display/print them to the console, 
# display 2 random integers, and display the product of these 2 
# random integers 

import random 
import string 
import io

# initial loop counter 
a = 0

# variable to hold the lowercase word 
lowerCaseWord = ""

# first loop
while a < 3: 

	b = 0
	
	while b < 10: 
		newChar = random.choice('abcdefghijklmnopqrstuvwxyz')
		lowerCaseWord += newChar
		b+=1 
	
	lowerCaseWord += "\n"	
	
	print(lowerCaseWord) 	

	#create the file name 

	fileName = str("JoaquinSaldana_file" + str(a) + ".txt")
	
	# open the file to "write" 

	newFile = open(fileName, "w")
	
	#write the 10 characters, including the new line onto the file 

	newFile.write(lowerCaseWord)
 
	#reset the string and close the file 

	lowerCaseWord = ""
	
	newFile.close() 	
	
	a += 1 

# call of randomint and assign each to a variable (2 variables) 

firstNumber = random.randint(1, 42)

secondNumber = random.randint(1,42)

# print the variables 

print(firstNumber) 

print("\n")

print(secondNumber) 

# find the product of the two numbers and print it   

sum = firstNumber * secondNumber 

print("\n")

print(str(sum))

 





