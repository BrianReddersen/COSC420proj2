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
A.rows = 1628118;
A.cols = 1628118;
A.data = NULL;
//A.data = malloc(A.rows*A.cols*sizeof(float));

//Authority score vector
matrix a;
a.rows = A.cols;
a.cols = 1;
a.data = NULL;
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
float* vecBufa = NULL;
//Fill hub score data file with vector
vecBufa = malloc(a.rows*sizeof(float)); //Hub score
//Set all values in vector to 1
for(int i=0; i<a.rows; i++)
{
 vecBufa[i] = 1.0;
}

//Every process opens file only root writes data to file
//char* fname = "hubVec.data";

MPI_Barrier(world);
MPI_File_open(world, "authVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

if(myRank==0) MPI_File_write(fh, vecBufa, a.rows*a.cols, MPI_FLOAT, MPI_STATUS_IGNORE);

MPI_File_close(&fh);
free(vecBufa);
MPI_Barrier(world);
//-----------------End output vector to file---------------------


float scalar;	//Scalar for eigenvector
float max;
float* multBuf = NULL;
float* rowBufA = NULL;


//------------------------------Begin Authority Vector Calculation-------------------------------------

//read in authority score vector before calculation
vecBufa = malloc(a.rows*sizeof(float));

MPI_Barrier(world);

MPI_File_open(world, "authVec.data", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
MPI_File_read(fh, vecBufa, a.rows*a.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
MPI_File_close(&fh);

MPI_Barrier(world);

//---------WHILE !CONVERGE----------
for(int j=0; j<50; j++)
{

 //malloc multBuf for calculation
 //multBuf is each processes piece of the authority Score vector
 //multBuf is full when each process completes their rows X vecBufa
 multBuf = malloc(rowcts[myRank]*sizeof(float)); 
 
 MPI_Barrier(world);
 //----------------(L^T)y---------------------
 MPI_File_open(world, "adjacencyMatTPose.data", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
 for(int i=0; i<rowcts[myRank]; i++)
 {

  rowBufA = malloc(A.cols*sizeof(float)); 
  //Read in the current column 
   
  MPI_File_read_at(fh, (rowDispls[myRank]+i)*A.cols*sizeof(float), rowBufA, A.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
  
  //Do inner product calculation 
  multBuf[i] = (float)ipMatrix(rowBufA, vecBufa, a.rows);
  free(rowBufA);
 }
 MPI_Barrier(world);
 MPI_File_close(&fh);
 

 //GATHER RESULTS of multBuf on vecBufh for L*x; x=((L^T)y
 MPI_Allgatherv(
  multBuf, //sendbuf
  rowcts[myRank], //sendcount
  MPI_FLOAT, //sendType
  vecBufa, //recvBuf
  rowcts, //recvCounts (int array)
  rowDispls, //displs (int array)
  MPI_FLOAT, //recvType
  world); //comm
  
  free(multBuf);

 //--------------------------------Lx; x=(L^T)y-------------------------------------
 
 MPI_Barrier(world); 
 MPI_File_open(world, "adjacencyMat.data", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
 for(int i=0; i<rowcts[myRank]; i++)
 {
  //Malloc rowBufA on each process
  rowBufA = malloc(A.cols*sizeof(float)); 
  //Read in the current row 
  MPI_File_read_at(fh, (rowDispls[myRank]+i)*A.cols*sizeof(float), rowBufA, A.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
   
  //hold results of inner product in multBuf
  multBuf[i] = (float)ipMatrix(rowBufA, vecBufa, a.rows);
  free(rowBufA);
 }
 //Close file
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

 MPI_Barrier(world); 
 //Store new eigenvector in respective data file
 MPI_File_open(world, "authVec.data", MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
/*
 MPI_File_set_view(fh, rowDispls[myRank]*sizeof(float), MPI_FLOAT, MPI_FLOAT, "native", MPI_INFO_NULL);
 MPI_File_write(fh, multBuf, rowcts[myRank], MPI_FLOAT, MPI_STATUS_IGNORE);  
*/
 //		       offset			         buf      sendcts         type       status
 MPI_File_write_at(fh, rowDispls[myRank]*sizeof(float), multBuf, rowcts[myRank], MPI_FLOAT, MPI_STATUS_IGNORE);

 MPI_File_close(&fh);
 
 printf("Step %d produces eigenvalue of %5f \n", j, scalar);

 free(multBuf);
}


MPI_Finalize();
return 0;
}


