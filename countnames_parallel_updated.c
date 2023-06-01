#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>

/**
*@Author: Ajeet Kotturu
*@Description: Counts the number of occurences of each name in each file and returns count of each name as output. If blank lines exist, the program pritns to stderr a warning message.
*/
int main(int argc, char *argv[]) {

	int childid;
	int child_state = -1;

	//Created max element variable to determine size of arrays
	const int TOTAL_NUM_ELEMENTS = 100;
	//Created char array that store each name
	char totalNameWords[TOTAL_NUM_ELEMENTS][30];
	//Created int array that stores the number of occurence of each 
    	//name. Indexes of both array link the name from char array with
    	//number of occurences in int array
	int totalNameCounter[TOTAL_NUM_ELEMENTS];
	
	//Keeps track of total number of names in arrays
	int totalCounter = 0;
	
	//Stores the name retrieved from the pipe
	char nameSaved[30];
	
	//Passes names from the child to the parent
	int p[2];


	if (pipe(p) < 0)
		exit(1);
		
	//Passes name frequencies from the child to the parent
	int *p01 = (int*)mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);

	if (p01 == MAP_FAILED) {
		perror("mmap failed");
		return -1;
	}
		
	//Passes the total line count from each file from the child
	//to the parent
	int *p02 = (int*)mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);

	if (p02 == MAP_FAILED) {
		perror("mmap failed");
		return -1;
	}
		
	//Passes indices of files that cannot be opened form the child to
	//the parent
	int *pInvalid = (int*)mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);

	if (pInvalid == MAP_FAILED) {
		perror("mmap failed");
		return -1;
	}
			
	//traverses through all the file until all lines have been scanned
	for (int jj = 1; jj < argc; jj++) {
	
		//creates a child process
		childid = fork();
		
		//when the child process is 0, scan throguh the file
		if (childid == 0) {
			//Created max element variable to determine size of arrays
			const int NUM_ELEMENTS = 100;
			//Created char array that store each name
			char nameWords[NUM_ELEMENTS][30];
			//Created int array that stores the number of occurence of each 
    			//name. Indexes of both array link the name from char array with
    			//number of occurences in int array
			int nameCounter[NUM_ELEMENTS];
			
			//Created char vairable to store line in a file
			char line[30];

			//Opens the file. If file is not found, prints a message stating
			//file cannot be open and writes index to pInvalid. If file is
			//found, writes -1 to pInvalid.
			FILE *fp = fopen(argv[jj], "r");
			if (fp == NULL) {
				printf("Cannot open file %s\n", argv[jj]);
				//write(pInvalid[1], &jj, sizeof(jj));
				//strcpy(pInvalid[jj], jj);
				pInvalid[jj] = jj;
				exit(1);
			}
			else {
				int neg = -1;
				//write(pInvalid[1], &neg, sizeof(neg));
				//strcpy(pInvalid[jj], neg);
				pInvalid[jj] = neg;
			}
			
			
			//Created counter int to help store names in char array
			int counter = 0;
			//Created lineCounter int to count number of line traversed in file
			int lineCounter = 1;
			//Created spaces variable to count number of blank lines
			int spaces = 0;
			//traverses through the file until all lines have been scanned
			while (fgets(line, sizeof(line), fp)) {
				//if the character scanned is not a newline or space,
				//scan the line
				if (line[0] != '\n' && line[0] != ' ') {
					//sets last char of line to blank char
					line[strlen(line) - 1] = '\0';
					//created ispresent int to serve as boolean for
					//names that exist in the char array
					int isPresent = 1;
					//created found variable to find index when 
					//name is found
					int found = -1;
					//traverses through the loop to find the new
					
					//name in char array
					for (int i = 0; i < counter; i++) {
						//if line is equal to name in char
						//array, set isPresent to true and
						//get index where name occurs
						if (strcmp(line, nameWords[i]) == 0) {
							isPresent = 0;
							found = i;
						}
					}
					
					//if name is found in char array, increment number
					//of occurence of name by 1 in int array	
					if (isPresent == 0) {
						nameCounter[found]++;
					}
					//if counter is less than 100, add new name to 
					//char array
					else if (counter <100) {
						strcpy(nameWords[counter],line);
						nameCounter[counter] = 1;
						counter++;
					}
				}
				//if the character scanned is a newline or space,
				//print warning message to stderr
				else {
					fprintf(stderr, "Warning - File %s line %d is empty.\n", argv[jj], lineCounter + spaces);
					spaces++;
					continue;
				}
				lineCounter++;
		    	}
			
			//print names in char array and the number number of occurences 
				
			//of each name
			
			
			int offsetter = 0;
			for (int iii = 1; iii <= jj + 1; iii++) {
				if (pInvalid[jj] == -1) {
					//printf("OS %d\n", p02[iii]);
					offsetter += p02[iii];
					//printf("%d\n", offsetter);
				}
			}
			//printf("%d\n", offsetter);
					
					
					
			for (int i = 0; i < counter; i++) {
				//writes the name to the pipe
				write(p[1], nameWords[i], 30);
				
				
				//writes the name frequency to the pipe
				//write(p01[1], &nameCounter[i], sizeof(nameCounter[i]));
				//printf("%d\n", offsetter + i);
				p01[offsetter + i] = nameCounter[i];

			}
			
			
			
			//printf("Read %d\n", counter);
			//writes the total number of lines to the pipe
			//write(p02[1], &counter, sizeof(counter));
			//p02[jj] = counter;
			p02[jj] = counter;
			//printf("w%d %d\n", jj, p02[jj]);
			
			munmap(p01, 4096);
			munmap(p02, 4096);
			munmap(pInvalid, 4096);

			//closes the file
			fclose(fp);
			exit(0);
		}
		
		
		
	}
	
	

	//when process exits child and enters parent
	while ((wait(NULL)) > 0) {
		waitpid(childid, &child_state, 0);
	
		//created temporary counter to store total line count for each file
		//int tempCounter = 0;
		
		//created run counter to stores total line count for all files
		int runCounter = 0;
		
		//created invalid file index to store index of invalid file in argv array
		int invalidFileNo = -1;
		
		//created valid counter that stores total number of valid files
		int validCounter = argc;
		
		//traverses through the argv array the count number of valid files
		for (int mm = 1; mm < argc; mm++) {
			
			//reads pInvalid and stores it into invalidFileNo
			//read(pInvalid[0], &invalidFileNo, sizeof(invalidFileNo));
			//strcpy(&invalidFileNo, pInvalid[mm - 1])
			invalidFileNo = pInvalid[mm];
			
			//decrements the counter when file index is not -1
			if (invalidFileNo != -1) {
				validCounter--;
			}
		}
		
		//loops through the number of valid files counter and sums the total line count for each file
		for (int jjj = 1; jjj < argc; jjj++) {
			if (pInvalid[jjj] == -1) {
				//reads the total number of lines from the pipe
				//and stores this into the tmpCounter variable
				//read(p02[0], &tempCounter, sizeof(tempCounter));
				
				//printf("Temp Counter %d = %d\n", jjj, tempCounter);
				//tempCounter = p02[jjj];
				//tempCounter = p02[jjj];
				//printf("Temp Counter %d = %d\n", jjj, tempCounter);
				//runCounter+=tempCounter;
				
				//printf("W%d %d\n", jjj, p02[jjj]);
				
				
				runCounter+=p02[jjj];
			}
		}
		
		//loops through the number of total lines and find the new
		//name in char array
		for (int jj = 0; jj < runCounter; jj++) {
		
			//reads the name from the pipe
			//and stores this into the nameSave variable
			read(p[0], nameSaved, 30);
			
			//created ispresent int to serve as boolean for
			//names that exist in the char array
			int isPresent = 1;
			//created found variable to find index when 
			//name is found
			int found = -1;
			//traverses through the loop to find the new
			//name in char array
			for (int i = 0; i < totalCounter; i++) {
				//if line is equal to name in char
				//array, set isPresent to true and
				//get index where name occurs
				if (strcmp(nameSaved, totalNameWords[i]) == 0) {
					isPresent = 0;
					found = i;
				}
			}
				
			//if name is found in char array, increment number
			//of occurence of name by a temporary value
			if (isPresent == 0) {
				int temp;
				
				//reads the name frequency from the pipe
				//and stores this into a temporary
				// variable
				//read(p01[0], &temp, sizeof(temp));
				temp = p01[jj];
				
				//increments totalNameCounter at found index by temporary value
				totalNameCounter[found]+=temp;
			}
			//if counter is less than 100, add new name to 
			//char array
			else if (totalCounter <100) {
				strcpy(totalNameWords[totalCounter],nameSaved);
				int temp;
				
				//reads the name frequency from the pipe
				//and stores this into a temporary
				// variable
				//read(p01[0], &temp, sizeof(temp));
				temp = p01[jj];
				totalNameCounter[totalCounter] = temp;
				totalCounter++;
			}
    		}
		  
		//print names in char array and the number number of occurences 
		//of each name
		for (int i = 0; i < totalCounter; i++) {
			printf("%s: %d\n", totalNameWords[i], totalNameCounter[i]);
		}
		  
		munmap(p01, 4096);
		munmap(p02, 4096);
		munmap(pInvalid, 4096);
		  
		//exits the parent process
		exit(0);  
	}
	return 0;
}
