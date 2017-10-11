#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include "indexer.h"


void trie_destroy(TrieNode *root){
	if (root == NULL){
		return;
	}

	trie_destroy(root->child);
	trie_destroy(root->sibling);

	OccNode *occ = root->occurrences;
	while (occ != NULL){
		OccNode *temp = occ;
		occ = occ->next;
		free(temp->file_name);
		free(temp);
	}

	free(root);
}

void index_destroy(Index *index){
	if (index == NULL) return;

	trie_destroy(index->root);
	free(index);
}
Index *create_index(){
	Index *index = (Index *) malloc(sizeof(Index));
	index->root = createNode('*');
	return index;
}
 
TrieNode *createNode(char letter){
	TrieNode *newNode = (TrieNode *)malloc(sizeof(TrieNode));
	newNode->key = tolower(letter);
	newNode->sibling = NULL;
	newNode->child = NULL;
	newNode->occurrences = NULL;
	newNode->curr_occur = 0;

	return newNode;
}



/*
 * Take in file pointer and places all tokenized words into a Trie. If word already exists, increase count
 */

void read_file(Index *index, FILE *file, char *file_name){
	char buffer[128];
	int i;

	fscanf(file, "%*[^0-9a-zA-Z]");

	while(fscanf(file, "%[0-9a-zA-Z]%*[^0-9a-zA-Z]", buffer) == 1){
		TrieNode *temp = index->root;
		for(i = 0; buffer[i]; i++){
			temp = get_child(temp, tolower(buffer[i]), 1);
		}
		temp->curr_occur += 1;
	}

	file_count(index, file_name);
}

void file_count(Index *index, char *file_name)
{
	count_helper(index->root, file_name);
}

OccNode *create_occur(char *file_name, int count){
	OccNode *occ = (OccNode *) malloc(sizeof(OccNode));
	occ->file_name = (char *) malloc(sizeof(char) * (strlen(file_name) + 1));
	strcpy(occ->file_name, file_name);
	occ->count = count;
	occ->next = NULL;

	return occ;
}

void count_helper(TrieNode *node, char *file_name){
	if (node == NULL){
		return;
	}

	if (node->curr_occur > 0){
		OccNode *occ = create_occur(file_name, node->curr_occur);
		if (node->occurrences == NULL || occ->count > node->occurrences->count){
			occ->next = node->occurrences;
			node->occurrences = occ;
		} else {
			OccNode *prev = node->occurrences;
			OccNode *curr = prev->next;
			while (curr != NULL && (occ->count < curr->count)) {
				prev = curr;
				curr = curr->next;
			}
			occ->next = curr;
			prev->next = occ;
		}
	}

	node->curr_occur = 0;

	count_helper(node->child, file_name);
	count_helper(node->sibling, file_name);
}




/*
 * Returns a pointer to the child of the parent node, if create is not equal to NULL. Otherwise, returns null
 */
TrieNode *get_child(TrieNode *parent, char letter, int create){
	TrieNode *prev, *curr, *temp;
	letter = tolower(letter);

	if (parent->child == NULL) {
		if (create){
			parent->child = createNode(letter);
			return parent->child;
		} else {
			return NULL;
		}	
	}

	if (parent->child->key > letter){
		if (create){
			temp = parent->child;
			parent->child = createNode(letter);
			parent->child->sibling = temp;
			return parent->child;
		} else {
			return NULL;
		}
	}

	prev = NULL;
	curr = parent->child;

	while (curr != NULL){

		if (curr->key == letter){
			return curr;
		} else if (curr->key > letter){
			if (create){
				prev->sibling = createNode(letter);
				prev->sibling->sibling = curr;
				return prev->sibling;
			} else {
				return NULL;
			}
		}
		prev = curr;
		curr = curr->sibling;
	}

	if (create){
		prev->sibling = createNode(letter);
		return prev->sibling;
	} else {
		return NULL;
	}
}

void print_index(FILE *out, Index *index){
	char buffer[128];
	print_index_help(out, index->root->child, buffer, 0);
}

/*
 * Helper fuction that recursivley prints out the trie
 */
void print_index_help(FILE *out, TrieNode *root, char *buffer, int index){
	if (root == NULL) return;

	buffer[index] = root->key;

	if (root->occurrences != NULL){
		buffer[index + 1] = '\0';
		fprintf(out, "\t<word text=\"%s\"", buffer);
		fprintf(out, ">\n");
		OccNode *occ = root->occurrences;
		int count = 0;
		while (occ != NULL){
			fprintf(out, "\t\t <file name=");
			fprintf(out, "\"%s\"", basename(occ ->file_name));
			fprintf(out, ">");
			fprintf(out, "%d", occ -> count);
			fprintf(out, "</file>\n");
			occ = occ->next;
			count++;
			if (count % 5 == 0){
				//fprintf(out, "\n");
			}
		}
		if (count % 5 != 0){
			//fprintf(out, "\n");
		}
		fprintf(out, "\t</word>\n");

	}

	print_index_help(out, root->child, buffer, index + 1);
	
	buffer[index] = '\0'; /* not really necessary, will get overwritten anyway */

	print_index_help(out, root->sibling, buffer, index);
}


/* Recursivly reads either a directory or file. If dir, continues to go deeper, until all files are reached. */
void recursive_read_file(Index *index, char *dir_name){
	DIR *dir;
	struct dirent *target;

	if (!(dir = opendir(dir_name)) || !(target = readdir(dir))) {
        return;
	}

	do {
	char path[1024];
        int len = snprintf(path, sizeof(path)-1, "%s/%s", dir_name, target->d_name);
        path[len] = 0;

        if (target->d_type == DT_DIR) {
            if (strcmp(target->d_name, ".") == 0|| strcmp(target->d_name, "..") == 0) {
                continue;
            }
            recursive_read_file(index, path);
        } else if (target->d_name[0] != '.') {
            FILE *to_invert = fopen(path, "r");
            read_file(index, to_invert, path);
            fclose(to_invert);
        }
    } while ((target = readdir(dir)));

    closedir(dir);
}

/* MAIN, reads in string as path name */

int main(int argc, char const *argv[]){
	Index *index = create_index();
	struct stat s;

	if (argc < 3) {
		fprintf(stderr, "invalid input\n");
		return -1;
	}

	if(stat(argv[2], &s) == 0) {
	    if(s.st_mode & S_IFDIR) {
			recursive_read_file(index, (char *) argv[2]);
	    } else if(s.st_mode & S_IFREG) {
	    	FILE *to_invert = fopen(argv[2], "r");
	    	read_file(index, to_invert, (char *) argv[2]);
	    	fclose(to_invert);
	    } else {
	    	printf("error - '%s' is not a file or directory\n", argv[2]);
	    	index_destroy(index);
	    	return -1;
	    }
	} else {
	    printf("error - '%s' is not a file or directory\n", argv[2]);
	    index_destroy(index);
	    return -1;
	}

	char file_name[64];
	strcpy(file_name, argv[1]);
	FILE *invert_file = fopen(file_name, "r");

	while (invert_file){
		fclose(invert_file);
		char answer;
		printf("A file with the name %s already exists. Overwrite? (y/n) ", argv[1]);
		scanf(" %c", &answer);
		if (answer == 'y'){
			break;
		} else {
			printf("Enter a different file name: ");
			scanf("%s", file_name);
			invert_file = fopen(file_name, "r");
		}
	}

	invert_file = fopen(file_name, "w");
	fprintf(invert_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(invert_file, "<fileIndex>\n");

	print_index(invert_file, index);

	fprintf(invert_file, "<fileIndex>\n");
	fclose(invert_file);

	index_destroy(index);

	return 0;
}


