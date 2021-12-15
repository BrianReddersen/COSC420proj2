#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

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
	A->data = calloc(rows*cols, sizeof(double));
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
	MPI_Init(NULL, NULL);
	MPI_Comm world = MPI_COMM_WORLD;
	int rank;
	int worldsize;
	MPI_Comm_size(world, &worldsize);
	MPI_Comm_rank(world, &rank);
	MPI_File fh;
	matrix m;
	int file_length = 1628118;
	int file_break_point = 1354752;
	initMatrix(&m, 1, file_length);
	int i = 0;
	int mode = 0;
	int ind = 0;
	int start_point = rank*(file_length/worldsize);
	char *line = malloc(100*sizeof(char));
	char *cmp_line = malloc(100*sizeof(char));
	char *file_line = malloc(100*sizeof(char));
	char *temp = malloc(100*sizeof(char));
	size_t bufsiz;
	size_t bufsiz2;
	FILE *stream = fopen("arxiv-citations.txt", "r");
	FILE *indexes = fopen("indexes", "r");

	printf("%d\n", m.cols);	
	printf("rank: %d working on %d - %d\n", rank, rank*(file_length/worldsize), (rank + 1)*(file_length/worldsize));
	
	// indexes file is in a different order than the citations, i'll do some fuckery once the
	// hits and pageranks are done to fix it
	MPI_File_open(world, "adjacencyMat.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
	//change 2 to file_length for full matrix
	for(i = 0; i < (rank+1)*(file_length/worldsize); i){
		while (i < start_point){
			getline(&line, &bufsiz, stream);
			if (!strcmp(line, "+++++\n")) i++;
		}
		MPI_Barrier(world);
		memset(m.data, 0, file_length * sizeof(float));
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
		//since citation file is broken, enter a row of zeroes after the break point
		if (i < file_break_point){
			if (mode == 1){
				while (getline(&file_line, &bufsiz2, indexes) != EOF){
					ind = atoi(tokenize_index(file_line));
					temp = tokenize_id(file_line);
					if (!strcmp(temp, line)){
						ACCESS(m,0,ind) = 1;
					}
				}
			}
		}
		MPI_File_write_at(fh, i*m.cols*sizeof(float), m.data, file_length, MPI_FLOAT, MPI_STATUS_IGNORE);
	}
// 	for (int i = 0; i < 100; i++){
// 		for (int j = 0; j < 100; j++){
// 			printf("%14f", ACCESS(m,i,j));
// 		}
// 		printf("\n");
// 	}
	
	
	MPI_File_close(&fh);
	fclose(stream);
// 	fclose(out);
	free(line);
	free(cmp_line);
// 	free(temp);
	MPI_Finalize();
	return 0;
}
