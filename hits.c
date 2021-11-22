/*
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

//Authority score vector
matrix a;
a.rows = A.cols;
a.cols = 1;
a.data = NULL;
//a.data = malloc(a.rows*a.cols*sizeof(float));

//Hub score vector
matrix h;
h.rows = A.cols;
h.cols = 1;
h.data = NULL;
//h.data = malloc(h.rows*h.cols*sizeof(float));

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

//Fill authority vector data file with vector
//Adjacency matrix data file was built by buildGraph.c
float* vecBufa = malloc(a.rows*sizeof(float)); //Authority score
//Set all values in vector to 1
for(int i=0; i<a.rows; i++)
{
 vecBufa[i] = 1.0;
}

MPI_File fh;
char* fname = NULL;

fname = "authorityVec.data";

MPI_File_open(world, fname, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

if(myRank==0) MPI_File_write(fh, vecBufa, a.rows*a.cols, MPI_FLOAT, MPI_STATUS_IGNORE);

MPI_File_close(&fh);

//Free buffer used for file input
free(vecBufa);

//Fill hub score data file with vector
float* vecBufh = malloc(h.rows*sizeof(float)); //Hub score
//Set all values in vector to 1
for(int i=0; i<a.rows; i++)
{
 vecBufh[i] = 1.0;
}

fname = "hubVec.data";
MPI_File_open(world, fname, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

if(myRank==0) MPI_File_write(fh, vecBufh, h.rows*h.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
MPI_File_close(&fh);

//Free buffer used for file input
free(vecBufh);


//Fill pahe rank data file with vector
float* vecBufp = malloc(p.rows*sizeof(float));
//Set all values in vector to 1
for(int i=0; i<a.rows; i++)
{
 vecBufp[i] = 1.0;
}

fname = "pgRankVec.data";
MPI_File_open(world, fname, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

if(myRank==0) MPI_File_write(fh, vecBufp, p.rows*p.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
MPI_File_close(&fh);

//Free buffer used for file input
free(vecBufp);

//-----------------End output vector to file---------------------


//double* vectB = malloc(B.rows*B.cols*sizeof(double));
float scalar;	//Scalar for eigenvector
float max;
float* multBuf = NULL;
float* rowBufA = NULL;

//double startTime, stopTime;


//startTime = MPI_Wtime();

//Have each process read and store the full b vector

 //Compenents needed for hits algorithm
 //1. rows need to be read on from adjacency matrix file
 //2. hub and authority score vectors need to be read from respective files
 //3. After each iteration, vectors should be normalized and stored in respective files

/*
 *For authority score L* L^T * vecA
 *	1. Do L^T * vecA then multiply that by L
 *For hub score L^T * L * vecH
 *	2. do L*vecH then multiply by L^T
 */


//For loop will run until the results converge to the principle eigenvalue
//float ipBuf;	//Buffer to hold result of inner product row and col for matrix mult
//First buffer cannot be reused because that data must be preserved for entire matrix mult

/*
 * For loop now instead of while loop
 * eventually implement while loop and boolean test for convergence
 *
 * WHEN CHANGING TO WHILE LOOP
 * 
 * Each vector calculation must have separate while loop
 * because they might converge at different points of iteration 
 */

//------------------------------Begin Hub Vector Calculation-------------------------------------
for(int j=0; j<20; j++)
{
 //re-malloc the buffers for use
 rowBufA = malloc(A.cols*sizeof(float)); 
 vecBufh = malloc(h.rows*sizeof(float));
 //read in hub score vector
 MPI_File_open(world, "hubVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
 MPI_File_read(fh, vecBufh, h.rows*h.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
 MPI_File_close(&fh);

 //Each process will read in a row or col and perform mat mult
 //After each row, the current row is trashed and a new row is put in its place

 //hub score multiplication
 //Read in current row of adj Matrix perform inner produt, then dump and move to next row
 
 //malloc multBuf for calculation
 multBuf = malloc(rowcts[myRank]*sizeof(float)); 

 //For loop for A*h calculation for every row of A assigned to process 
 for(int i=0; i<rowcts[myRank]; i++)
 {
  //Read in current row from A on respective process 
  MPI_File_open(world, "adjacencyMat.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
  MPI_File_read_at
  (
   fh, 
   (rowDispls[myRank]+i)*A.cols*sizeof(float), //Offset
   rowBufA, //Buffer
   A.cols, //Count
   MPI_FLOAT, MPI_STATUS_IGNORE //Status
  );
 
  multBuf[i] = (float)ipMatrix(rowBufA, vecBufh, h.rows);
  //MPI_File_close(&fh);
 }
 //Next multiply th multBuf results by A^T for every row of A assigned to process
 //To do this we will read in rows of adjacency matrix for inner product

 //For loop to complete A^T * (A*h)
 //Result of A*h store in multBuf 
 for(int i=0; i<rowcts[myRank]; i++)
 {
  //Read new row into buffer using MPI_File_read_at
  MPI_File_read_at
  (
   fh,
   //Offset calculation slightly dif from one above because coltype datatype will be used
   //Offset calculation will start each process at the "top" of their respective column
   (rowDispls[myRank]+i)*sizeof(float),	//Offset
   rowBufA, //Buffer
   1, //Count
   coltype, //Datatype
   MPI_STATUS_IGNORE //Status
  );    
 
  vecBufh[i] = (float)ipMatrix(rowBufA, multBuf, h.rows);
  MPI_File_close(&fh);
 }
 free(multBuf);  
 
 //Now that each process has their multiplication operations complete
 //We can perform power method operations
 //After power method is complete, new eigenvectors will be store in respective files
 max=vecBufh[0];
 for(int i=1; i<rowcts[myRank]; i++)
 {
  if(vecBufh[i] > max) max = vecBufh[i];
 }
 MPI_Allreduce(&max, &scalar, 1, MPI_DOUBLE, MPI_MAX, world);

 for(int i=0; i<rowcts[myRank]; i++)
 {
  vecBufh[i] *= (1/scalar);
 }
 //Store new eigenvector in respective data file
 MPI_File_open(world, "hubVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
 MPI_File_write_at
 (
  fh, //Handle
  rowDispls[myRank]*sizeof(float), //Offset
  vecBufh, //Buffer
  rowcts[myRank], //Count
  MPI_FLOAT, MPI_STATUS_IGNORE //Datatype, Status
 ); 

 MPI_File_close(&fh);
 
 //Free vec and row buf 
 free(vecBufh);
}


//-----------------------End of hub vector calculation-------------------------------
//-----------------------Begin authority calulcation---------------------------------


for(int j=0; j<20; j++)
{

//malloc buffer to hold authority score vector
vecBufa = malloc(a.rows*sizeof(float));
//read in hub score vector
MPI_File_open(world, "authorityVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
MPI_File_read(fh, vecBufa, a.rows*a.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
MPI_File_close(&fh);


//Now perform similar operation on the authority score vector

//multiply L^T by current authority vector
//To do this we will read in columns of adjacency matrix for inner product

//Re-malloc multBuf for use in authority vector calc
multBuf = malloc(rowcts[myRank]*sizeof(float)); //Buffer to hold matrix multiplication results

//For loop to complete A^T * a for every row of A^T assigned to a process
for(int i=0; i<rowcts[myRank]; i++)
{
 MPI_File_open(world, "adjacencyMat.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
 MPI_File_read_at
 (
  fh, //Handle
  (rowDispls[myRank]+i)*sizeof(float),	//Offset
  //Offset calculation slightly dif from one above because coltype datatype will be used
  //Offset calculation will start each process at the "top" of their respective column
  rowBufA, //Buffer
  1, //Count
  coltype, //Datatype
  MPI_STATUS_IGNORE //Status
 );    
  
 multBuf[i] = (float)ipMatrix(rowBufA, vecBufa, a.rows);
 //MPI_File_close(&fh);
}

//For loop to complete A * (A^T*a) on every row of A assigned respective processes
//Result of A^T * a store in multBuf
for(int i=0; i<rowcts[myRank]; i++)
{
 //Read in current row on respective process 
 //MPI_File_open(world, "hubVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
 MPI_File_read_at
 (
  fh, //handle 
  (rowDispls[myRank]+i)*A.cols*sizeof(float), //Offset
  rowBufA, //Buffer
  A.cols, //Count
  MPI_FLOAT, MPI_STATUS_IGNORE //Datatype, Status
 );
 

 vecBufa[i] = (float)ipMatrix(rowBufA, multBuf, h.rows);
 MPI_File_close(&fh);
}
//Free multBuf after it is no longer needed
free(multBuf);
//Normalize the vector before writing to file
max=vecBufa[0];
for(int i=1; i<rowcts[myRank]; i++)
{
 if(vecBufa[i] > max) max = vecBufa[i];
}
MPI_Allreduce(&max, &scalar, 1, MPI_DOUBLE, MPI_MAX, world);
 for(int i=0; i<rowcts[myRank]; i++)
 {
  vecBufa[i] *= (1/scalar);
 }

//Write new eigenvector in repsective datafile after normalization
MPI_File_open(world, "authorityVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
MPI_File_write_at
(
 fh, //handle
 rowDispls[myRank]*sizeof(float), //Offset
 vecBufa, //Buffer
 rowcts[myRank], //Count
 MPI_FLOAT, MPI_STATUS_IGNORE //Datatype, status
);

MPI_File_close(&fh);
free(vecBufa);

}
//-------------------------------End authority vec calculation----------------------------
//-------------------------------Begin Page rank calculation------------------------------

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

//For loop for each process to take a col and make it stochastic
for(int i=0; i<rowcts[myRank]; i++)
{
 MPI_File_open(world, "adjacencyMat.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
 MPI_File_read_at
 (
  fh, //Handle
  (rowDispls[myRank]+i)*sizeof(float),	//Offset
  //Offset calculation slightly dif from one above because coltype datatype will be used
  //Offset calculation will start each process at the "top" of their respective column
  rowBufA, //Buffer
  1, //Count
  coltype, //Datatype
  MPI_STATUS_IGNORE //Status
 );    
  
 MPI_File_close(&fh);

 //For loop to sum entries in column
 int sum=0;
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

 MPI_File_write_at
 (
  fh, //handle
  (rowDispls[myRank]+i)*sizeof(float), //Offset
  vecBufa, //Buffer
  1, //Count
  coltype, MPI_STATUS_IGNORE //Datatype, status
 );
}



//For loop for power method instead of while loop testing convergence
for(int j=0; j<20; j++)
{
 //Use MPI_File i/o to read correct row in from data file
  
 //malloc buffer to hold authority score vector
 vecBufp = malloc(p.rows*sizeof(float));
 multBuf = malloc(p.rows*sizeof(float));
//read in hub score vector
 MPI_File_open(world, "pgRankVec.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
 MPI_File_read(fh, vecBufp, p.rows*p.cols, MPI_FLOAT, MPI_STATUS_IGNORE);
 MPI_File_close(&fh);

 
 //For loop to perform M*p for every row in M
 for(int i=0; i<rowcts[myRank]; i++)
 {
 
 MPI_File_open(world, "stochasticMat.data", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
  //Read in current row on respective process 
  MPI_File_read_at
  (
   fh, //handle 
   (rowDispls[myRank]+i)*A.cols*sizeof(float), //Offset
   rowBufA, //Buffer
   A.cols, //Count
   MPI_FLOAT, MPI_STATUS_IGNORE //Datatype, Status
  );
 MPI_File_close(&fh);
 
 multBuf[i] = (float)ipMatrix(rowBufA, multBuf, p.rows);

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

/*
 //Write stochastic matrix to datafile

 MPI_File_open(world, "stochasticMat.data", MPI_MODE_CREATE | MPI_MODE_RD_WR, MPI_INFO_NULL, &fh);

 MPI_File_write_at
 (
  fh, //handle
  (rowdispls[myRank]+i)*sizeof(float), //Offset
  multBuf, //Buffer
  1, //Count
  coltype, MPI_STATUS_IGNORE //Datatype, status
 );
*/
 }

 //free buffers
 free(multBuf);
 free(vecBufp);
}
//--------------------------End pgRank calculation------------------------



//stopTime = MPI_Wtime();


MPI_Finalize();
return 0;
}











