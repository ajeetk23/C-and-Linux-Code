#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>



/**
*@Author: Ajeet Kotturu, Camellia Bazargan
*@Description: Counts the number of occurences of each name in each file and returns count of each name as output. If blank lines exist, the program pritns to stderr a warning message.
*/

//thread mutex lock for access to the log index
//Use for mutexlock mutual exclusion
//when you print log messages from each thread
pthread_mutex_t tlock1 = PTHREAD_MUTEX_INITIALIZER;


//thread mutex lock for critical sections of allocating THREADDATA
//Use for mutexlock mutual exclusion
pthread_mutex_t tlock2 = PTHREAD_MUTEX_INITIALIZER;


//thread mutex lock for access to the name counts data structure
//Use for mutexlock mutual exclusion
pthread_mutex_t tlock3 = PTHREAD_MUTEX_INITIALIZER;


//Declared thread runner method and threads
void* thread_runner(void*);
pthread_t tid1, tid2;

//struct points to the thread that created the object.
//This is useful for you to know which is thread1. Later thread1 will also deallocate.
struct THREADDATA_STRUCT
{
  pthread_t creator;
};
typedef struct THREADDATA_STRUCT THREADDATA;

THREADDATA* p=NULL;


//variable for indexing of messages by the logging function.
int logindex=1;
int *logip = &logindex;


//Make Hash Table
struct NAME_STRUCT
{
  struct NAME_STRUCT *next; //Pointer to the next node
  char *name; //Key to hash table, stores name
  int count; //Value of hash table, stores name frequencies
};
#define HASHSIZE 101 //Size of hash table
static struct NAME_STRUCT *THREAD_NAME[HASHSIZE]; //Data structure for the hash table

//TODO hash function for hash table
unsigned hash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}


/* lookup: look for s in hashtab */
/* This is traversing the linked list under a slot of the hash table. The array position to look in is returned by the hash function */
struct NAME_STRUCT *lookup(char *s)
{
    struct NAME_STRUCT *np;
    for (np = THREAD_NAME[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
          return np; /* found */
    return NULL; /* not found */
}



/* insert: put (name, count) in the hash table */
/* This insert returns a NAME_STRUCT node. Thus when insert is cvalled in the threads  */
struct NAME_STRUCT *insert(char *name, int count)
{
    struct NAME_STRUCT *np;
    unsigned hashval;
    //TODO change to lookup by pid. There are 2 cases:
    if ((np = lookup(name)) == NULL) { /* case 1: the pid is not found, so you have to create it with malloc. Then you want to set the pid, command and index */
        np = (struct NAME_STRUCT *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL)
          return NULL;
        np->count = count; //Stores the count
        hashval = hash(name);
        np->next = THREAD_NAME[hashval];
        THREAD_NAME[hashval] = np;
    } else {/* case 2: the pid is already there in the hashslot, i.e. lookup found the pid. In this case you can either do nothing, or you may want to set again the command  and index (depends on your implementation). */
        //free((void *) np->count); /*free previous defn */
        np->count = count;
    }
    if (count == 0)
	       return NULL;
    return np;
}

//Frees the hash table
void FreeHashTable() {
	struct NAME_STRUCT *np; //Initial name struct node
	for (int i = 0; i < 100; i++) {
		np = THREAD_NAME[i]; //Node equals element at index
		while (THREAD_NAME[i] != NULL) { //While element is not null
			np = THREAD_NAME[i]; //Node equals element at index
			THREAD_NAME[i] = np->next; //Element equals node next to initial node
			free(np); //Free node
		}
	}
}


// Variables to store date and time components
int hours, minutes, seconds, day, month, year;
time_t now;
struct tm *local;



//Created char vairable to store line in a file
char line[30];






/*********************************************************
// function main
*********************************************************/
/*Runs the threads that gather the names and their frequencies and displays then using the hash table
 * Only 2 files are used
 * */
int main(int argc, char *argv[])
{
  //Less than 2 Files
	if (argc < 3) {
		printf("Error: Less than 2 files inputted\n");
	}
  //More than 2 Files
	else if (argc > 3) {
		printf("Error: More than 2 files inputted\n");
	}
  //Exactly 2 FIle
	else {
		//Similar interface as A2: give as command-line arguments three filenames of numbers (the numbers in the files are newline-separated).
		printf("======================== Log Message and Error Message ========================\n");

		printf("create first thread\n");
		pthread_create(&tid1,NULL,thread_runner,(void* )argv[1]);

		printf("create second thread\n");
		pthread_create(&tid2,NULL,thread_runner,(void* )argv[2]);

		printf("wait for first thread to exit\n");
		pthread_join(tid1,NULL);
		printf("first thread exited\n");

		printf("wait for second thread to exit\n");
		pthread_join(tid2,NULL);
		printf("second thread exited\n");

		//Print out the sum variable with the sum of all the numbers
		printf("======================== Name Count Result ========================\n");

		struct NAME_STRUCT *np; //Initialize name struct node
		for (int i = 0; i < 100; i++) {
			np = THREAD_NAME[i]; //Node equals element at index
			while (np != NULL) { //While element is not null
				printf("%s: %d\n", np->name, np->count); //Print node key and value
				np = np->next; //Next node
			}
		}

		//Frees the Hash Table
		FreeHashTable();
	}




	exit(0);

}//end main

//Runs the threads
void* thread_runner(void* x)
{
  //Initializes the threads
	pthread_t me;

	me = pthread_self();
  //Display a message that shows that the thread was initialized
	printf("This is thread %ld (p=%p)\n",me,p);

	pthread_mutex_lock(&tlock2); // critical section starts
	if (p==NULL) {
		p = (THREADDATA*) malloc(sizeof(THREADDATA));
		p->creator=me;
	}
	pthread_mutex_unlock(&tlock2);  // critical section ends

	if (p!=NULL && p->creator==me) {

		//Locking the mutex code so no other thread or process can edit it
		pthread_mutex_lock(&tlock1);
		//Sets time and sets based on location
		time(&now);
		local = localtime(&now);

		//Initializes day, month, and year
		day = local->tm_mday;            
		month = local->tm_mon + 1;       //Add aditional month to get correct month
		year = local->tm_year + 1900;    //Add aditional 1900 to get correct year

		//Initializes hours, minutes, and seconds
		hours = local->tm_hour;          
		minutes = local->tm_min;         
		seconds = local->tm_sec;  
			
		if (hours < 12) {	
			printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d am: This is thread %ld and I created THREADDATA %p\n", logindex, me, getpid(), day, month, year, hours, minutes, seconds, me,p);
		}
		else {
			printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d pm: This is thread %ld and I created THREADDATA %p\n", logindex, me, getpid(), day, month, year, hours, minutes, seconds, me,p);
		}
		++logindex;
		pthread_mutex_unlock(&tlock1);
	} else {
		//Locking the mutex code so no other thread or process can edit it
		pthread_mutex_lock(&tlock1);
		//Sets time and sets based on location
		time(&now);
		local = localtime(&now);

		//Initializes day, month, and year
		day = local->tm_mday;            
		month = local->tm_mon + 1;       //Add aditional month to get correct month
		year = local->tm_year + 1900;    //Add aditional 1900 to get correct year

		//Initializes hours, minutes, and seconds
		hours = local->tm_hour;          
		minutes = local->tm_min;         
		seconds = local->tm_sec;
		
		if (hours < 12) {
			printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d am: This is thread %ld and I can access the THREADDATA %p\n",logindex, me, getpid(), day, month, year, hours, minutes, seconds, me,p);
		}
		else {
			printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d pm: This is thread %ld and I can access the THREADDATA %p\n",logindex, me, getpid(), day, month, year, hours, minutes, seconds, me,p);
		}
		
		++logindex;
		pthread_mutex_unlock(&tlock1);
	}


  //First Thread
	if (p!=NULL && p->creator==me) {
		//Opens the file. If file is not found, prints a message stating
		//file cannot be open and writes index to pInvalid. If file is
		//found, writes -1 to pInvalid.
		FILE *fp = fopen((char *)x, "r");
		if (fp == NULL) {
			printf("Cannot open file %s\n", (char *)x);
			exit(1);
		}
		else {
			//Locking the mutex code so no other thread or process can edit it
			pthread_mutex_lock(&tlock1);
			//Sets time and sets based on location
			time(&now);
			local = localtime(&now);

			//Initializes day, month, and year
			day = local->tm_mday;            
			month = local->tm_mon + 1;       //Add aditional month to get correct month
			year = local->tm_year + 1900;    //Add aditional 1900 to get correct year

			//Initializes hours, minutes, and seconds
			hours = local->tm_hour;          
			minutes = local->tm_min;         
			seconds = local->tm_sec;         

			//If hours is AM time
			if (hours < 12)
			    printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d am: This is thread %ld and I opened %s\n", logindex, me, getpid(), day, month, year, hours, minutes, seconds, me, (char *)x);

			//If hours is PM time
			else {
			    printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d pm: This is thread %ld and I opened %s\n", logindex, me, getpid(), day, month, year, hours - 12, minutes, seconds, me, (char *)x);
			}

			++logindex;
			pthread_mutex_unlock(&tlock1);
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
				for (int i = 0; i < 100; i++) {
					//if line is equal to name in char
					//array, set isPresent to true and
					//get index where name occurs
					if (THREAD_NAME[i] != NULL) {
						if (strcmp(line, THREAD_NAME[i]->name) == 0) {
							isPresent = 0;
							found = i;
						}
					}
				}



				//if name is found in char array, increment number
				//of occurence of name by 1 in int array
				if (isPresent == 0) {
					int curr = THREAD_NAME[found]->count;
					curr++;
					insert(line, curr);
				}
				//if counter is less than 100, add new name to
				//char array
				else if (counter <100) {
					insert(line, 1);
					counter++;
				}
			}
			//if the character scanned is a newline or space,
			//print warnign message to stderr
			else {
				fprintf(stderr, "Warning - File %s line %d is empty.\n", (char *)x, lineCounter + spaces);
				spaces++;
				continue;
			}
			lineCounter++;
	    	}




	}
  //Second Thread
	else {
		//Opens the file. If file is not found, prints a message stating
		//file cannot be open and writes index to pInvalid. If file is
		//found, writes -1 to pInvalid.
		FILE *fp = fopen((char *)x, "r");
		if (fp == NULL) {
			printf("Cannot open file %s\n", (char *)x);
			exit(1);
		}
		else {
			//Locking the mutex code so no other thread or process can edit it
			pthread_mutex_lock(&tlock1);
			//Sets time and sets based on location
			time(&now);
			local = localtime(&now);

			//Initializes day, month, and year
			day = local->tm_mday;            
			month = local->tm_mon + 1;       //Add aditional month to get correct month
			year = local->tm_year + 1900;    //Add aditional 1900 to get correct year

			//Initializes hours, minutes, and seconds
			hours = local->tm_hour;          
			minutes = local->tm_min;         
			seconds = local->tm_sec;

			//If hours is AM time
			if (hours < 12)
			    printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d am: This is thread %ld and I opened %s\n", logindex, me, getpid(), day, month, year, hours, minutes, seconds, me, (char *)x);

			//If hours is PM time
			else {
			    printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d pm: This is thread %ld and I opened %s\n", logindex, me, getpid(), day, month, year, hours - 12, minutes, seconds, me, (char *)x);
			}

			++logindex;
			pthread_mutex_unlock(&tlock1);
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
				for (int i = 0; i < 100; i++) {
					//if line is equal to name in char
					//array, set isPresent to true and
					//get index where name occurs
					if (THREAD_NAME[i] != NULL) {
						if (strcmp(line, THREAD_NAME[i]->name) == 0) {
							isPresent = 0;
							found = i;
						}
					}
				}



				//if name is found in char array, increment number
				//of occurence of name by 1 in int array

				//of occurence of name by 1 in int array
				if (isPresent == 0) {
					int curr = THREAD_NAME[found]->count;
					curr++;
					insert(line, curr);
				}
				//if counter is less than 100, add new name to
				//char array
				else if (counter <100) {
					insert(line, 1);

					counter++;
				}
			}
			//if the character scanned is a newline or space,

			//print warnign message to stderr
			else {
				fprintf(stderr, "Warning - File %s line %d is empty.\n", (char *)x, lineCounter + spaces);
				spaces++;
				continue;
			}
			lineCounter++;
	    	}
	}




	//Frees the main threads and locks the code so that toher rpocess cannot edit it while the changes ar ebeing made
  //use mutex to make this a start of a critical section
	pthread_mutex_lock(&tlock2);
	if (p!=NULL && p->creator==me) {
		/**
		* Free the THREADDATA object.
		* Freeing should be done by the same thread that created it.
		* See how the THREADDATA was created for an example of how this is done.
		*/
		free(p);
		p = NULL;


		//Locking the mutex code so no other thread or process can edit it
			pthread_mutex_lock(&tlock1);
			//Sets time and sets based on location
			time(&now);
			local = localtime(&now);

			//Initializes day, month, and year
			day = local->tm_mday;            
			month = local->tm_mon + 1;       //Add aditional month to get correct month
			year = local->tm_year + 1900;    //Add aditional 1900 to get correct year

			//Initializes hours, minutes, and seconds
			hours = local->tm_hour;          
			minutes = local->tm_min;         
			seconds = local->tm_sec;

		//If hours is AM time
		if (hours < 12)
		    printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d am: This is thread %ld and I delete THREADDATA\n", logindex, me, getpid(), day, month, year, hours, minutes, seconds, me);

		//If hours is PM time
		else {
		    printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d pm: This is thread %ld and I delete THREADDATA\n", logindex, me, getpid(), day, month, year, hours - 12, minutes, seconds, me);
		}

		++logindex;
		pthread_mutex_unlock(&tlock1);

	} else {

		//Locking the mutex code so no other thread or process can edit it
			pthread_mutex_lock(&tlock1);
			//Sets time and sets based on location
			time(&now);
			local = localtime(&now);

			//Initializes day, month, and year
			day = local->tm_mday;            
			month = local->tm_mon + 1;       //Add aditional month to get correct month
			year = local->tm_year + 1900;    //Add aditional 1900 to get correct year

			//Initializes hours, minutes, and seconds
			hours = local->tm_hour;          
			minutes = local->tm_min;         
			seconds = local->tm_sec;

		//If hours is AM time
		if (hours < 12)
		    printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d am: This is thread %ld and I can access the THREADDATA\n", logindex, me, getpid(), day, month, year, hours, minutes, seconds, me);

		//If hours is PM time
		else {
		    printf("Logindex %d, thread %ld, PID %d, %02d/%02d/%d %02d:%02d:%02d pm: This is thread %ld and I can access the THREADDATA\n", logindex, me, getpid(), day, month, year, hours - 12, minutes, seconds, me);
		}

		++logindex;
        	pthread_mutex_unlock(&tlock1);
	}
	//{INSERT COMMENT} critical section ends
	pthread_mutex_unlock(&tlock2);

	pthread_exit(NULL);
	return NULL;

}//end thread_runner
