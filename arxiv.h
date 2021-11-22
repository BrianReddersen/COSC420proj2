#ifndef ARXIV_H
#define ARXIV_H

// group members: Spencer Lefever, Cody Murrer, Brian Reddersen

#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>

#define INDEX(i,j,n,m) i*m + j
#define INDEX2(i,j,n,m) j*m + i
#define ACCESS(A,i,j) A.data[INDEX(i,j,A.rows,A.cols)]
#define ACCESSTP(A,i,j) A.data[INDEX2(i,j,A.rows,A.cols)]
#define ACCESSP(A,i,j) A->data[INDEX(i,j,A->rows,A->cols)]

typedef struct matrix{
	int rows;
	int cols;
	double *data;
} matrix;

struct node{
	char *key;
	char *text;
	struct node *left;
	struct node *right;
} node;

void initMatrix(matrix* A, int rows, int cols){
	A->rows = rows;
	A->cols = cols;
	A->data = calloc(rows*cols, sizeof(double));
}

char *tokenize_key(char line[]){
	char cpy[100];
	strcpy(cpy, line);
	char *token = strtok(cpy, "|");
	return token;
}

char *tokenize_data(char line[]){
	char cpy[512];
	strcpy(cpy, line);
	char *token = strtok(cpy, "|");
	token = strtok(NULL, "|");
	return token;
}

struct node *createNode(const char *key, const char *data){
	char temp[50];
	strcpy(temp, key);
	struct node *n = malloc(sizeof(struct node));
	n->text = malloc(strlen(data)+1);
	n->key = malloc(strlen(temp)+1);
	strcpy(n->key, temp);
	strcpy(n->text, data);
	n->left = NULL;
	n->right = NULL;
	return n;
}

// if a new word is found, it is added to the BST and the paper ID it was found in
// is stored in n->text
// if a word is already present in the BST this function appends n->data with the ID of every
// subsequent document it is found in
struct node *insert(struct node *n, char *key, char *data){
	if (!n) return createNode(key, data);
	
	if (!strcmp(key, n->key)){
		char *temp = malloc(strlen(n->text)*sizeof(char));
		strcpy(temp, n->text);
		int nsize = strlen(temp) + strlen(data);
		n->text = realloc(n->text, nsize + 1);
		strcpy(n->text, temp);
		strcat(n->text, data);
		free(temp);
		return n;
	}
	
	if (strcmp(key, n->key) < 0){
		n->left = insert(n->left, key, data);
	} else {
		n->right = insert(n->right, key, data);
	}
	
	return n;
}

struct node *search(struct node *n, char *key){
	if (!strcmp(key, n->key)){
		printf("All papers containing %s: \n%s\n", key, n->text); 
	}
	
	if (strcmp(key, n->key) < 0){
		n->left = search(n->left, key);
	} else if (strcmp(key, n->key) > 0) {
		n->right = search(n->right, key);
	}
	
	return n;
}

//for testing
void printTree(struct node *n){
	if (n){
		printTree(n->left);
		printf("%s - %s\n", n->key, n->text);
		printTree(n->right);
	}
}

//prints the built bst to a file to be read in by the driver code
void fprintTree(struct node *n, FILE *fname){
	if (n){
		fprintf(fname, "%s|%s\n", n->key, n->text);
		fprintTree(n->left, fname);
		fprintTree(n->right, fname);
	}
}
// methods to create and maintain an edge graph of the network
	// spencer has these functions
	// build adjacency graph for paper citation network
	// compute page rank hub score and authority score and store them to be used by engine later

#endif
