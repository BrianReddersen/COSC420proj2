#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "matrix.h"


#define INDEX(i,j,n,m) i*m + j
#define ACCESS(A,i,j) A.data[INDEX(i,j,A.rows,A.cols)]

//builds a NxN adjacency matrix for the first N files in the arxiv_metadata file
//change N by changing the numbers in the initmatrix call and for loop

typedef struct matrix{
	int rows;
	int cols;
	float *data;
} matrix;

void initMatrix(matrix *A, int rows, int cols){
	A->rows = rows;
	A->cols = cols;
	A->data = calloc(rows*cols, sizeof(float));
}

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

//builds an adjacency matrix for tht first 1000 entries in the citation file
int main(){
	matrix m;
	initMatrix(&m, 1000, 1000);
	int i = 0;
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
					ACCESS(m,i,ind) = 1;
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
