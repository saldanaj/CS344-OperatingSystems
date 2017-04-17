/*
 Author: Joaquin Saldana
 Class: CS344
 Assignment 2 / adventure.c
 
 This file contains the main method.  This file is intended to act as the UI for the user.  
 This will run the game and request the user select which rooms they wish to visit and 
 if the correct room is choosen will exit the game.  
 
 This program will run multiple threads so the program has the option of giving the time to
 the user.  Furthermore, it keeps track of which rooms the user is entering and the path count.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <assert.h>


# define ALLOWED_ROOMS 7
# define MINIUM_CONNECTIONS 3
# define MAXIMUM_CONNECTIONS 6
# define START_ROOM 1
# define MID_ROOM 2
# define END_ROOM 3

// Just like in the buildrooms.c file, creating a struct to hold all of the room
// data.  Will need to import the file data into the structs and use the
// within the game to record the movements

struct room
{
    char name[40];
    int totalConnections;
    int roomType;
    char fileConnections[6][40];
};

// variable declaration below taken from
// following site: http://www.thegeekstuff.com/2012/05/c-mutex-examples/?refcom

// Multiprocessing variables
pthread_t threads[2];
pthread_mutex_t threadLock[2];
int thread_args[2];


/*
 Function Prototypes
 */

int findConnection(char connects[6][40], int, const char *, struct room * r);
void* writeTimeToFileThread(void*);
void* readTimeFromFileThread(void*);
void getDirectory(const char *directory, char *buffer);
void createRoomStructs(const char *directory, struct room *r, int *roomIndex);

int main()
{
    /*
     Following variables will be used to track the players progress throughout the game
     Due to scope issues, these variables will be declared as global variables
     */
    
    int numberOfPaths = 0; // track the number of rooms visited
    int currentRoom;        // will hold the index of the room currently in
    int roomsVisited[50];   // array of ints that will hold the index of the rooms visited
    int indexReturned;
    
    int choiceFlag;       // will hold the values returned by functions
    
    
    // ints to hold the result of the pthread create function
    int resultIntThread1;
    int resultIntThread2;
    int threadCloseFlag = 0; // used to indicate when to join threads

    
    // Variables used to read the files
    char directoryBuffer[150];
    char * getLine;
    char * temp;
    int nBytes = 100;
    
    // will be used as loop counters and reset often
    int a;
    int b;
    
    
    
    // Dynamically initialize the rooms struct's using malloc
    // and getline pointer
    struct room * sevenRooms = malloc(7 * sizeof(struct room));
    getLine = (char*) malloc(nBytes + 1);
    
    
    for(a = 0; a < 2 ; a++)
    {
        if(pthread_mutex_init(&threadLock[a], NULL) != 0)
        {
            printf("Mutex %d initialization failed\n", a+1);
            free(getLine);
            free(sevenRooms);
            exit(2);
        }
    }
    
    // Now getting ready to start the game
    // Will first initialize the directory so
    // we can read from the files
    getDirectory("./", directoryBuffer);
    
    
    // Now going to populate the information in the our structs
    createRoomStructs(directoryBuffer, sevenRooms, &currentRoom);
    
    
    // This is where the game begins
    // Game will continue until the End Room Type has been reached
    
    // WHILE WE HAVE NOT REACHED THE END ROOM,
    do
    {
      
        
        if (choiceFlag != 5000)
        {
            // if user did not choose to see the "time"
            // stdout to console to inform the user the current room information
            
            printf("CURRENT LOCATION: %s\n", sevenRooms[currentRoom].name);
            
            printf("POSSIBLE CONNECTIONS:");
            
            // Print the current rooms connections by using array to iterate
            // through list of 2d array
            for (a = 0; a < sevenRooms[currentRoom].totalConnections; a++)
            {
                
                if (a + 1 != sevenRooms[currentRoom].totalConnections)
                {
                    // print with connection name with a comma
                    printf(" %s,", sevenRooms[currentRoom].fileConnections[a]);
                }
                else
                {
                    // prints with a period to denote this is the last connection
                    printf(" %s.", sevenRooms[currentRoom].fileConnections[a]);
                }
            }
        }
        
        // Request user next room
        printf("\nWHERE TO? > ");

        // Get the users input
        choiceFlag = getline(&getLine, &nBytes, stdin);
        
        // Read from stdin and store input to char pointer temp.
        // Read up to new line when user hit "ENTER"
        temp = strchr(getLine, '\n');
        
        // Place a null terminator in last position of temp
        // by dereferencing it
        *temp = '\0';
        
        
        if (strcmp(getLine, "time") == 0)
        {
            // threads will be created and join each calling the same
            // mutex lock
            int j,k;
            
            for(j=0; j < 2; j++)
            {
                pthread_create(&threads[j], NULL, writeTimeToFileThread, (void*) &thread_args[j]);
                sleep(1); 
                pthread_create(&threads[j], NULL, readTimeFromFileThread, (void*) &thread_args[j]);
            }
            
            for(k=0; k < 2; k++)
            {
                pthread_join(threads[k], NULL);
            }
            
        }
        
        else if(findConnection(sevenRooms[currentRoom].fileConnections, sevenRooms[currentRoom].totalConnections, getLine, sevenRooms) < 0)
        {
            printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
        }
        // room is in connections move to the room
        else
        {
            // Index where next room is in the struct structure
            indexReturned = findConnection(sevenRooms[currentRoom].fileConnections, sevenRooms[currentRoom].totalConnections, getLine, sevenRooms);
            
            roomsVisited[numberOfPaths] = currentRoom; // add room to visited and increment counter
            
            currentRoom = indexReturned;
            
            numberOfPaths++;
            
            printf("\n"); //print formatting newline;
        }

        
        //break loop when end room is found
        
    } while (sevenRooms[currentRoom].roomType != END_ROOM);
    
    /*
     Exit portion of the program.  We will print to console the stats of the game 
     and deallocation the dynamic memory
     */
    
    // User found room
    // end room found print victory message
    
    roomsVisited[numberOfPaths++] = currentRoom; // add room to visited and increment counter

    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", numberOfPaths - 1);
    
    
    // List Room Names Visited
    for (a = 1; a < numberOfPaths; a++)
    {
        printf("%s\n", sevenRooms[roomsVisited[a]].name);
    }
    

    // Unlock the mutex locks
    
    thread_args[0] = 1;
    thread_args[1] = 1;
    
    pthread_mutex_unlock(&threadLock[0]);
    pthread_mutex_unlock(&threadLock[1]);
    
    pthread_mutex_destroy(&threadLock[1]);
    pthread_mutex_destroy(&threadLock[1]);

    
    /*
     Code to deallocate the memory from the heap of the struct rooms
     */

    free(getLine);
    free(sevenRooms);
    
    return 0;
}

/*
 ==============================================================================================
 FUNCTION DECLARATIONS AND INITIALIZATIONS 
 ==============================================================================================
 */


/*
 Function: getDirectory(const char *, char * )
 Parameter: const char * directory
 Parameter: char * buffer
 Description: Function finds the most recent directory to work from using the stat function 
 and reading the directory info using the dirent struct and it's properties/variables
 Source: http://stackoverflow.com/questions/12991334/members-of-dirent-structure
 */

void getDirectory(const char *directory, char *buffer)
{
    // Variables which hold the new and max time
    int maxTime = -1;
    int newTime = 0;
    
    // Struct to store the file information
    struct stat directoryStats;
    
    // Directory stream file pointer
    DIR *newDirectory;
    
    // Will be used to read the directories information
    struct dirent *dirInfo;
    
    // Open the directory the program is running from
    newDirectory = opendir(directory);
    
    // Read the directories information
    while ((dirInfo = readdir(newDirectory)))
    {
        // Compare the strings up to 19 characters long
        if (strncmp(dirInfo->d_name, "saldanaj.rooms", 13) == 0)
        {
            // Retrieve the stats of the directory
            stat(dirInfo->d_name, &directoryStats);
            
            // assigning the new time to the variable
            newTime = (int)directoryStats.st_mtime;
            
            // if the newTime is greater than the current maxTime
            // then we need to re-assign and change the buffer
            if (newTime > maxTime)
            {
                strcpy(buffer, dirInfo->d_name);
                
                maxTime= newTime;
            }
        }
    }
    
    closedir(newDirectory);
    
}

/*
 Function: getDirectory(const char *, char * )
 Parameter: const char * directory
 Parameter:
 Description: Function reviews the directory files and reads from the populating the information 
 in the struct created to hold the file information for the game.
 Source: https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html
 */

void createRoomStructs(const char *directory, struct room *r, int *roomIndex)
{
    
    // Buffer for reading file
    char buffer[100];
    
    // Used to index a certain char like a new line or null terminator
    char *charIndex;
    
    // Indicate colon position
    int colon = 0;
    
    // General loop variable
    int a;
    
    // Struct to store directory information
    struct dirent *dirInfo;
    
    // New directory stream object
    DIR *newDirectoryStream;
    
    // New File stream object
    FILE *newFile;
    
    
    // give each room an outgoing count of 0
    for (a = 0; a < ALLOWED_ROOMS; a++)
    {
        r[a].totalConnections = 0;
    }
    
    a = 0;
    
    // If were able to open the directory
    
    if((newDirectoryStream = opendir(directory)))
    {
        while((dirInfo = readdir (newDirectoryStream)) != NULL)
        {
            // copy the directory name to the buffer to test to see if it's the file
            strcpy(buffer, dirInfo->d_name);
            
            // Checking the length of the string making sure we are not
            // reading the contents of ./ or ../
            if(strlen(buffer) > 3)
            {
                // copy the full path of the file to the buffer
                sprintf(buffer, "./%s/%s", directory, dirInfo->d_name);
                
                // Open the file for reading
                newFile = fopen(buffer, "r");
                
                while(fgets(buffer, 100, newFile)!=NULL)
                {
                    
                    // Move pointer up until the new line character
                    charIndex = strchr(buffer, '\n');
                    
                    // dereference the pointer and replace the char with the null
                    // terminator
                    *charIndex = '\0';
                    
                    // Move the pointer to the : so we can read the rest of the
                    // information
                    charIndex = strchr(buffer, ':');
                    
                    // get index of colon by taking the offset from buffer
                    colon = charIndex - buffer;
                    
                    // Here we being to capture the room information
                    // this includes, names, connections, and room type
                    if (strncmp(buffer, "ROOM NAME", colon) == 0)
                    {
                        strcpy(r[a].name, charIndex + 2);
                    }
                    else if (strncmp(buffer, "CONNECTION", colon - 2) == 0)
                    {
                        strcpy(r[a].fileConnections[r[a].totalConnections++], charIndex + 2);
                    }
                    else if (strncmp(buffer, "ROOM TYPE", colon) == 0)
                    {
                        strcpy(buffer, charIndex + 2);
                        
                        if (strcmp(buffer, "END_ROOM") == 0)
                        {
                            r[a].roomType = END_ROOM;
                        }
                        else if (strcmp(buffer, "START_ROOM") == 0)
                        {
                            r[a].roomType = START_ROOM;
                            
                            // This will assign the struct index of the file/room
                            // of type start room
                            *roomIndex = a;
                        }
                        else
                        {
                            r[a].roomType = MID_ROOM;
                        }
                        
                    }
                    
                }
                
                // Close file
                fclose(newFile);
                
                // Next struct room
                a++;
            }
        }
        
        // Close directory
        closedir(newDirectoryStream);
    }
}

/*
 Function: findConnection 
 Parameters - 2D array holding the rooms connections 
            - int totalConnects - will be used as loop counter for total connections room has 
            - connectName - name of connectiong we are trying to find
 Description: finds the connection the user is trying to connect to, if the input is incorrect 
              then a negative int is returned signaling the room was not found
 */

int findConnection(char connects[6][40], int totalConnects, const char *connectName, struct room * r)
{
    int a;
    int b;
    
    // Iterate through the 2D array to find the connection
    for (a = 0; a < totalConnects; a++)
    {
        if (strcmp(connects[a], connectName) == 0)
        {
            // Connection was found and returning index
            // of the struct in the struct array
            
            for(b = 0; b < ALLOWED_ROOMS; b++)
            {
                if(strcmp(r[b].name, connectName) == 0)
                {
                    return b;
                }
            }
        }
    }
    
    return -5;
}


/*
 Function: writeTimeToFile(int) 
 Parameter: int - flag to indicate whether to break, begin, or wait 
 Description: this function will write the system time to a file called currentTime.txt 
 in the same directory as the program is running
 Source: http://stackoverflow.com/questions/7411301/how-to-introduce-date-and-time-in-log-file
 */

void*  writeTimeToFileThread(void* argument)
{
    int passed_in_value = *((int *) argument);
    
    // input and output buffer
    char buffer[100];
    
    // variable to hold the time
    time_t tempTime;
    
    // struct to hold the time parameters
    struct tm *timeinfo;
    
    // new file stream
    FILE *newTimeFile;
    
    // locking mutex
    pthread_mutex_lock(&threadLock[0]);
    
    // Open file, if not there it will create the file
    newTimeFile = fopen("./currentTime.txt", "w");
    
    // Assign the current time to the variable
    time ( &tempTime );
    
    // Complete the struct
    timeinfo = localtime ( &tempTime );
    
    // Send the information to the char buffer array
    // in anticipation for writing it
    strftime(buffer, 50, "%I:%M%p, %A, %B %d, %Y", timeinfo);
    
    // Format the time
    if (timeinfo->tm_hour < 10 || (timeinfo->tm_hour > 12 && timeinfo->tm_hour < 22))
    {
        memmove(buffer, buffer+1, strlen(buffer));
    }
    
    // Write to file
    fputs(buffer, newTimeFile);
    
    // Close file
    fclose(newTimeFile);
    
    // Attempting to acquire the lock
    pthread_mutex_unlock(&threadLock[0]);
    
    sleep(1);
    
    return NULL;
    
}

/*
 Function: readTimeFromFileThread(int)
 Parameter: int - flag to indicate whether to break, begin, or wait
 Description: this funtion will read the time from the file created by the 
 writeTimeToFileThread
 */

void* readTimeFromFileThread(void* argument)
{
    int passed_in_value = *((int *) argument);
    
    // Variable declarations needed to read from file
    char buffer[100];
    FILE *newTimeFile;
    
    
    // Locking mutex
    pthread_mutex_lock(&threadLock[0]);
    
    // Open the file currently holding the time
    newTimeFile = fopen("./currentTime.txt", "r");
    
    // Read from file
    fgets(buffer, 50, newTimeFile);
    
    // Print to console
    printf("\n%s\n", buffer);
    
    // Close file
    fclose(newTimeFile);
    
    // Unlock the mutex and sleep
    pthread_mutex_unlock(&threadLock[0]);
    
    sleep(1);
    
    
    return NULL;
}
















