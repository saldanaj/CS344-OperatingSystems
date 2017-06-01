/*
 Student: Joaquin Saldana 
 Assignment 4 / CS344 Operating Systems 1 
 
 Description: This is the keygen.c program where it takes a numerical argument and creates a key of that length.
 Ex: keygen 256 > myKey // will create a 256 key and write it to the file "myKey"
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char * argv[])
{
    // check to ensure the user entered a numerical amount
    // in the command line
    if(argc < 2)
    {
        fprintf(stderr, "Incorrect syntax.  Correct syntax: keygen 20\n");
        exit(1);
    }

    int length = atoi(argv[1]);
    
    char keyChars[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    
    char key[length+1];
    
    srand(time(0));
    
    int a;
    
    for(a = 0; a < length; a++)
    {
        key[a] = keyChars[rand() % 27];
    }
    
    key[length] = '\0';
    
    printf("%s\n", key);
    
    return 0;
}
