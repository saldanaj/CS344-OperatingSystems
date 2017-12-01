
/*
 Student: Joaquin Saldana
 Class: CS344 - Operating Systems 1
 Assignment 3 / smallsh
 
 
 Description: This is the main function for the smallsh assignment.  
 
 It initializes the sigaction structures and signals with the appropriate 
 signal handlers.  Afterwards the run_smallsh() function is called the shell begins to run.
 
 */

#include "smallsh.h"

int main()
{
    // Sig handler for interrupts
    struct sigaction fgAction;
    
    fgAction.sa_handler = fgCatchSig;

    sigfillset(&fgAction.sa_mask);
    
    fgAction.sa_flags = 0;
    
    sigaction(SIGINT, &fgAction, NULL);
    
    
    // Sig handler for background processs
    struct sigaction bgAction;
    
    bgAction.sa_handler = bgCatchSig;
    
    sigfillset(&bgAction.sa_mask);
    
    bgAction.sa_flags = 0;
    
    sigaction(SIGCHLD, &bgAction, NULL);
    
    
    // Sig handler for the SIGSTP signal
    struct sigaction stopAction;
    
    stopAction.sa_handler = catchStopSig;

    sigfillset(&stopAction.sa_mask);
    
    stopAction.sa_flags = 0;
    
    sigaction(SIGTSTP, &stopAction, NULL);
    
    
    runSmallShell();
    return 0;
}
