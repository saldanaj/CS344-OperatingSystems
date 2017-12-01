/*
 Student: Joaquin Saldana
 Class: Operating Systems 1 
 Assignment 3 / smallsh
 
 Description: this is the header file to my smallsh program.  This has all of the function definitions 
 and the parameters required to pass the function.

 This was especially necesasry for my saveProc funciton that saves info on a background process.
 */


#ifndef SMALLSH_H
#define SMALLSH_H


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>


// Returns struct that holds information on background process
struct bgProcess* saveProcess(int spawnpid);

// Catches the stop signal for foreground process only
void catchStopSig(int sigNum);

// Signal handler for the SIGINT signal which will be used to have a foreground child/process
// terminate itself
void fgCatchSig(int sigNum);

// Catches child terminations signals sent by the background process
void bgCatchSig(int sigNum);

// Reada and tokenizes the user's input from the command line
char** parseInput(char* command, int len, char** inputFile, char** outputFile, int* inBackground);

// Function to assist with the "cd" command to change directory
void changeDirectory(char* path);

// Function helps print the exit status of the recently terminated process/child
void getStatus(int exitStatus);

// Function used to execute the user's command entered into the command line
int execCommands(char** args, char* inputFile, char* outputFile, int inBackground);

// Runs the smallsh shell
void runSmallShell();

// Function to exit the shell
void exitSmallShell();

#endif
