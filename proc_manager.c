#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * @Author: Ajeet Kotturu, Camellia Bazargan
 * @Author emails: camellia.bazargan@sjsu.edu, ajeet.kotturu@sjsu.edu
 * @Description: Executing multiple commands through either stdin or through a file input that is redirected through a pipe. 
 * Takes in commands that have zero or more arguements and outputs the results in the .out log file and any errors that occur in 
 * the .err log file for the associated child process. Runs all the commands inputted and waits until all command have completed.
*/
int main(void) {
  
	const int MAXLINE = 4096;
	char buf[MAXLINE];
 	pid_t pid; 
	int status;
	int i, j, ctr;
	
	char newString[20][20]; //Stores array of strings

	int commandCounter = 1; 
	printf("%% ");	/* print prompt (printf requires %% to print %) */
	while (fgets(buf, MAXLINE, stdin) != NULL) { //Cycle through commands
		
		if (buf[strlen(buf) - 1] == '\n') //If character is newline
			buf[strlen(buf) - 1] = 0; //Remove character

		int k;
		for (k=0; k<20; k++) { //Rows
			int ll;
            		for (ll=0; ll<20; ll++) { //Columns
                		newString[k][ll]=0; //Initialize newString array
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
		
		char *commArg[21]; //One dimentional string array to store commands
		int counter = 0; //Initialize counter
		int totalCount = ctr; 
		for (counter = 0; counter < totalCount; counter++) {
			commArg[counter] = newString[counter]; //Initialize string array
		}
		commArg[totalCount] = NULL; //Set last index to null
		
		if ((pid = fork()) < 0) { //If ID is less than 0
			printf("fork error"); //Print error
		} else if (pid == 0) {		/* child */
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
		}

		/* parent */
		while ((pid = wait(&status))>0) { //Parent process
			
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
				fprintf(file_desc_err, "Exited with exitcode = %d", WEXITSTATUS(status)); //Prints out exitcode that exited process
			
			} else if (WIFSIGNALED(status)) { //If signal exists

				fprintf(file_desc, "Finished child %d pid of parent %d\n", pid, (int)getpid()); //Print child and parent ID
				fprintf(file_desc_err, "Killed with signal %d", WTERMSIG(status)); //Prints out signal that killed process
			}
		}
		commandCounter++;	//incrementing the number of commands inputted
	}
	exit(0);
}
