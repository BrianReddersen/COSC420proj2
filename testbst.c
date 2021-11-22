#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include "arxiv.h"

// sample driver code to show how the final product will function

int main(){
	FILE *fp = fopen("searchTree", "r");
 	char *line = NULL;
 	int bufsiz;
	printf("building search tree from file...\n");
 	getline(&line, &bufsiz, fp);
 	char key[50];
 	strcpy(key, tokenize_key(line));
 	char *data = tokenize_data(line);
	struct node *root = NULL;
	root = insert(root, key, data);
	while (getline(&line, &bufsiz, fp) != EOF){
		strcpy(key, tokenize_key(line));
		data = tokenize_data(line);
		printf("inserting %s %s", key, data);
		insert(root, key, data);
	}
	printf("done\n");
	printTree(root);
	printf("____________________\n");
	insert(root, "zetagundam", "zetagundam2\n");
	insert(root, "apple", "test string 4");
	insert(root, "eigenvalue", "asdfgh");
	printTree(root);
	search(root, "apple");
// 	printf("root key %s\n", test->key);
	fclose(fp);
}
