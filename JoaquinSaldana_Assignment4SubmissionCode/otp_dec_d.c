/*
 Student: Joaquin Saldana
 Assignment 4 / CS344 Operating Systems 1
 
 Description: This is the encryption program that acts as the server and encrypts the data sent
 by the client.  It does this by listening to one port and once the data is received forking a process w/ it's own
 socket to write the data to and encrypt it.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

// FUNCTION PROTOTYPES

unsigned int getFiles(int socketFD, char **key, char **encryptedText);
pid_t decryptFork(int socketFD);
char* decryptChars(char *encryptedText, char *key, unsigned int size);
char modularSubtraction(char a, char b);
int writeToClient(int socketFD, char *plainText, int size);

// MAIN FUNCTION

int main(int argc, char *argv[])
{
    // Variable declarations
    int listenSocketFD, establishedConnectionFD, forkSocketFD, forkPort, port, n;
    socklen_t sizeOfClientInfo;
    char buffer[100];
    char port_str[6];
    struct sockaddr_in serverAddress, forkServerAddress, clientAddress;
    
    // Seed the random number generator
    srand(time(0));
    
    // Error checking the input from the command line
    // User must at least enter the port number
    
    if (argc < 2)
    {
        fprintf(stderr, "Incorrect syntax");
        
        exit(0);
    }
    
    // convert port number into integer
    port = atoi(argv[1]);
    
    // Verify the port number is valid
    if(port < 50000)
    {
        fprintf(stderr, "ERROR: must pick a port > 50000" );
        
        exit(1);
    }
    
    // Open the socket for the listening port
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocketFD < 0)
    {
        error("ERROR opening socket");
    }
    
    
    // Open the socket for forking to a new connection
    forkSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    if (forkSocketFD < 0)
    {
        error("ERROR opening socket");
    }
    
    
    // Set up the server information struct
    memset((char *) &serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    // Set up the server struct for the forked connection
    memset((char *) &forkServerAddress, '\0', sizeof(forkServerAddress));
    forkServerAddress.sin_family = AF_INET;
    forkServerAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    // Bind the listening port
    if (bind(listenSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        error("ERROR on binding");
    }
    
    // Listen on the socket
    listen(listenSocketFD,5);
    
    // Get the length of the client adress
    sizeOfClientInfo = sizeof(clientAddress);
    
    // Inifnite loop to accept connects so long as it's running
    while(1)
    {
        // Accept a connection with a new socket fd
        establishedConnectionFD = accept(listenSocketFD,(struct sockaddr *) &clientAddress, &sizeOfClientInfo);
        
        // Throw error if problem accpeting the data.
        if (establishedConnectionFD < 0)
        {
            error("ERROR on accept");
        }
        
        
        // Bind the connection to fork to a random port
        // Continue trying ports until an open port is found
        do
        {
            forkPort = rand() % 65536;
            forkServerAddress.sin_port = htons(forkPort);
            
        }while(bind(forkSocketFD, (struct sockaddr *)&forkServerAddress, sizeof(forkServerAddress)) < 0);
        
        
        // Begin listening on the new port
        listen(forkSocketFD,1);
        
        // Send the new port to the client to connect
        sprintf(port_str, "%i", forkPort);
        
        n = write(establishedConnectionFD, port_str, strlen(port_str));
        
        // Throw error if problem sending data
        if (n < 0)
        {
            error("ERROR sending new port to client");
        }
        
        // Fork to a new process for encryption
        decryptFork(forkSocketFD);
        
        // Close the forked socket for this process
        close(forkSocketFD);
        
        // Open a new socket to fork a new connection
        forkSocketFD = socket(AF_INET, SOCK_STREAM, 0);
        
        // Throw error is problem opening socket
        if (forkSocketFD < 0)
        {
            error("ERROR opening socket");
        }
        
    }

    // Close the socket although this is an infinite loop technically
    close(listenSocketFD);

    return 0;
    
} // end of main function


/*
 Function: getFiles
 Parameters: int socketFD, char pointer to key file, char pointer to plaintext file
 Returns: unsigned int
 Description: retrieves the key file and txt file
 */

unsigned int getFiles(int socketFD, char **_key, char **_encryptedText)
{
    // Create a buffer for retrieving file sizes
    char buffer[100];
    
    // Variable declarations
    unsigned int keySize, encryptedTextSize, i = 0, n;
    
    // Receive the first two characters to determine if we need to encrypt or decrypt
    n = recv(socketFD, buffer, 2, 0);
    
    if (n < 0)
    {
        error("ERROR reading from socket");
    }
    
    
    // If the first character is "0", client wants
    // decryption.  Return -1
    if (buffer[0] == '0')
    {
        return -1;
    }
    
    // Next to read is the size of the file
    do
    {
        n = recv(socketFD, buffer+i, 1, 0);
        
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        
        i++;
    }while (buffer[i-1] != '\n');
    
    // Add null terminator to the end of the array line
    buffer[i-1] = '\0';
    
    // convert it to a integer since it's the length of the key
    keySize = atoi(buffer);
    
    // Allocate memory to hold the size of the key
    char *key = malloc(keySize);
    
    // Set the input argument to point to this address
    *_key = key;
    
    
    // The next item sent is the key file
    i = 0;
    
    do
    {
        n = recv(socketFD, key+i, 1, 0);
        
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        
        i++;
    }while (key[i-1] != '\n');
    
    
    // The next is the size of the encrypted file

    i = 0;

    buffer[0] = 0;
    do
    {
        n = recv(socketFD, buffer+i, 1, 0);
        
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        
        i++;
    }while (buffer[i-1] != '\n');
    
    // Set the null terminator
    buffer[i-1] = '\0';
    
    // Convert the string to an integer
    encryptedTextSize = atoi(buffer);
    
    // Allocate memory for the encrypted file
    char *encryptedText = malloc(keySize);
    
    // Set the input argument to point to this address
    *_encryptedText = encryptedText;
    
    
    // The next item sent is the contents of the encrypted file
    i = 0;
    
    do
    {
        n = recv(socketFD, encryptedText + i, 1, 0);
        
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        
        i++;
    }
    while (encryptedText[i-1] != '\n');
    
    // Return the size of the retrieved encrypted file
    return encryptedTextSize;
}


/*
 Function: encryptFork
 Parameters: int socketFD
 Returns: unsigned int
 Description: retrieves the key file and txt file
 */

pid_t decryptFork(int socketFD)
{
    int n;
    
    // fork a process id
    pid_t pid = fork();
    
    char buffer[2];
    
    // Process is the parent so just return
    if (pid > 0)
    {
        return pid;
    }
    
    
    // Process is the child
    else if (pid == 0)
    {
        // Declare variables here to return efficiently
        // in case fork() fails
        char *key;
        char *plainText;
        char *encryptedText;
        unsigned int size;
        
        // Setup the socket
        struct sockaddr_in clientAddress;
        socklen_t clientLength;
        clientLength = sizeof(clientAddress);
        
        socketFD = accept(socketFD, (struct sockaddr *)&clientAddress, &clientLength);
        
        // Retrieve the size of encryptedText and key

        size = getFiles(socketFD, &key, &encryptedText);
        
        // If size == -1, its an intending encrypt which means the client is
        // attempting to contact the wrong server
        if (size == -1)
        {
            // Send back "0" to indicate request rejected
            sprintf(buffer,"0\n");
            
            n = send(socketFD, buffer, 2, 0);
            
            // Throw error if it failed
            if (n < 2)
            {
                error("ERROR writing to socket");
            }
            
            fprintf(stderr,"ERROR: cannot use otp_dec_d\n");
            
            // Close the socket and exit
            close(socketFD);
            
            exit(0);
        }
        else
        {
            // Send back message to indicate request was accepted
            sprintf(buffer,"1\n");

            n = send(socketFD, buffer, 2, 0);
            
            // Throw error if it fails
            if (n < 2)
            {
                error("ERROR writing to socket");
            }
            
        }
        
        // Decrypt the cipherText using the key
        plainText = decryptChars(encryptedText, key, size);
        
        // Send the decrypted plaintext to the client
        writeToClient(socketFD, plainText, size);
        
        // Close the socket and exit
        close(socketFD);
        
        exit(0);
    }
    
    // Fork failed, print error
    else if (pid < 0)
    {
        fprintf(stderr,"otp_dec_d: fork failed\n");
    }
    
    // Execution only gets here if fork failed
    return pid;
    
}


/*
 Function: decryptChars
 Parameters: char ptr to encrypted text file, char ptr to key file, and size of both files
 Returns: char *
 Description: decrypts the encrypted file w/ the key given.
 */

char* decryptChars(char *encryptedText, char *key, unsigned int size)
{
    // Local declarations
    int i, n;
    
    char *plainText;
    
    // Create memory to store the plaintext
    plainText = malloc(size);
    
    // The last character should be a newline for
    // sending back to the client
    plainText[size-1] = '\n';
    
    // Decrypt, one character at a time
    for (i = 0; i < size-1; i++)
    {
        plainText[i] = modularSubtraction(encryptedText[i], key[i]);
    }
    
    // Return a pointer to the plaintext
    return plainText;
}



/*
 Function: modularSubtraction
 Parameters: two chars to calculate
 Returns: char modulated
 Description: performs the modular subtraction of two chars
 */

char modularSubtraction(char a, char b)
{
    char c;
    
    if (a == 32)
    {
        a = 26;
    }
    else
    {
        a -= 65;
    }
    
    if (b == 32)
    {
        b = 26;
    }
    else
    {
        b -= 65;
    }
    
    // Subtract the two chars
    c = a - b;
    
    if (c < 0)
    {
        c += 27;
    }
    
    if (c == 26)
    {
        return 32;
    }
    else
    {
        return c + 65;
    }
}


/*
 Function: writeToClient
 Parameters: int socket file descriptor, char pointer to encrypted text, size of file
 Returns: int
 Description: sends the plaintext data back to the client
 */

int writeToClient(int socketFD, char *plainText, int size)
{
    // Variable declarations
    int n, remaining, packetSize, totalLeft, outTotal = 0;
    
    // Continue sending packets of 1000 bytes until no more data
    while (outTotal < size)
    {
        // The number of bytes sent in the current packet
        totalLeft = 0;
        
        // If there is more than 1000 characters remaining
        // to be sent, send a packet of 1000 bytes
        if (size - outTotal > 1000)
        {
            packetSize = 1000;
        }
        else
        {
            packetSize = size - outTotal;
        }
        
        
        // Reset remaining
        remaining = packetSize;
        
        // keep sending
        while (totalLeft < packetSize)
        {

            n = send(socketFD, plainText + outTotal,remaining, 0);
            
            // Return total bytes sent in case of error
            if (n == -1)
            {
                return outTotal;
            }
            
            // Increase total left and outotals
            totalLeft += n;
            
            outTotal += n;
            
            // Decrement the remaining
            remaining -= n;
        }
    }
    
    return outTotal;
}


