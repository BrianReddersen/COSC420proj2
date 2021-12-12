/*
 * Spencer Lefever
 * COSC420 Proj2
 * This file takes the adjacency matrix and stores the transposed version in a separate file
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
 A.rows = 1000;
 A.cols = 1000;
 A.data = NULL;

 int* rowcts = malloc(worldSize*sizeof(int));
 int* rowDispls = malloc(worldSize*sizeof(int));
 calcRowcts(rowcts, worldSize, A.rows);
 calcRowDispls(rowDispls, rowcts, worldSize);

 MPI_Datatype col, coltype;
 MPI_Type_vector(A.rows, 1, A.cols, MPI_FLOAT, &col);
 MPI_Type_commit(&col);

 MPI_Type_create_resized(col, 0, sizeof(float), &coltype);
 MPI_Type_commit(&coltype);

 MPI_File fh;

 float* rowBufA;

 for(int i=0; i<rowcts[myRank]; i++)
 {
 rowBufA = malloc(A.cols*sizeof(float));
 //Read in column of adj mat from file
 MPI_File_open(world, "adjacencyMat.data", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
 MPI_File_set_view(fh, (rowDispls[myRank]+i)*sizeof(float), MPI_FLOAT, coltype, "native", MPI_INFO_NULL);
 MPI_File_read(fh, rowBufA, A.rows, MPI_FLOAT, MPI_STATUS_IGNORE);
 MPI_File_close(&fh);
 //Output columns as rows to separate file
 MPI_File_open(world, "adjMatTPose.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
 MPI_File_write_at(fh, (rowDispls[myRank]+i)*A.cols*sizeof(float), rowBufA, A.rows, MPI_FLOAT, MPI_STATUS_IGNORE);
 MPI_File_close(&fh);

 free(rowBufA);
 }

 MPI_Finalize();
 return 0;
}









