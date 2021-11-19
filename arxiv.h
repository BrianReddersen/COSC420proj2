#ifndef ARXIV_H
#define ARXIV_H

#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>

typedef struct matrix{
	int rows;
	int cols;
	//determine if data is
	//doubl* data
	//char* data
	//int* data
}
void initMatrix(matrix* A, int rows, int cols);

// methods for creating and maintaining the document search index
	// method that reads through file and creates a backward index
	// method(s) for storage(BST hash table etc)
// methods to create and maintain an edge graph of the network
	// build adjacency graph for paper citation network
	// compute page rank hub score and authority score and store them to be used by engine later

#endif
