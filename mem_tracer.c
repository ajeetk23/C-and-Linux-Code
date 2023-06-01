#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

/**
 *CS149 assignment#4
 * Description: Print an array and linked list objects to output file. Also print memory usage statements to output file
 * Author names: Camellia Bazargan, Ajeet Kotturu
 * Author emails: camellia.bazargan@sjsu.edu, ajeet.kotturu@sjsu.edu
**/

/**
 * TRACE_NODE_STRUCT is a linked list of
 * pointers to function identifiers
 * TRACE_TOP is the head of the list is the top of the stack
**/
struct TRACE_NODE_STRUCT {
  char* functionid;                // ptr to function identifier (a function name)
  struct TRACE_NODE_STRUCT* next;  // ptr to next frama
};
typedef struct TRACE_NODE_STRUCT TRACE_NODE;
static TRACE_NODE* TRACE_TOP = NULL;       // ptr to the top of the stack


/* --------------------------------*/
/* function PUSH_TRACE */
/*
 * The purpose of this stack is to trace the sequence of function calls,
 * just like the stack in your computer would do.
 * The "global" string denotes the start of the function call trace.
 * The char *p parameter is the name of the new function that is added to the call trace.
**/
void PUSH_TRACE(char* p)          // push p on the stack
{
  TRACE_NODE* tnode;
  static char glob[]="global";

  if (TRACE_TOP==NULL) {

    // initialize the stack with "global" identifier
    TRACE_TOP=(TRACE_NODE*) malloc(sizeof(TRACE_NODE));

    // no recovery needed if allocation failed, this is only
    // used in debugging, not in production
    if (TRACE_TOP==NULL) {
      printf("PUSH_TRACE: memory allocation error\n");
      exit(1);
    }

    TRACE_TOP->functionid = glob;
    TRACE_TOP->next=NULL;
  }//if

  // create the node for p
  tnode = (TRACE_NODE*) malloc(sizeof(TRACE_NODE));

  // no recovery needed if allocation failed, this is only
  // used in debugging, not in production
  if (tnode==NULL) {
    printf("PUSH_TRACE: memory allocation error\n");
    exit(1);
  }//if

  tnode->functionid=p;
  tnode->next = TRACE_TOP;  // insert fnode as the first in the list
  TRACE_TOP=tnode;          // point TRACE_TOP to the first node

}/*end PUSH_TRACE*/

/* --------------------------------*/
/* function POP_TRACE */
/* Pop a function call from the stack */
void POP_TRACE()    // remove the op of the stack
{
  TRACE_NODE* tnode;
  tnode = TRACE_TOP;
  TRACE_TOP = tnode->next;
  free(tnode);

}/*end POP_TRACE*/



/* ---------------------------------------------- */
/* function PRINT_TRACE prints out the sequence of function calls that are on the stack at this instance */
char* PRINT_TRACE()
{
  int depth = 50; //A max of 50 levels in the stack will be combined in a string for printing out.
  int i, length, j;
  TRACE_NODE* tnode;
  static char buf[100];

  if (TRACE_TOP==NULL) {     // stack not initialized yet, so we are
    strcpy(buf,"global");   // still in the `global' area
    return buf;
  }

  /* peek at the depth(50) top entries on the stack, but do not
     go over 100 chars and do not go over the bottom of the
     stack */

  sprintf(buf,"%s",TRACE_TOP->functionid);
  length = strlen(buf);                  // length of the string so far
  for(i=1, tnode=TRACE_TOP->next;
                        tnode!=NULL && i < depth;
                                  i++,tnode=tnode->next) {
    j = strlen(tnode->functionid);             // length of what we want to add
    if (length+j+1 < 100) {              // total length is ok
      sprintf(buf+length,":%s",tnode->functionid);
      length += j+1;
    }else                                // it would be too long
      break;
  }
  return buf;
} /*end PRINT_TRACE*/


// -----------------------------------------
// function REALLOC calls realloc
// REALLOC should also print info about memory usage.
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void* REALLOC(void* p,int t,char* file,int line)
{
	p = realloc(p,t);
	printf("file = \"%s\", line = %d, function = %s, segment reallocated to address %p to a new size %d\n", file, line, PRINT_TRACE(), (void*)p, t );
	return p;
}

// -------------------------------------------
// function MALLOC calls malloc
// MALLOC should also print info about memory usage.
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void* MALLOC(int t,char* file,int line)
{
	void* p;
	p = malloc(t);
	printf("file = \"%s\", line = %d, function = %s, segment allocated to address %p to a new size %d\n", file, line, PRINT_TRACE(), (void*)p, t );
	return p;
}

// ----------------------------------------------
// function FREE calls free
// FREE should also print info about memory usage.
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void FREE(void* p,char* file,int line)
{
	printf("file = \"%s\", line = %d, function = %s, segment deallocated to address %p\n", file, line, PRINT_TRACE(), (void*)p );
	free(p);
}

#define realloc(a,b) REALLOC(a,b,__FILE__,__LINE__)
#define malloc(a) MALLOC(a,__FILE__,__LINE__)
#define free(a) FREE(a,__FILE__,__LINE__)








//Stores val, index, and next Node
struct linkedNode {
	char* val;
	int index;
	struct linkedNode *next;
};

//node head
static struct linkedNode* head = NULL;

//adds to link list
void addToLinkedList(char* value, int ii) {
	PUSH_TRACE("addToLinkedList"); 
	struct linkedNode* nodeNew = (struct linkedNode*)malloc(sizeof(struct linkedNode)); //Create a new node
	nodeNew->val = value; //Initialize value of new node
	nodeNew->index = ii; //Initialize index of new node
	nodeNew->next = head; //Initialize the node next to new node as head
	head = nodeNew; //Initialize head as new node
	POP_TRACE();
}

//prints nodes
void printLinkedList(struct linkedNode* n) {
	PUSH_TRACE("printLinkedList");
	if (n != NULL) { //If the end of list has not been reached
		printf("linkedList[%d]=%s\n",n->index,n->val); //Print node index and value
		n = n->next; //Iterate list
		printLinkedList(n); //Recursive
	}
	POP_TRACE();
}

void freeLinkedList(struct linkedNode* n) {
	PUSH_TRACE("freeLinkedList");
	while (n != NULL) { //While end of list has not been reached
		struct linkedNode* nafter = n->next; //Initialize nafer to the next node
		free((void*)n); //Free node
		n = nafter; //Iterate list
	}
	POP_TRACE();
}



// -----------------------------------------
// function add_rows
// This function is intended to demonstrate how memory usage tracing of realloc is done
//the method increase the array size by 1
char** add_rows(char** array, int rows) {
	PUSH_TRACE("add_row");

	//initializing temp array
	char** tempArr = realloc(array, sizeof(char*)*(rows+1));
	//if temp array is NULL exits from the program
	if (tempArr == NULL) {
		exit(1);
	}
	else {
		array = tempArr; //setting array to temp array
	}
	
	
	POP_TRACE();
	return array;
}// end add_rows


// ------------------------------------------
// function initializeArray
// This function is intended to demonstrate how memory usage tracing of malloc and free is done
char** initializeArray(int rows) {
	PUSH_TRACE("initializeArray");

	char** array = (char**) malloc(sizeof(char*)*rows);  // make an array with 4 rows
	 
	  
	POP_TRACE();
	return array;
}//end initializeArray

void freeArray(char** array,int rows) {
	PUSH_TRACE("freeArray");
	//now deallocate it
	int i;
	for(i=0; i<rows; i++) //Go through array
		free((void*)array[i]); //Free array element
	free((void*)array); //Free array
	POP_TRACE();
	return;
}

// ----------------------------------------------
// function main
int main()
{
	char fileNameOut[13] = "memtrace.out"; //Copy file name
	int file_desc = open(fileNameOut, O_WRONLY | O_CREAT, 0777); //Create file
	if (dup2(file_desc, STDOUT_FILENO) == -1) { //Invalid file
		return 1;
	}
	
        PUSH_TRACE("main");

	char **array = NULL; //Create string array
	int ROW = 3; //Initialize row
	array = initializeArray(ROW); //Initialize array
	
	size_t length = 0; //Initialize length for reading line
	char* buf = NULL; //Initialize buf for storage

	int counter = 0; //Initialize counter
	while (getline(&buf, &length, stdin) != -1) { //Traverse inputs until user exits
		buf[strlen(buf)-1] = '\0'; //Gets rid of newline character
		if(counter == ROW) { //If counter equals row
			array = add_rows(array, ROW); //Resize array to add more rows
			ROW = ROW + 1;
			array[ROW - 1] = strdup(buf); //Assign string value
		}
		else {
			array[counter] = strdup(buf); //Assign string value
		}
		addToLinkedList(array[counter], counter); //Add to linked list
		counter++; //Increment counter
	}
	
	struct linkedNode* n = head; //Establish head
	printLinkedList(n); //Print list
	
	n = head; //Initialize n as head
	freeLinkedList(n); //Free list
	
	//int i, j;
  //Print array of strings
	int i;
	for(i=0; i<counter; i++){ 
		printf("array[%d]=",i); //Print index
		printf("%s",array[i]); //Print element
		printf("\n"); //Newline
	}
	printf("\n"); //Newline
	
	freeArray(array, ROW); //Free array
	
	
	
	
	free(buf); //Free string input

        POP_TRACE();
        fflush(stdout); //Stops program from printing file
        return(0);
}// end main
