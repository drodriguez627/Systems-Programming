typedef struct OccurrenceStruct {
	char *file_name;
	int count;
	struct OccurrenceStruct *next;
} OccNode;

typedef struct TrieNodeStruct {
    char key;
    struct TrieNodeStruct *sibling;
    struct TrieNodeStruct *child;
    OccNode *occurrences;
    int curr_occur;
} TrieNode;

typedef struct {
	TrieNode *root;
} Index;


void index_destroy(Index *index);

void trie_destroy(TrieNode *root);

TrieNode *createNode(char letter);

Index *create_index();

void read_file(Index *index, FILE *dict_file, char *file_name);

void file_count(Index *index, char *file_name);

void count_helper(TrieNode *node, char *file_name);

OccNode *create_occur(char *file_name, int count);

TrieNode *get_child(TrieNode *parent, char letter, int create);

void print_index(FILE *out, Index *index);

void print_index_help(FILE *out, TrieNode *root, char *buffer, int index);
