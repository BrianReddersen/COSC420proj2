/* Spencer Lefever
 * COSC420Proj2
 * Program to create stochastic matrix
 * outputting to a file
 * tPoseAdjMat.c MUST BE RAN FIRST!!!!!!!!!!!!!!
 */

#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include"matrix.h"


typedef struct matrix
{
 int rows;
 int cols;
 float* data;
} matrix;

int main(int argc, char** argv)
{
MPI_Init(&argc, &argv);
int worldSize, myRank;
MPI_Comm world = MPI_COMM_WORLD;
MPI_Comm_size(world, &worldSize);
MPI_Comm_rank(world, &myRank);


matrix A;
A.rows = 100;
A.cols = 100;
A.data = NULL;
//A.data = malloc(A.rows*A.cols*sizeof(float));


//Calculate row counts and row displacements 
//FUnction used found in matrix.h
int* rowcts = malloc(worldSize*sizeof(int));
int* rowDispls = malloc(worldSize*sizeof(int));
calcRowcts(rowcts, worldSize, A.rows);
calcRowDispls(rowDispls, rowcts, worldSize);

MPI_Datatype row; 
MPI_Type_contiguous(A.cols, MPI_FLOAT, &row);
MPI_Type_commit(&row);


//MPI_Datatype for columns of adj mat
MPI_Datatype col, coltype;
MPI_Type_vector(A.rows, 1, A.cols, MPI_FLOAT, &col);
MPI_Type_commit(&col);
//Resize col datatype to coltype
MPI_Type_create_resized(col, 0, sizeof(float), &coltype);
MPI_Type_commit(&coltype);

MPI_File fh;

float* rowBufA = NULL;

//For loop for each process to take a col and make it stochastic
for(int i=0; i<rowcts[myRank]; i++)
{
 rowBufA = malloc(A.cols*sizeof(float));
 MPI_File_open(world, "adjacencyMatTPose.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
 
 //Read column of matrix 
 MPI_File_read_at(fh, (rowDispls[myRank]+i)*A.cols*sizeof(float), rowBufA, A.cols, MPI_FLOAT, MPI_STATUS_IGNORE); 

 MPI_File_close(&fh);

 //For loop to sum entries in column
 float sum=0;
 for(int j=0; j<A.rows; j++)
 {
  sum += rowBufA[i];
 }
 //For loop to convert to stochastic  
 for(int j=0; j<A.rows; j++)
 {
  rowBufA[i] = rowBufA[i] / sum;
 }

 //Open file to store the stochastic matrix data
 MPI_File_open(world, "stochasticMat.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

 //Set view for processes to write cols
 MPI_File_set_view(fh, (rowDispls[myRank]+i)*sizeof(float), MPI_FLOAT, coltype, "native", MPI_INFO_NULL);

 //Write current col to file
 MPI_File_write(fh, rowBufA, A.rows, MPI_FLOAT, MPI_STATUS_IGNORE); 

 //Close file
 MPI_File_close(&fh);
}



MPI_Finalize();
return 0;
}











