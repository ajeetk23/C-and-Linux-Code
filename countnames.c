#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
*@Author: Ajeet Kotturu
*@Description: Counts the number of occurences of each name in the file and returns count of each name as output. If blank lines exist, the program pritns to stderr a warning message.
*/
int main(int argc, char *argv[]) {
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
	//file cannot be open
	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("cannot open file\n");
		exit(1);
	}
		
	//Created counter int to help store names in char array
	int counter = 0;
	//Created lineCounter int to count number of line traversed in file
	int lineCounter = 1;
	//Created spaces variable to count number of blank lines
	int spaces = 0;
	//traverses throguh the file until all lines have been scanned
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
			else if (counter < 100){
				
				strcpy(nameWords[counter],line);
				nameCounter[counter] = 1;
				counter++;
			}
			
		}
		//if the character scanned is a newline or space,
		//print warnign message to stderr
		else {
			fprintf(stderr, "Warning - Line %d is empty.\n", lineCounter + spaces);
			spaces++;
			continue;
		}
		lineCounter++;
    	}
	
	//print names in char array and the number number of occurences 
	//of each name
	for (int i = 0; i < counter; i++) {
		printf("%s: %d\n", nameWords[i], nameCounter[i]);
	}

	//closes the file
    	fclose(fp);

	return 0;
}
