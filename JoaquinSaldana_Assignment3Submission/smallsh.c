/*
 Student: Joaquin Saldana 
 Class: CS344 - Operating Systems 1 
 Assignment 3 - smallsh
 
 Description:  This is the c file that will define the functions necessary to run my
 own shell 
 
 */


#include "smallsh.h"


// DEFINING CONSTANT VARIABLES

#define ERROR_NO -1;
#define CHILD_PROCESS 0
#define MAX_PROCESSES 50
#define MAX_CMD_LENGTH 2048
#define TRUE 1
#define FALSE 0

//=============================================================================


// Struct that stores info on background process (one for each)

struct bgProcess
{
    //background process PID
    pid_t bgPID;
    
    // to know if process is still running or has terminated
    int active;

    // exit status
    int status;
};

//=============================================================================

/* GLOBAL VARIABLES */

// bool value will be changed if user hits CTRL-Z (SIGSTP) signal
bool foregroundOnlyMode = false;

// will be used to track the current foregrounds PID
pid_t fgPID = -1;

//array to hold the PID's of background processes
struct bgProcess* bgProcs[MAX_PROCESSES];


//=============================================================================

/* FUNCTIONS */

/*
 Function: saveProcces 
 Parameters: (int) / spawnpid
 Return: Pointer to strut of type bgProcess
 Description: saves a background process in a array.  This array will be occasionaly check to see if 
 processes inside have completed
 */

struct bgProcess* saveProcess(pid_t spawnpid)
{
    int saved = FALSE;
    
    int count = 0;
    
    //create a new background process container and initialize its data variables
    
    // Initializes a new struct to store the background process
    // It also initializes its variables
    
    struct bgProcess* bgProc = malloc(sizeof(struct bgProcess));
    
    bgProc->bgPID = spawnpid;
    
    bgProc->active = TRUE;
    
    // Add the process to the bgProcs global array
    while(saved == FALSE)
    {
        if(bgProcs[count] == NULL) //if an empty spot is found
        {
            bgProcs[count] = bgProc; //save the process information
            saved = TRUE;
        }
        
        count++;
    }
    
    return bgProc;
}

/*
 Function: catchStopSig
 Parameter: int
 Return: N/A
 Description: this is the signal handler for the SIGSTP / CTRL-Z if the user want's to disallow
 background mode
 */

void catchStopSig(int sigNum)
{
    // checking if the foreground boolean flag
    
    if(foregroundOnlyMode == false)
    {
        char * mesasge = ("\nEntering foreground-only mode (& is now ignored)\n");
        write(STDOUT_FILENO, mesasge, 50);
        foregroundOnlyMode = true;
    }
    else
    {
        // Exiting foreground only mode
        
        char * message = "\nExiting foreground-only mode\n";
        write(STDOUT_FILENO, message, 50);
        foregroundOnlyMode = false;
    }
}


/*
 Function: fgSignalHandler
 Parameters: int 
 Return: N/A 
 Description: function kills foreground process and outputs the signal number 
 that killed/terminated the process
 */

void fgCatchSig(int sigNum)
{
    fprintf(stdout,"Terminated by signal %d\n",sigNum);
    fflush(stdout);
    kill(fgPID,SIGKILL);
}

/*
 Function: bgCatchSig 
 Parameter: int 
 Return: N/A 
 Description: called when a child terminates, but does not affect foreground processes.  If the child is a background 
 process, then the data variables are updated to reflect the termination.
 
 */

void bgCatchSig(int sigNum)
{
    int exitStatus = 0;
    
    int i;
 
    // Searching array for the background process that already completed
    for(i = 0; i < MAX_PROCESSES; i++)
    {
        if(bgProcs[i] != NULL)
        {
            // Get the PID
            pid_t pid = waitpid(bgProcs[i]->bgPID, &exitStatus, WNOHANG);
        
            if(pid > 0)
            {
                // Change to false to indicte the process is not running anymore
                bgProcs[i]->active = FALSE;
                
                // Assign it the exit status
                bgProcs[i]->status = exitStatus;
                
                // Kill whatever is left of the process
                kill(bgProcs[i]->bgPID,SIGKILL);
            }
        }		
    }
}


/*
 Function: parseInput 
 Parameter: char* command, int length, char ** inputFile, char** outputFile, int * isBackground 
 Return: char ** 
 Description: function parses the input entered by the user on the command line on the shell
 */
char** parseInput(char* command, int len, char** inputFile,char** outputFile, int* inBackground)
{
    
    // bool flag to determine if output needs to be redirected
    int outputRedirection = FALSE;
    
    // bool flag to determine if input needs to be redirected
    int inputRedirection = FALSE;
    
    // set a delimeter variable
    char* delimeter = " \n";
    
    // double pointer to array of commands which will be passed to execvp
    char** args;
    
    // initialize on the heap
    args = malloc(len * sizeof(char*));
    
    assert(args != NULL);
    
    int count = 0;
    
    int i;
    
    // initialize the dynamic array to all NULL
    for (i = 0; i < len; i++)
    {
        args[i] = NULL;
    }
    
    // collect the entire string up to the new line char
    
    char* token = strtok(command, delimeter);
    
    // while we continue to have strings
    // in while loop, are nested if statements that checks for input/output redirection,
    // if the process can be runned in the background
    while (token != NULL)
    {
        
        // checks for a comment symbol in the token
        regex_t regex;
     
        regcomp(&regex,"^#",0);
        
        int isComment = regexec(&regex,token,0,NULL,0);
        
        if(outputRedirection == TRUE)
        {
            // user wants to redirect output
            outputRedirection = FALSE;
            
            // file name is assigned to the outputfile variable
            *outputFile = token;
        }
        else if(inputRedirection == TRUE)
        {
            // user wants to redirect input
            inputRedirection = FALSE;
         
            // file name is assigned to the inputFile variable
            *inputFile = token;
        }
        else if(isComment == 0) // a commment is present
        {
            // a comment is present
            // ignore the entire input line and just break from the while loop
            args[count] = '\0';
         
            break;
        }
        else if(strcmp(token,"<") == 0)
        {
            // redirection special character is present so we set the input redirection flag
            inputRedirection = TRUE;
        }
        else if(strcmp(token,">") == 0)
        {
            // output redirection spcial character is prsent so we set the output redirection flag
            outputRedirection = TRUE; //set the output redirection flag
        }
        else if(strcmp(token,"&") == 0)
        {
            // indicates if the process should be run in the background, however
            // will only work if the foregroundOnlyMode variable is set to false
            // else it's been disabled
            if(foregroundOnlyMode == false)
            {
                *inBackground = TRUE; //set the background process flag
            }
        }
        else //save the current token only if it is a valid unix command or a file name. ignore all redirection and background process symbols
        {
            // if it's a legit command, save it to the args array we are going to return
            args[count++] = token;
        }
        
        token = strtok(NULL, delimeter); //get the next token
    }
    
    // return the args array as an array of "strings" which will be used for the execvp function
    return args;
}



/*
 Function: changeDirectory 
 Parameters: char * path - holds path to directory 
 Return: N/A
 Description: function will change the current working directory
 
 */

void changeDirectory(char* path)
{
    int directory;
    
    
    // if the user simple enters "cd" in the command line
    if (path == NULL)
    {
        // go to the "HOME" environment variable directory
        directory = chdir(getenv("HOME"));
        
        // if an error occured
        if(directory == -1)
        {
            perror("Could not change directory:");
            return;
        }
    }
    else
    {
        // change to path found in argument
        directory = chdir(path);
        
        // Check for error
        if(directory == -1)
        {
            perror("Could not change directory:");
            return;
        }	
    }
}


/*
 Function: getStatus
 Parameters: int 
 Return: N/A (simple print statement) 
 Description: prints to terminal the exit status of the most recent terminated process
 */

void getStatus(int exitStatus)
{
    fprintf(stdout,"Exit value %d\n",exitStatus);
    fflush(stdout);
}


/*
 Function: execCommands
 Parameters: char ** args doub-pointer, char * to inputfile, char * to outputFile, true/false if background
 Returns: int - representing exit status 
 Description: function executes commands by using fork() and exec() functions.  This includes changing I/O redirection 
 and background processes.
 
 Reference Links: http://stackoverflow.com/questions/13667364/exit-failure-vs-exit1

 */

int execCommands(char** args, char* inputFile, char* outputFile, int inBackground)
{
    // Function variables
    
    // save stdin and stdout current direction
    int stdOut = dup(STDOUT_FILENO);
    int stdIn = dup(STDIN_FILENO);
    
    int process_status;
    
    
    // error checking if stdIn or stdOut were
    // incorrectly set for dup() function
    if(stdOut < 0 || stdOut < 0)
    {
        perror("Error with redirection");
        exit(EXIT_FAILURE);
    }
    
    // fork to a new process
    pid_t spawnpid = fork();
    
    // if it's the child
    if(spawnpid == CHILD_PROCESS)
    {
        //set file descriptors
        int inFD = -1;
        int outFD = -1;
        
        // if the process needs to run in the background
        // b/c user entered "&"
        if(inBackground == TRUE && foregroundOnlyMode == FALSE)
        {
            // if no output file
            if(outputFile == NULL)
            {
                // output to /dev/null, write only
                outFD = open("/dev/null",O_WRONLY);
                
                // Error checking if open caused a failure
                if(outFD == -1)
                {
                    perror("Error writing to console");
                    
                    exit(EXIT_FAILURE);
                }
                
                // redirect output
                dup2(outFD,STDOUT_FILENO);
                
                // close file descriptor
                close(outFD);
            }
            
            // if input file is NULL
            // meaning no input redirection
            if(inputFile == NULL)
            {
                // input to /dev/null, read only
                inFD = open("/dev/null",O_RDONLY);
                
                // error checking
                if(inFD == -1)
                {
                    perror("Error reading from console");
                    exit(EXIT_FAILURE);
                }
                
                // redirect input
                dup2(inFD,STDIN_FILENO);
                
                close(inFD);
            }
        }
        else
        {
            // process intends to run in the foreground
            
            if(outputFile != NULL)
            {
                // output redirection
                
                // open file or create a new one
                outFD = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, 0777);
                
                // error handling
                if(outFD == -1)
                {
                    perror("Error opening/creating file");
                    exit(EXIT_FAILURE);
                }
                
                dup2(outFD,STDOUT_FILENO);
                
                close(outFD);
            }
            
            if(inputFile != NULL)
            {
                
                // input redirection
                
                // open the file
                inFD = open(inputFile, O_RDONLY, 0777);
             
                // error handling
                if(inFD == -1)
                {
                    perror("Error reading file");
                    exit(EXIT_FAILURE);
                }
                
                dup2(inFD,STDIN_FILENO);
                
                close(inFD);
            }
        }
        
        // execute the command running execvp
        execvp(args[0], args);
        
        // in case error executing command
        perror("Error");
        
        exit(EXIT_FAILURE);
        
    }
    else if(spawnpid > CHILD_PROCESS)
    {
        // if it's the parent
        
        pid_t exitpid;
        
        if(inBackground == TRUE && foregroundOnlyMode == FALSE) //if child is a background process
        {
            // save child PID to array to check on it later
            struct bgProcess* process = saveProcess(spawnpid);
            
            // print out the message informing the user the command/process has been in the
            // background and the PID
            fprintf(stdout,"Background pid is %d\n",spawnpid);
            
            fflush(stdout);
        
        }
        else
        {
            // child is a foreground process
            fgPID = spawnpid;
        
            // wait for completion of child process
            exitpid = waitpid(spawnpid,&process_status,0);
        }
        
        // stdin and out are reset
        
        if(inputFile != NULL)
        {
            dup2(stdIn,STDIN_FILENO);
        }
        
        if(outputFile != NULL)
        {
            dup2(stdOut,STDOUT_FILENO);
        }
        
        fflush(stdout);
        
        // get the exit status
        return WEXITSTATUS(process_status);
    
    }
    else
    {
        perror("Error executing command");
        
        exit(EXIT_FAILURE);
    }
}


/*
 Function: runSmallShell() 
 Parameters: N/A
 Return: N/A
 Description: This function runs the small shell and manages various operations
 */

void runSmallShell()
{
    // exit code
    int exitStatus = 0;
    
    int i;
    
    
    // initialize the global array for the background processes
    for(i = 0; i < MAX_PROCESSES; i++)
    {
        bgProcs[i] = NULL;
    }
    
    // infinite loop to continue running the shell
    while (1)
    {
        // variable to hold the input file name
        char* inputFile = NULL;
        
        // variable to hold the output file name
        char* outputFile = NULL;
        
        // int (true or false) to indicate if the user is requesting a background process
        int isBackground = FALSE;
        
        //initialize the command buffer
        char buffer[MAX_CMD_LENGTH];
        memset(buffer, '\0', sizeof(buffer));
        
        fprintf(stdout, ": "); //print command prompt
        
        fflush(stdout);
        
        // get the users input
        fgets(buffer,sizeof(buffer),stdin);
        
        // parse the input from the user via the command line
        // this is done so the char pointer can be passed to the exec() command unless it's one of the
        // built in commands
        char** command = parseInput(buffer, strlen(buffer), &inputFile, &outputFile, &isBackground);
        
        // if statements to determine what we need to do
        // if the commands entered by the user
        
        if(command[0] == '\0')
        {
            // either user entered a comment or hit the ENTER button
            // so we just continue and do nothing
            continue;
        }
        else if (strcmp("exit", command[0]) == 0)
        {
            // user elected to exit the small shell
            
            // free the command
            free(command);
            
            command = NULL;
            
            // call exit shell function
            exitSmallShell();
        }
        else if (strcmp("cd", command[0]) == 0) //built in cd command
        {
            // user elected to change directory
            changeDirectory(command[1]);
        }
        else if (strcmp("status", command[0]) == 0) //built in status command
        {
            // user elected to print the status of most recent process that
            // terminated
            getStatus(exitStatus);
        }
        else
        {
            // all other commands
            fflush(stdout);
            
            // run command and get exit status
            exitStatus = execCommands(command,inputFile,outputFile,isBackground);
        }
        
        // free the memory in order to accept the next command
        if(command != NULL)
        {
            free(command);
            command = NULL;
        }
        
        // check on the background processes and dthose that termianted print out the
        // PIDS and free the structs
        for(i = 0; i < MAX_PROCESSES; i++)
        {
            if(bgProcs[i] != NULL && bgProcs[i]->active == FALSE)
            {
                fflush(stdout);
                
                fprintf(stdout,"Background pid %d is done. Exit value: %d\n",bgProcs[i]->bgPID,bgProcs[i]->status);
                
                fflush(stdout);
                
                free(bgProcs[i]);
                
                bgProcs[i] = NULL;
            }
        }
    }
}


/*
 Function: exitSmallShell()
 Parameters: N/A
 Return: N/A
 Description: function that kills all processes still active within the shell.
 */

void exitSmallShell()
{
    int i;
    
    // loop through the struct array to kill any currently active processes
    // to avoid creating any zombies
    
    for(i = 0; i < MAX_PROCESSES; i++)
    {
        if(bgProcs[i] != NULL)
        {
            kill(bgProcs[i]->bgPID,SIGKILL);
            free(bgProcs[i]);
            bgProcs[i] = NULL;
        }
    }
    
    exit(EXIT_SUCCESS); //exit the shell
}



























