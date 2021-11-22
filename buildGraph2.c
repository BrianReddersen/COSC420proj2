/*
 * buildGraph2.c is second iteration of buildGraph.c
 * Additions/Changes
 * 1. Matrix is store in MPI_FILE
 *
 * 2. HITS algorithm to produce authority and hub score vector
 *
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include "matrix.h"


#define INDEX(i,j,n,m) i*m + j
#define ACCESS(A,i,j) A.data[INDEX(i,j,A.rows,A.cols)]

//builds a NxN adjacency matrix for the first N files in the arxiv_metadata file
//change N by changing the numbers in the initmatrix call and for loop

typedef struct matrix
{
	int rows;
	int cols;
	float *data;
} matrix;
//Function not used to row and col values 
//can be changed easily and used in for loops
//rather than hard coded numbers
/*
void initMatrix(matrix *A, int rows, int cols){
	A->rows = rows;
	A->cols = cols;
	A->data = calloc(rows*cols, sizeof(float));
}
*/
char *tokenize_index(char *line){
	char cpy[100];
	strcpy(cpy, line);
	char *token = strtok(cpy, ":");
	return token;
}

char *tokenize_id(char *line){
	char cpy[100];
	strcpy(cpy, line);
	char *token = strtok(cpy, ":");
	token = strtok(NULL, ":");
	return token;
}






int main(int argc, char** argv)
{
MPI_Init(&argc, &argv);
int worldSize, myRank;
MPI_Comm world = MPI_COMM_WORLD;
MPI_Comm_size(world, &worldSize);
MPI_Comm_rank(world,&myRank);


//Adjacency matrix 
//matrix will be malloced with correct amount of distributed rows
matrix A;
//A is a square matrix where A.rows = A.cols = number of database entries
A.rows = 100;
A.cols = 100;
A.data = NULL;
//MPI_Datatype for rows and columns of adjacency matrix
MPI_Datatype row;
MPI_Type_contiguous(A.cols, MPI_FLOAT, &row);
MPI_Type_commit(&row);

//column datatype needs a resize type as well
//IF USING COLUMN DATAYPTE USE coltype VARIABLE NOT col
MPI_Datatype col, coltype;
MPI_Type_vector(A.rows, 1, A.cols, MPI_FLOAT, &col)
MPI_Type_commit(&col);
//resize col
MPI_Type_create_resized(col, 0, sizeof(float), &coltype);
MPI_Type_commit(&coltype);



//Arrays to hold the row count and the row displs of each rank
//Function calcRowcts and calcRowDispls are found in matrix.h 
int* rowcts = malloc(worldSize*sizeof(int));
int* rowDispls = malloc(worldSize*sizeof(int));
calcRowcts(rowcts, worldSize, A.rows);
calcRowDispls(rowDispls, rowcts, worldSize);

//malloc A.data after row calculation is made
A.data = malloc(rowcts[myRank]*A.cols*sizeof(float));




//MPI_File to store data
char* fname = "adjacencyMat.data";
MPI_File fhA;
MPI_File_open(world, fname, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fhA); 

//have one node create MPI_File for adj matrix
//and one node create MPI_File for indexes 


	int mode = 0;
	int ind = 0;
	char *line = malloc(50*sizeof(char));
	char *cmp_line = malloc(50*sizeof(char));
	char *file_line = malloc(50*sizeof(char));
	char *temp = malloc(50*sizeof(char));
	size_t bufsiz;
	size_t bufsiz2;
	FILE *stream = fopen("arxiv-citations.txt", "r");
	FILE *indexes = fopen("indexes", "r");
	
	//Buffer to hold current row and 
	//for every entry in file
	//After every row input the row to the file
	for (i = 0; i < A.rows; i)
	{
	//Malloc and memset rowBuf for new row in matrix/for loop
	float* rowBuf = malloc(A.cols*sizeof(float));
	memset(rowBuf, 0, A.cols*sizeof(float) )
		

		getline(&line, &bufsiz, stream);
		if (!strcmp(line, "-----\n"))
		{
			mode = 1;
			continue;
		}
		else if (!strcmp(line, "+++++\n"))
		{
			mode = 0;
			i++;
			continue;
		}
		if (mode == 1)
		{       
			while (getline(&file_line, &bufsiz2, indexes) != EOF)
			{
				ind = atoi(tokenize_index(file_line));
				temp = tokenize_id(file_line);
				if (!strcmp(temp, line))
				{
					//ACCESS(A,i,ind) = 1.0;
					rowBuf[ind] = 1.0;
				}
			}
			//Output row to file
			MPI_File_write_at(fh, //handle
			i*A.cols*sizeof(float), //offset
			rowBuf, //buffer
			A.cols, //count
			MPI_FLOAT, MPI_STATUS_IGNORE);	
		}
 	 free(rowBuf);	
	}
	fclose(stream);
	fclose(indexes);

MPI_Finalize();
return 0;
}








/*
//builds an adjacency matrix for tht first 1000 entries in the citation file
int main()
{


	//int i = 0;
	int mode = 0;
	int ind = 0;
	char *line = malloc(50*sizeof(char));
	char *cmp_line = malloc(50*sizeof(char));
	char *file_line = malloc(50*sizeof(char));
	char *temp = malloc(50*sizeof(char));
	size_t bufsiz;
	size_t bufsiz2;
	FILE *stream = fopen("arxiv-citations.txt", "r");
	FILE *indexes = fopen("indexes", "r");
	


	//for 
	for (i = 0; i < 100; i){
		getline(&line, &bufsiz, stream);
		if (!strcmp(line, "-----\n")){
			mode = 1;
			continue;
		}
		else if (!strcmp(line, "+++++\n")){
			mode = 0;
			i++;
			continue;
		}
		if (mode == 1){
			while (getline(&file_line, &bufsiz2, indexes) != EOF){
				ind = atoi(tokenize_index(file_line));
				temp = tokenize_id(file_line);
				if (!strcmp(temp, line)){
					ACCESS(A,i,ind) = 1.0;
				}
			}
		}
	}
	for (int i = 0; i < 100; i++){
		for (int j = 0; j < 100; j++){
			printf("%d", ACCESS(m,i,j));
		}
		printf("\n");
	}
	fclose(stream);
// 	fclose(out);
	free(line);
	free(cmp_line);
// 	free(temp);
}
*/
