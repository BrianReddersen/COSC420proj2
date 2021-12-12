/*
 * Spencer Lefever
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

//Matrix struct to hold row and col values for Adjacency matrix
matrix A;
A.rows = 100;
A.cols = 100;
A.data = NULL;
//A.data = malloc(A.rows*A.cols*sizeof(float));

//Hub score vector
matrix h;
h.rows = A.cols;
h.cols = 1;
h.data = NULL;
//h.data = malloc(h.rows*h.cols*sizeof(float));


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
float* vecBufh;
//Fill hub score data file with vector
vecBufh = malloc(h.rows*sizeof(float)); //Hub score
//Set all values in vector to 1
for(int i=0; i<h.rows; i++)
{
 vecBufh[i] = 1.0;
}

//Every process opens file only root writes data to file
//char* fname = "hubVec.data";
MPI_File_open(world, "hubVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);


if(myRank==0)
{
MPI_File_write(fh, vecBufh, h.rows*h.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
}

MPI_File_close(&fh);

free(vecBufh);
//-----------------End output vector to file---------------------


float scalar;	//Scalar for eigenvector
float max;
float* multBuf = NULL;
float* rowBufA = NULL;


//------------------------------Begin Hub Vector Calculation-------------------------------------


 vecBufh = malloc(h.rows*sizeof(float));
 //read in hub score vector
 MPI_File_open(world, "hubVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
 MPI_File_read(fh, vecBufh, h.rows*h.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
 MPI_File_close(&fh);
/*
printf("Rank %d here 1\n", myRank);
MPI_Barrier(world);
printf("Rank %d here\n", myRank);
*/

//---------WHILE !CONVERGE----------
for(int j=0; j<20; j++)
{

 //malloc multBuf for calculation
 multBuf = malloc(rowcts[myRank]*sizeof(float)); 
 
 //------------Ly in parrallel------------
 //For loop for A*h calculation for every row of A assigned to process 
 //printf("Process %d beginning first matrix mult\n", myRank); 
 
 //MPI_Barrier(world);
 //printf("rank %d Before open file\n", myRank);
 MPI_File_open(world, "adjacencyMat.data", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
 //printf("rank %d after open file\n", myRank);
 for(int i=0; i<rowcts[myRank]; i++)
 {
  //Malloc rowBufA on each process
  rowBufA = malloc(A.cols*sizeof(float)); 
  //Read in the current row 
  MPI_File_read_at(fh, (rowDispls[myRank]+i)*A.cols*sizeof(float), rowBufA, A.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
   
  //hold results of inner product in multBuf
  multBuf[i] = (float)ipMatrix(rowBufA, vecBufh, h.rows);
  free(rowBufA);
 }
 //Close file
 MPI_File_close(&fh);
 //GATHER RESULTS OF multBuf on vecBufh
 MPI_Allgatherv(
  multBuf, //sendbuf
  rowcts[myRank], //sendcount
  MPI_FLOAT, //sendType
  vecBufh, //recvBuf
  rowcts, //recvCounts (int array)
  rowDispls, //displs (int array)
  MPI_FLOAT, //recvType
  world); //comm
 
 //printf("After Gather %d\n", myRank);

 //----------------(L^T)x; x=Ly---------------------
 MPI_File_open(world, "adjacencyMatTPose.data", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
 for(int i=0; i<rowcts[myRank]; i++)
 {

  rowBufA = malloc(A.cols*sizeof(float)); 
  //Read in the current column 
   
  MPI_File_read_at(fh, (rowDispls[myRank]+i)*A.cols*sizeof(float), rowBufA, A.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
  
  //Do inner product calculation 
  multBuf[i] = (float)ipMatrix(rowBufA, vecBufh, h.rows);
  free(rowBufA);
 }
 MPI_File_close(&fh);

//----------------Normalization process---------------------
 
 //Find the max on the current processor 
 max=multBuf[0];
 for(int i=1; i<rowcts[myRank]; i++)
 {
  if(multBuf[i] > max) max = multBuf[i];
 }
 //Reduce to find overall max
 MPI_Allreduce(&max, &scalar, 1, MPI_DOUBLE, MPI_MAX, world);

 //Normalize vector
 for(int i=0; i<rowcts[myRank]; i++)
 {
  multBuf[i] *= (1/scalar);
 }

 
 //Store new eigenvector in respective data file
 MPI_File_open(world, "hubVec.data", MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

 /*
 MPI_File_set_view(fh, rowDispls[myRank]*sizeof(float), MPI_FLOAT, MPI_FLOAT, "native", MPI_INFO_NULL);
 MPI_File_write(fh, multBuf, rowcts[myRank], MPI_FLOAT, MPI_STATUS_IGNORE);  
 */
 //                    offset                           buf      send count      type 
 MPI_File_write_at(fh, rowDispls[myRank]*sizeof(float), multBuf, rowcts[myRank], MPI_FLOAT, MPI_STATUS_IGNORE);

 MPI_File_close(&fh);
 
 if(myRank==0) printf("Step %d produces eigenvalue of %5f \n", j, scalar);

 free(multBuf);
}


MPI_Finalize();
return 0;
}


