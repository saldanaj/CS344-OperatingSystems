/*
 Student: Joaquin Saldana
 Assignment 4 / CS344 Operating Systems 1
 
 Description: This is the encryption program that sends the plantext and key to the encryption server 
 to cipher and later sends it back
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>

const int MAXCHARS = 1024;


// Standard error message
void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues


// Function Prototypes

int findFileSize(char* file);

int checkFile(char *file);

int sendFile(int socketFD, char *file);

// MAIN FUNCTION

int main(int argc, char *argv[])
{
    // Variable declarations
    int socketFD, port, n;
    struct sockaddr_in serverAddress;
    struct hostent *serverHostInfo;
    char buffer[100];
    char portno[6];
    char *keyFile;
    char *plainTextFile;
    
    // Checking to ensure there are at least
    // four commands entered by the user
    if (argc < 4)
    {
        fprintf(stderr,"ERROR: Incorrect syntax \n");
        exit(1);
    }
    
    // Get the name of the key file from the command line
    keyFile = argv[2];
    
    // Get the name of the plaintext file from the command line
    plainTextFile = argv[1];
    
    // Get the port number from the command line
    port = atoi(argv[3]);
    
    // Ensure the port number is greater than 50,000
    if (port < 50000)
    {
        fprintf(stderr, "Port number be > 50000\n");
        exit(2);
    }
    
    // Make sure the key file and the plaintxt file are of the same size in chars
    // Using the helper function "fileSize
    int plainTextFileSize = findFileSize(plainTextFile);
    
    int keyFileSize = findFileSize(keyFile);
    
    if (plainTextFileSize > keyFileSize)
    {
        fprintf(stderr,"ERROR: Key and plaintext file size are not the same\n");
        
        exit(1);
    }
    
    // Check that both files do not contain invalid characters
    if (checkFile(keyFile) || checkFile(plainTextFile))
    {
        fprintf(stderr, "ERROR: documents contained an invalid character\n");
        exit(1);
    }
    
    // Open the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    // Error handling if socket was not opened correctly
    if (socketFD < 0)
    {
        error("ERROR opening socket");
    }

    // Set the server information to localhost
    serverHostInfo = gethostbyname("localhost");
    
    // Error handling
    if (serverHostInfo == NULL)
    {
        fprintf(stderr,"ERROR, no such host\n");
        
        exit(0);
    }
    
    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);
    
    
    // Connect to the server
    if (connect(socketFD,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        error("ERROR connecting");
    }
    
    
    /************************************************************************************/
    
    // Retrieve the new port to connect to the server on
    memset(portno, '\0', 6);
    
    n = read(socketFD, portno, 5);
    
    serverAddress.sin_port = htons(atoi(portno));
    
    // Close the socket
    close(socketFD);
    
    // Open a new socket using the retrieved port
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    // Connect using the new port, else error out
    if (connect(socketFD,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        error("ERROR connecting");
    }
    
    /************************************************************************************/
    
    
    // Send number to server to indicate an encryption is needed
    sprintf(buffer,"0\n", 2);
    
    n = send(socketFD, buffer, 2, 0);
    
    // Report error
    if (n < 2)
    {
        error("ERROR writing to socket");
    }
    
    // Sending server the length of the file containing the key
    sprintf(buffer,"%i\n", findFileSize(keyFile));
    
    n = send(socketFD, buffer, strlen(buffer), 0);
    
    // Routine error handling
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    
    // Send the keyfile contents including the null terminator
    n = sendFile(socketFD, keyFile);
    
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    
    // Now doing the same for the plaintext file
    // sending it's length and it's contents w/ the null terminator
    sprintf(buffer,"%i\n", findFileSize(plainTextFile));
    
    n = send(socketFD, buffer, strlen(buffer), 0);
    
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    n = sendFile(socketFD, plainTextFile);
    
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    
    // Retrieve the message returned by the server to indicate if everything was successful
    // of there was an error
    n = recv(socketFD, buffer, 2, 0);
    
    if (n < 2)
    {
        error("ERROR reading from socket");
    }
    
    // Throw error is attempting to connect to the decypher server
    // rather than the connect server
    
    if (buffer[0] == '0')
    {
        close(socketFD);
        
        fprintf(stderr,"ERROR: cannot connect to the decypher server\n");
        
        exit(1);
    }
    
    // If no errors, then print out to stdout
    while (n = recv(socketFD, buffer, 1, 0))
    {
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        
        printf("%c",buffer[0]);
    }
    
    close(socketFD);
    
    return 0;
    
} // end of main function

//=======================================================================================================

/*
 Function: findFileSize 
 Parameters: char * 
 Returns: int
 Description: returns the file size of the file pointer passed as an 
 argument.
 */
int findFileSize(char *file)
{
    // open the file
    FILE *newfile = fopen(file, "r");
    
    // Throw error if problem opening file
    if (newfile == 0)
    {
        fprintf(stderr, "ERROR: Cannot open file %s\n", file);
        
        exit(1);
    }
    
    // Set file position to the end of the file
    fseek(newfile, 0L, SEEK_END);
    
    int size = ftell(newfile);

    fseek(newfile, 0L, SEEK_SET);
    
    fclose(newfile);
    
    return size;
}

/*
 Function: checkFile 
 Parameter: char * pointer 
 Returns: int 
 Description: checks that the file only contains upper case characters and blank spaces 
 returns 1 if it finds invalid characters
 */
int checkFile(char *file)
{
    // Local variables
    char c;
    FILE *newfile = fopen(file, "r");
    
    // Throw error if there is a problem with the contents of the file
    if (newfile == 0)
    {
        fprintf(stderr, "ERROR: Cannot open file %s\n", file);
        
        exit(1);
    }
    
    // Read the first line of the file up to the newline
    while((c = fgetc(newfile)) != '\n')
    {
        if ((c < 65 || c > 90) && c != 32)
        {
            fclose(newfile);
            
            return 1;
        }
    }
    
    // Close the file
    fclose(newfile);
    
    return 0;
}

/*
 Function: sendFile
 Parameter: int socket file descriptor, char * pointer
 Returns: int 
 Description: sends the contents of the file passed as the argument to the socket established 
 with the socket file descriptor
 */
int sendFile (int socketFD, char *file)
{
    // Local variables
    char buffer[1001];
    FILE *newfile;
    
    // Variables used to detect how much data is being sent and if successful
    int n, charsRemaining, totalLeft, outTotal = 0;
    
    // Open file
    newfile = fopen(file, "r");
    
    // Sends data 1000 chars at a time
    while (fgets(buffer, 1001, newfile))
    {
        // The total succesffully transmitted
        // in a given packet
        totalLeft = 0;
        
        // The amount remaining to be sent
        // in a given packet
        charsRemaining = strlen(buffer);
        
        // Continue sending until the entire packet
        // has been sent successfully.
        while (totalLeft < strlen(buffer))
        {
            // Send data
            n = send(socketFD, buffer + totalLeft, charsRemaining, 0);
            
            // If there was an error, return total amount of
            // file sent so far
            if (n == -1)
            {
                fclose(newfile);
                
                return outTotal;
            }
            
            totalLeft += n;
        
            charsRemaining -= n;
        }
        
        // Update number of bytes sent from the file
        outTotal += totalLeft;
    }
    
    // Close the file and return
    fclose(newfile);
    
    return outTotal;
}













