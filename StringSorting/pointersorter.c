#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/*node struct for linked list*/
typedef struct _Node
{
	char * token;
	struct _Node * next;
} Node;

/* Objective: Prints out a linked list from the first node to the last node.
*  Parameters: Node * head
*  Return: void
*/
void printList(Node * head)
{
	Node * currentNode = head;

	if (head != NULL)
	{
		while (currentNode != NULL)
		{
			if (currentNode->token != NULL)
			{
				printf("%s \n", currentNode->token);
			}
			currentNode = currentNode->next;
		}
	}
}

/* Objective: Swaps the tokens inside two nodes with each other.
*  Parameters: Node * currNode
*	       Node * nxtNode
*  Return: void
*/
void swap(Node * currNode, Node * nxtNode)
{
	char * tempToken = currNode->token;
	currNode->token = nxtNode->token;
	nxtNode->token = tempToken;
}

/* Objective: Sorts a linked list using a bubble sort algorithm. The function
*	      walks through the linked list multiple times, checking if tokens
*	      in adjacent nodes are in the right order and swaps them if they
*	      aren't.
*  Parameters: Node * start
*  Return: void
*/
void sortList(Node * start)
{
	int swapped = 1;
	int numElemsCompared = 0;	

        Node * currentNode = start;
	Node * topNode = start;

	if (start != NULL && start->next != NULL)
	{
		while (swapped)
		{
			swapped = 0;
			currentNode = topNode;
			while (currentNode->next->next != NULL)
			{
				//set the maximum amount of characters to compare between the two tokens
				if (strlen(currentNode->token) > strlen(currentNode->next->token))
				{          
					numElemsCompared = strlen(currentNode->next->token);
				} else {
					numElemsCompared = strlen(currentNode->token);
				}
				
				//compare the current token and the next token
				if (strncmp(currentNode->token, currentNode->next->token, numElemsCompared) > 0)
				{
					swap(currentNode, currentNode->next);
					swapped = 1;
				} else if (strncmp(currentNode->token, currentNode->next->token, numElemsCompared) == 0) {
					if (strlen(currentNode->token) > strlen(currentNode->next->token))
					{
						swap(currentNode, currentNode->next);
                                        	swapped = 1;
					}
				}
				
				//advance to next node
				if (currentNode->next != NULL)
				{
					currentNode = currentNode->next;
				}
			}
		}
	}
}

/* Objective: Insert a new node into the linked list and push
*	      any nodes currently in the list to the right.
*  Parameters: Node ** start_ref
*	       char * string
*  Return: void
*/
void insertNode(Node ** startRef, char * string)
{
	Node * ptr = (Node *) malloc(sizeof(Node));
	ptr->token = string;

	ptr->next = * startRef;
	* startRef = ptr;
}

/* Objective: A custom calloc function. It includes an error check to see if calloc
*	      was able to allocate memory and exits the program if it wasn't able to.
*  Parameters: size_t nmemb
*	       size_t size
*  Return: void *
*/
void * ecalloc(size_t nmemb, size_t size)
{
	void * ptr = calloc(nmemb, size);

	if (ptr == NULL)
	{
		fprintf(stderr, "Unable to calloc memory. Exiting program.\n");
		exit(EXIT_FAILURE);
	}

	return ptr;
}

/* Objective: Breaks up the single string input into N amount of tokens and stores
*	      them in a linked list.
*  Parameters: char * inputString
*  Return: Node *
*/
Node * breakUpInput(char * inputString)
{
	//loop variable
	int i = 0;

	//positions used to keep track of where in the input string the
	//loop currently is; also used to determine the size of a token
	//recieved from input
        int startPos = 0;
        int currentPos = startPos;

	//temporary string to holds characters in to put in linked list
	char * tempTok = NULL;
	//used in loop when putting chars into the temporary string
	int tempTokPos = 0;
	//size of a token from input string
	int size = 0;

	//linked list where the tokens will be stored	
	Node * stringTokens;
        stringTokens = (Node *) malloc(sizeof(Node));
        stringTokens->next = NULL;

	while (inputString[currentPos] != '\0')
	{
		if (isalpha(inputString[currentPos]) != 0 && inputString[currentPos] != '\0')
		{
			//advance position in input string
			currentPos++;
		} else {
			//size of token
			size = currentPos - startPos;
			if (size != 0)
			{
				//copy into a string
				tempTok = (char *) ecalloc(size, size * sizeof(char) + 1);
				tempTokPos = 0;
				
				for (i = startPos; i < currentPos; i++)
				{
					tempTok[tempTokPos] = inputString[i];
					tempTokPos++;
				}
				
				insertNode(&stringTokens, tempTok);
			}
			currentPos++;
			startPos = currentPos;
		}
	}
	//last token
	size = currentPos - startPos;
	if (size != 0)
	{
		tempTok = (char *) ecalloc(size, size * sizeof(char) + 1);
		tempTokPos = 0;
		for (i = startPos; i < currentPos; i++)
		{
			tempTok[tempTokPos] = inputString[i];
			tempTokPos++;
		}
		insertNode(&stringTokens, tempTok);
	}
	return stringTokens;
}

/* Objective: A program designed to break apart a single string into individual string
*	      tokens, sort them in alphabetical order and print them out in a list. Tokens
*	      are separated by any non-alphabetical character.
*  Parameters: int argc
*	       char * argv[]
*  Return: int
*/
int main(int argc, char * argv[])
{
	Node * listOfStringTokens;

	if (argc != 2)
	{
		printf("Incorrect number of parameters.\nUsage: ./pointersorter \"astring\"\n");
	} else {
		listOfStringTokens = breakUpInput(argv[1]);
		sortList(listOfStringTokens);
		printList(listOfStringTokens);

		free(listOfStringTokens->next);            
		free(listOfStringTokens);
	}

	return 0;
}
