#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


/*
 * @Author: Ajeet Kotturu, Camellia Bazargan
 * @Author emails: camellia.bazargan@sjsu.edu, ajeet.kotturu@sjsu.edu
 * @Description: Executing multiple commands through either stdin or through a file input that is redirected through a pipe. 
 * Takes in commands that have zero or more arguements and outputs the results in the .out log file and any errors that occur in 
 * the .err log file for the associated child process. Runs all the commands inputted and waits until all command have completed.
 * Stores each command in the hash table and record the start and finish time for each command using the hash table. Gets the 
 * duration of the command using the start and finish times. If command has a duration more than 2 seconds, it restarts until it 
 * recieve a kill signal. Otherwise, it prints a statement stating that the process spawned too quickly.
*/

/* Please note the following before grading
 * Point 1: Kill commands are kill all or p kill, and it takes more than one command to temrinate the program
 * Point 2: Last .out file does not necessarily contain a command that was killed by a signal
 */


struct nlist { /* table entry: */
    struct nlist *next; /* next entry in chain */
    struct timespec starttime; //Stores start time
    struct timespec finishtime; //Store finish time
    int index; // this is the line index in the input text file
    int pid;  // the process id. you can use the pid result of wait to lookup in the hashtable
    char *command; // command. This is good to store for when you decide to restart a command
};

#define HASHSIZE 101
static struct nlist *hashtab[HASHSIZE]; /* pointer table */

/* This is the hash function: form hash value for string s */
unsigned hash(int pid) {
    return pid % HASHSIZE;
}

/* lookup: look for s in hashtab */
/* This is traversing the linked list under a slot of the hash table. The array position to look in is returned by the hash function */
struct nlist *lookup(int pid)
{
    struct nlist *np;
    for (np = hashtab[hash(pid)]; np != NULL; np = np->next)
        if (pid == np->pid)
          return np; /* found */
    return NULL; /* not found */
}


struct nlist *insert(char *command, int pid, int index) {
    struct nlist *np;
    unsigned hashval;
    if ((np = lookup(pid)) == NULL) { /* case 1: the pid is not found, so you have to create it with malloc. Then you want to set the pid, command and index */
        np = (struct nlist *) malloc(sizeof(*np));
        np->pid = pid;
        if (np == NULL || np->pid == 0)
          return NULL;
        np->index = index;
        hashval = hash(pid);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else 
        free((void *) np->command);
    if ((np->command = strdup(command)) == NULL)
       return NULL;
    return np;
}





int main(void) {
  
	const int MAXLINE = 4096;
	char buf[MAXLINE]; //input string
 	pid_t pid; //process id
	int status; //int for wait method
	int i, j, ctr; //helper variables
	
	char newString[101][30]; //stores all commands inputted
	
	char *commArg[21]; //One dimentional string array to store commands

	int commandCounter = 1; //tracks index of commands
	
	struct timespec start; //gets os time
	
	while (fgets(buf, MAXLINE, stdin) != NULL) { //Cycle through commands
		if (buf[strlen(buf) - 1] == '\n') //If character is newline
			buf[strlen(buf) - 1] = 0; //Remove character

		int k;
		for (k=0; k<20; k++) { //Rows
			int ll;
            		for (ll=0; ll<20; ll++) { //Columns
                		newString[k][ll]=0; //initialize newString
                	}
                }

		j=0; 
		ctr=0;
		for(i=0;i<=(strlen(buf));i++)
		{
		    /* if space or NULL found, assign NULL into newString[ctr]*/
		    if(buf[i]==' ' || buf[i]=='\0') 
		    {
		        newString[ctr][j]='\0';
		        ctr++;  /*for next word*/
		        j=0;    /*for next word, init index to 0*/
		    }
		    else
		    {
		        newString[ctr][j]=buf[i]; 
		        j++;
		    }
		}
		
		
		int counter = 0; //Initialize counter
		int totalCount = ctr; 
		for (counter = 0; counter < totalCount; counter++) { //Cycle through totalCount
			commArg[counter] = newString[counter]; //Initialize string array
		}
		commArg[totalCount] = NULL; //Set last index to null
		
		/*--------------------------------------------------------*/
		
		
		
		
		if ((pid = fork()) < 0) { //If ID is less than 0
			printf("fork error"); //Print error
		} 
		clock_gettime(CLOCK_MONOTONIC, &start);
		
		if (pid == 0) {		/* child */

			
			
			char fileNamePID[8];
			sprintf(fileNamePID, "%d", (int)getpid()); //Convert int to string
			char fileNameOut[8];
			strcpy(fileNameOut, fileNamePID); //Copy file name
			strcat(fileNameOut, ".out"); //Append .out to file name
			int file_desc = open(fileNameOut, O_WRONLY | O_CREAT, 0777); //Create file
			if (dup2(file_desc, STDOUT_FILENO) == -1) { //Invalid file
				return 1;
			}
			printf("Starting Command %d: child %d pid of parent %d\n", commandCounter, (int)getpid(), (int)getppid()); //Prints to outfile
			fflush(stdout); //Flush output buffer
			
			
			char fileNameErr[8];
			strcpy(fileNameErr, fileNamePID); //Copy file name
			strcat(fileNameErr, ".err"); //Append .err to file name
			int file_desc_err = open(fileNameErr, O_WRONLY | O_CREAT, 0777); //Create file
			if (dup2(file_desc_err, STDERR_FILENO) == -1) { //Invalid file
				return 1;
			}
			fflush(stderr); //Flush stderr
			
			execvp(newString[0], commArg); //Run command
			
			
			close(STDOUT_FILENO); //Close file
			
			fprintf(stderr, "Error code");
			fflush(stderr); //Flush stderr
			close(STDERR_FILENO); //Close file
			
			
			exit(127); //Exit
		} else if (pid > 0) {
			struct nlist *entry_new = insert(buf, pid, commandCounter); //insert command process id and index to hash table
			entry_new->starttime = start; //Set start time parameter of hash table node
		}
 
		
		commandCounter++;	//incrementing the number of commands inputted
	}
	
	/* parent */
	while ((pid = wait(&status))>0) { //Parent process
	
		struct timespec finish; //Records os time
		double elapsed; //Duration
		
		if (pid > 0) { //Parent
			clock_gettime(CLOCK_MONOTONIC, &finish); //Record finish time
			
			struct nlist *entry_found = lookup(pid); //Get child id
			entry_found->finishtime = finish; //Set finish time
			
			elapsed = (finish.tv_sec - (entry_found->starttime).tv_sec); //Get elapsed time

			
			char fileNamePID[8];
			sprintf(fileNamePID, "%d", (int)pid); //Convert int to string
			char fileNameOut[8];
			strcpy(fileNameOut, fileNamePID); //Copy file name
			strcat(fileNameOut, ".out");	//Append .out to filename
			FILE *file_desc = fopen(fileNameOut, "a"); //Append to original output file of child process
			
			
			
			char fileNameErr[8];
			strcpy(fileNameErr, fileNamePID); //Copy file name
			strcat(fileNameErr, ".err"); //Append .err to filename
			FILE *file_desc_err = fopen(fileNameErr, "a"); //Append to original error file of child process
			
			if (WIFEXITED(status)) { //If the command is finished running
				
				fprintf(file_desc, "Finished child %d pid of parent %d\n", pid, (int)getpid()); //Print child and parent ID
				fprintf(file_desc_err, "Exited with exitcode = %d\n", WEXITSTATUS(status)); //Prints out exitcode that exited process
				
			} else if (WIFSIGNALED(status)) { //If signal exists

				fprintf(file_desc, "Finished child %d pid of parent %d\n", pid, (int)getpid()); //Print child and parent ID
				fprintf(file_desc_err, "Killed with signal %d\n", WTERMSIG(status)); //Prints out signal that killed process
			}
			
			fprintf(file_desc, "Finished at %ld, runtime duration %f\n", finish.tv_sec, elapsed); //Print duration of command
			
			/*----------------------------------------------*/
			
			if (elapsed > 2) { //If Duration is > 2 seconds
			
				struct nlist *entry_old = lookup(pid); //Get child id
				
				/*-------------------------------*/
			
				char *commArgNew[30]; //Stores pointer to command
				char **nextComm = commArgNew; //Stores pointer to pointer to command
				char bufCopy[MAXLINE];  //Copy input string
				strcpy(bufCopy, entry_old->command); //Stores input string from hash table
				char *tempComm = strtok(bufCopy, " \n"); //Split string by space
				while (tempComm != NULL) {
					*nextComm++=tempComm; //Storing a set of pointers
					tempComm = strtok(NULL, " \n"); //Takes tempComm and sets it to NULL
				}
				*nextComm = NULL; //Set to NULL
			
				/*-------------------------------*/
				
				if ((pid = fork()) < 0) { //If ID is less than 0
				printf("fork error"); //Print error
				} 
				clock_gettime(CLOCK_MONOTONIC, &start);
				
				if (pid == 0) {		/* child */
					
					char fileNamePID[8];
					sprintf(fileNamePID, "%d", (int)getpid()); //Convert int to string
					char fileNameOut[8];
					strcpy(fileNameOut, fileNamePID); //Copy file name
					strcat(fileNameOut, ".out"); //Append .out to file name
					int file_desc = open(fileNameOut, O_WRONLY | O_CREAT, 0777); //Create file
					if (dup2(file_desc, STDOUT_FILENO) == -1) { //Invalid file
						return 1;
					}
					printf("RESTARTING\n");
					printf("Starting Command %d: child %d pid of parent %d\n", entry_old->index, (int)getpid(), (int)getppid()); //Prints to outfile
					fflush(stdout); //Flush output buffer
					
					
					char fileNameErr[8];
					strcpy(fileNameErr, fileNamePID); //Copy file name
					strcat(fileNameErr, ".err"); //Append .err to file name
					int file_desc_err = open(fileNameErr, O_WRONLY | O_CREAT, 0777); //Create file
					if (dup2(file_desc_err, STDERR_FILENO) == -1) { //Invalid file
						return 1;
					}
					
					fprintf(stderr, "RESTARTING\n");
					fflush(stderr); //Flush stderr
					
					
					execvp(commArgNew[0], commArgNew); //Run command
					
					
					close(STDOUT_FILENO); //Close file
					
					
					fprintf(stderr, "Error code");
					fflush(stderr); //Flush stderr
					close(STDERR_FILENO); //Close file
					
					
					exit(127); //Exit
				} else if (pid > 0) {
					
					struct nlist *entry_new = insert(entry_old->command, pid, entry_old->index); //Insert original command and original index
					entry_new->starttime = start; //Stores start time value
				}
			}
			else {
				fprintf(file_desc_err, "Spawning too fast"); //If command takes <= 2 seconds
			}
			
		}
	}
	exit(0);

}
