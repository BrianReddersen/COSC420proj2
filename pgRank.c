/* Spencer Lefever
 * COSC420 Proj2
 *
 * principle eigenvector/value calculated using the power method
 * this iteration uses MPI_File for communication
 *
 * General Variable Naming guide
 * 1. matrices: matrix [A-Z]
 * 2. vectors: matrix [a-z]
 * 3. matrix buffer: float* rowBuf[A-Z] (Can also be used to store columns of sqaure matrix)
 * 4. vector buffer: float* vecBuf[a-z]
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

matrix p;
p.rows = A.cols;
p.cols = 1;
p.data = NULL;
//p.data = malloc(p.rows*p.cols*sizeof(float));

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


//---------------------Begin output vectors to file---------------------

MPI_File fh;

//Fill pahe rank data file with vector
float* vecBufp = malloc(p.rows*sizeof(float));
//Set all values in vector to 1
for(int i=0; i<p.rows; i++)
{
 vecBufp[i] = 1.0;
}

MPI_File_open(world, "pgRankVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

if(myRank==0) MPI_File_write(fh, vecBufp, p.rows*p.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
MPI_File_close(&fh);

//Free buffer used for file input
free(vecBufp);

//-----------------End output vector to file---------------------


float* multBuf = NULL;
float* rowBufA = NULL;


/* Page rank algorithm
 * p^k = M*p^k-1 (power method)
 * M is adj matrix but col of M sum to 1
 * 
 * Algortihm
 *
 * Make M stochastic (col sum to 1)
 * Store stochastic M in data file
 *
 * while(!converge)
 *  pull p from data file into buffer*
 *  for i<rowcts[myRank]
 *   p=M*p^(k-1)
 *   write new p into data file
 *  end for
 * end while
 */


vecBufp = malloc(p.rows*sizeof(float));
//read in hub score vector
MPI_File_open(world, "pgRankVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
MPI_File_read(fh, vecBufp, p.rows*p.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
MPI_File_close(&fh);

//For loop for power method instead of while loop testing convergence
for(int j=0; j<20; j++)
{
 //malloc buffer to hold authority score vector
 multBuf = malloc(p.rows*sizeof(float));
 
//For loop to perform M*p for every row in M
 for(int i=0; i<rowcts[myRank]; i++)
 { 
  MPI_File_open(world, "stochasticMat.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
  //Read in current row on respective process 
  MPI_File_read_at(fh, (rowDispls[myRank]+i)*A.cols*sizeof(float), rowBufA, A.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
  MPI_File_close(&fh);
 
  multBuf[i] = (float)ipMatrix(rowBufA, vecBufp, p.rows);

/*
  //Normalize the vector before writing to file
  max=multBuf[0];
  for(int i=1; i<rowcts[myRank]; i++)
  {
   if(multBuf[i] > max) max = multBuf[i];
  }
  MPI_Allreduce(&max, &scalar, 1, MPI_DOUBLE, MPI_MAX, world);
  for(int i=0; i<rowcts[myRank]; i++)
  {
   multBuf[i] *= (1/scalar);
  }
*/


 //All_gahter multBuf into vecBufP
 MPI_Allgatherv(multBuf, rowcts[myRank], MPI_FLOAT, vecBufp, rowcts, rowDispls, MPI_FLOAT, world);

 //Each process print multBuf into their part of the file 
 MPI_File_open(world, "pgRankVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
 MPI_File_write_at(fh, rowDispls[myRank]*sizeof(float), multBuf, rowcts[myRank], MPI_FLOAT, MPI_STATUS_IGNORE);
 MPI_File_close(&fh);
  

 }

 free(multBuf);
}


MPI_Finalize();
return 0;
}











