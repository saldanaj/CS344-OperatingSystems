/*
Author: Joaquin Saldana 
Class: CS344
Assignment 2 / buildrooms.c
 
This is the buildrooms.c file.  In this file we are creating a series of files that hold descriptions of the in-game rooms 
and how the rooms are connected.
 
 The program has declared a struct of type room that will hold all of the data for each of the room.  
 Once the structs are completed and initialized, we will then assign them room names at random, room types, 
 and connections.  Once completed, we will print all of the data to files in the directory path, creating files 
 with the names of the rooms randomly selected.  Lastly, we will create all of the files and input the struct 
 information into each of the files.
 */


// included libraries
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>


// CONSTANT GLOBAL VARIABLES
# define TOTAL_ROOMS 10
# define ALLOWED_ROOMS 7
# define MINIUM_CONNECTIONS 3
# define MAXIMUM_CONNECTIONS 6


// Constant array with the room names
const char* roomNames[TOTAL_ROOMS] = {  "Basement",
                                        "Kitchen",
                                        "Living_Room",
                                        "Star_Wars_Collectible_Room",
                                        "Game_Room",
                                        "Master_Bedroom",
                                        "Garage",
                                        "Kids_Room",
                                        "Vader_Room",
                                        "Office"};

/*
 Struct that will hold the room information 
 1. Room name 
 2. Room type 
 3. number of connections 
 4. pointer of type struct that holds the connecting rooms
 */
struct room
{
    const char *room_Name;
    const char *room_Type;
    int numberOfConnections;
    int connectionIndex;
    int * connectionsArray;
};

// FUNCTION PROTOTYPES
void createRoomNames(struct room *, char[], int[]);
void createConnections(struct room * , int [], char[]);
void writeTypesToFile(char [], struct room *);


// MAIN METHOD

int main()
{
    // SEED THE RANDOM NUMBER GENERATOR
    srand(time(NULL));
    
    /*
       First we'll begin by creating the directory
       We will initialize the directory arrayname in order to pass it to the
       mkdir() function.  Then we will check to make sure the function
       properly creates the directory by checking it's return integer
       in an if statement.  If something went wrong we'll exit the program
       with a status int other than 0 (in this case 2)
     */
    
    // char array to hold the directory name
    char directoryName[50];
    
    // ensure the array is filled with null chars to avoid any issues
    memset(directoryName, '\0', 50);
    
    // create the directory name by concatenating the process id with the
    // directory name
    sprintf(directoryName, "saldanaj.rooms.%d", getpid());
    
    // create the directory in the same directory where the program is stored
    // this should work either locally or at the OSU EOS server
    int directoryStatus = mkdir(directoryName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    
    // check if the directory was succesfully created
    // if not exit with a bad status code (in this case 2)
    if(directoryStatus != 0)
    {
        printf("There was a problem creating the directory");
        exit(2);
    }
    
    /*
     Now we will initialize dynamically an array of type struct room that will hold the 
     seven rooms that we will select randomly.  
     
     Afterwards we will dynamically allocate room for the array that will hold the 
     connections of the files
     
     */
    struct room* sevenRoomArray = malloc(7 * sizeof(struct room));
    
    int a;
    int b;
    
    
    // for loop to dynamically allocate the memory for each of the rooms int array
    // that will hold the connections to the rooms
    for(a= 0; a < ALLOWED_ROOMS; a++)
    {
        // create a random number of connections
        sevenRoomArray[a].numberOfConnections = rand() % 4 + 3;
        sevenRoomArray[a].connectionIndex = 0;
        
        int * tempIntArray = malloc(sevenRoomArray[a].numberOfConnections* sizeof(int));
        
        sevenRoomArray[a].connectionsArray = tempIntArray;
        
        for(b = 0; b < sevenRoomArray[a].numberOfConnections; b++)
        {
            sevenRoomArray[a].connectionsArray[b] = -1;
        }
        
    }
    
    int roomsCreated[7];
    
    
    //  Call function to randomly create rooms
    createRoomNames(sevenRoomArray, directoryName, roomsCreated);
    
    // Call function to randomly create connections
    createConnections(sevenRoomArray, roomsCreated,directoryName);

    // Call function to write the file type to the files
    writeTypesToFile(directoryName, sevenRoomArray);
    
    
    /*
     Now we will begin deallocating and free all of the memory in the heap.
     */
    
    for(a = 0; a < ALLOWED_ROOMS; a++)
    {
        free(sevenRoomArray[a].connectionsArray);
    }
    
    
    free(sevenRoomArray);
    
    return 0;
}

/* ====================================================================================================  */

/*
 Function: createRoomNames(struct room *, char [])
 Parameter: char * - a pointer to a char array/address 
 Returns: N/A
 Description: this function creates the files in the new directory and 
                prints onto the file the file name
 */

void createRoomNames(struct room * rArray, char * dPath, int * rCreated)
{
    
    /*
     Next we will create the files in the directly created in the previous
     section of code.  We will first create the file names.  To do this we
     are doing the following
     1. creating a random number between 0 and 9
     2. before adding that number to an int array, we will
     be checking if the number already exists
     3. we will then associate that number with the room name in the
     roomName array and create the file with the room name
     */
    

    // char array to hold the path name
    char filepath[75];

    int roomsUsed[ALLOWED_ROOMS];

    // ensure the array has all null terminators to avoid any
    // problems when reading the array and accidentally going
    // out of bounds

    memset(filepath, '\0', 75);

    memset(roomsUsed, -1, 10);

    int i = 0;

    bool isRoomUnique = false;
    bool numberGenerated = false;
    
    while(i < ALLOWED_ROOMS)
    {
        int randomNumber = rand() % 10;
        
        // while loop to continue generating random numbers
        // until the number is a unique number
        
        while(isRoomUnique == false)
        {
            // traverse the array to check if the number
            // is already in the array
            
            int j = 0;
            
            while(j < ALLOWED_ROOMS)
            {
                if(randomNumber == roomsUsed[j])
                {
                    // generate a new number and set the boolean value
                    // to true so we re-enter the for loop
                    
                    randomNumber = rand() % 10;
                    numberGenerated = true;
                }
                
                j++;
            }
            
            // check if the boolean value is still false
            // which is so, means we can exit the while
            // loop used to re-generate random numbers
            
            if(numberGenerated == false)
            {
                isRoomUnique = true;
            }
            
            numberGenerated = false;
        }
        
        // random number is unique placing the number in an
        // array for later use.  Naturally the first room generated
        // will be the start room and the last room generated
        // will be the exit room
        roomsUsed[i] = randomNumber;
        
        rArray[i].room_Name = roomNames[randomNumber];
        
        // creating the filepath name
        sprintf(filepath, "%s/%s", dPath, roomNames[randomNumber]);
        
        FILE * newFile;
        
        // Create the file in the directory
        newFile = fopen(filepath, "a");
        
        // Printing the room name to the file
        fprintf(newFile, "ROOM NAME: %s\n", roomNames[randomNumber]);
        
        // closing the newly  created file
        fclose(newFile);
        
        // reset the boolean variable
        isRoomUnique = false;
        
        // increment the loop counter
        i++;
    }
    
    int z;
    
    for(z = 0; z < ALLOWED_ROOMS; z++)
    {
        rCreated[z] = roomsUsed[z];
    }

}


/*
 Function: createConnections(struct room *)
 Parameter: struct room * - pointer to array of stype struct room
 Returns: N/A
 Description: this function creates the connections between the rooms.  It will iterate through
 the structs filling in the connections in the connections array.
 */

void createConnections(struct room * rArray, int * rCreated, char * dPath)
{
    int a;
    int b;
    int c;
    
    // this for loop is used to iterate through all the structs
    for(a = 0; a < ALLOWED_ROOMS; a++)
    {
        // this loop is to continue adding to the array while
        // the connection index is less than the number of connections
        while(rArray[a].connectionIndex < rArray[a].numberOfConnections)
        {
            // generate a random index to use on the rCreated array
            int randomIndex = rand() % 7;
            
            // booleans to help test
            bool isIndexUnique = false;
            
            b = 0;
            
            // loop that will continue until we have a unique number/index
            while(isIndexUnique == false)
            {
                
                while(b < rArray[a].numberOfConnections)
                {
                    // checking if the number is already in the array
                    if(rCreated[randomIndex] == rArray[a].connectionsArray[b])
                    {
                        randomIndex = rand() % 7;
                        b = 0;
                    }
                    else if(strcmp(rArray[a].room_Name, roomNames[rCreated[randomIndex]]) == 0)
                    {
                        // checking if the name is equal to the struct room name
                        randomIndex = rand() % 7;
                        b = 0;
                    }
                    else
                    {
                        b++;
                    }
                }
                
                // else the number is unique and we can exit the while loop
                isIndexUnique = true;
            }
            
            // add the number to the int array we are currently on "rArray[a]"
            rArray[a].connectionsArray[rArray[a].connectionIndex] = rCreated[randomIndex];
            
            // increment the index counter
            rArray[a].connectionIndex++;
            
            // now here is where we connect it to each other
            
            for(c = 0; c < ALLOWED_ROOMS; c++)
            {
                if(strcmp(rArray[c].room_Name, roomNames[rCreated[randomIndex]]) == 0)
                {
                    rArray[c].connectionsArray[rArray[c].connectionIndex] = rCreated[a];
                    rArray[c].connectionIndex++;
                }
                
            }
            
        }
        
        /*
         Now we will write the connections to the respective files
         */
        
        char filepath[75];
        memset(filepath, '\0', 75);
    
        
        for(b = 0; b < rArray[a].numberOfConnections; b++)
        {
            sprintf(filepath,"%s/%s", dPath, rArray[a].room_Name);
            FILE *newFile;
            newFile = fopen(filepath, "a");
            fprintf(newFile, "CONNECTION %d: %s\n", b + 1, roomNames[rArray[a].connectionsArray[b]]);
            fclose(newFile);
            
        }
    }
}


void writeTypesToFile(char *dPath, struct room * rArray)
{
    int a;
    
    /*
     Next we will assign the room type to each of the struct rooms in the sevenRoom struct array
     Naturally the first room will be the start room, the last room will be the end room
     and each rooms in between will be mid rooms
     */
    
    // First room is the start room
    a = 0;
    rArray[a].room_Type = "START_ROOM";
    
    // Last room is the end room
    a = 6;
    rArray[a].room_Type = "END_ROOM";
    
    // Rooms in the middle will be assigned as mid rooms
    for(a = 1; a < 6; a++)
    {
        rArray[a].room_Type = "MID_ROOM";
    }
    
    char filepath[75];
    memset(filepath, '\0', 75);
    
    // start printing the room types to the file
    for(a = 0; a < ALLOWED_ROOMS; a++)
    {
        sprintf(filepath,"%s/%s", dPath, rArray[a].room_Name);
        
        FILE *newFile;
        
        newFile = fopen(filepath, "a");
        
        fprintf(newFile, "ROOM TYPE: %s\n", rArray[a].room_Type);
        
        fclose(newFile);
    }
}









