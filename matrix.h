/*
 * Lab3.2 matrix.h 
 * iteration of lab 3.1 matrix.h
 * Functions added or improved
 * 1: Matrix Multiplication improved function argument takes 
 * transpose of B for better spatial locality when taking 
 * inner product of rows and columns
 *
 * 2: gaussJordan implemented as function?
 */



//Header file that stores algorithms for matrix operations
#define INDEX(i,j,n,m) i*m + j
//Function to calc send counts for scatterv
//Array to hold sendcts must be passed into function
void calcSendcts(int* sendcts, int worldSize, int row, int col)
{
 int minSend = row/worldSize;
 for(int i=0; i<worldSize; i++)
 {
  sendcts[i] = minSend;
 }
 for(int i=0; i<row%worldSize; i++)
 {
  sendcts[i]++;
 }
 for(int i=0; i<worldSize; i++)
 {
  sendcts[i] *= col;
 }
 return;
}
//Function to calc row count for use with MPI_Datatype
void calcRowcts(int* rowcts, int worldSize, int row)
{
 int minSend = row/worldSize;
 for(int i=0; i<worldSize; i++)
 {
  rowcts[i] = minSend;
 }
 for(int i=0; i<row%worldSize; i++)
 {
  rowcts[i]++;
 }
 return;
}
//Function to calc row displs for use with row MPI_Datatype
void calcRowDispls(int* rowDispls, int* rowcts, int worldSize)
{
 rowDispls[0] = 0;
 for(int i=1; i<worldSize; i++)
 {
  rowDispls[i] = rowDispls[i-1] + rowcts[i-1];
 }
 return;
}

//Function to calc displacmenet for scatterv
//Must send in array for displs and sendcts
void calcDispls(int* displs, int* sendcts, int worldSize)
{
 displs[0] = 0;
 for(int i=1; i<worldSize; i++)
 {
  displs[i] = displs[i-1] + sendcts[i-1];
 }
 return;
}


//Function for inner product(IP)
float ipMatrix(float* matrixA, float* matrixB, int sendct)
{
 float ip = 0;	//Var to hold ip
 //For loop to calc ip
 for(int i=0; i<sendct; i++)
 {
  ip += ((*(matrixA+i)) * (*(matrixB+i)));
  //ip += matrixA[i] * matrixB[i];
 }
 return ip;
}


//Function for addition
void addMatrix(double* matrixA, double* matrixB, double* prodMatrix, int sendct)
{
 //Add the arrays and store in new array
 for(int i=0; i<sendct; i++)
 {
  prodMatrix[i] = matrixA[i] + matrixB[i];
 }
 return;
// return prodMatrix;	
}
//Function for subtraction
void subMatrix(double* matrixA, double* matrixB, double* prodMatrix, int sendct)
{
 //Add the arrays and store in new array
 for(int i=0; i<sendct; i++)
 {
  prodMatrix[i] = matrixA[i] - matrixB[i];
 }
 return;
}

//Function for multiplication take in matrix A and transposed matrixB
void multMatrix(float* matrixA, float* matrixB, float* prodMatrix, int rowA, int colA, int colB, int rank)
{
 for(int i=0; i<rowA; i++)
 {
  for(int j=0; j<colB; j++)
  {
   prodMatrix[(i*colA)+j] = ipMatrix(matrixA+(i*colA), matrixB+(j*colB), colA);
   //printf("Inner product of %d %d on rank %d is %15.5f\n",i, j, rank, prodMatrix[(i*j)+j]);
  }
 }
  
 return;
}

//Function for vector matrix multiplication






void gjElim(int* matrix, int* vector, int row, int col)
{

return;
}

//Function to populate matrix memset to 0
void popM(double* matrix, int row, int col)
{
 for(int i=0; i<row; i++)
 {
  for(int j=0; j<col; j++)
  { 
   matrix[INDEX(i,j,row,col)] = (double)rand() / 1000;
  } 
 }
 return;
}


//Function to print matrix to a file
void fprintM(char* name, char rank, double* matrix, int row, int col)
{
 FILE *handle;
 char fname[256];
 
 sprintf(fname, "outfile_matrix%s%c.txt", name, rank);

 handle = fopen(fname, "w");
 //Double for loop prints the matrix out to 4 decimal places
 for(int i=0; i<row; i++)
 {
  for(int j=0; j<col; j++)
  {
   fprintf(handle, "%20.5f ", matrix[INDEX(i,j,row,col)]);
  }
  fprintf(handle,"\n");
 }
 fclose(handle);
 return;	
}


void fprintgjM(char* name, int rank, int step, double* matrix, int row, int col)
{
 FILE *handle;
 char fname[256];
 
 sprintf(fname, "outfile_matrix%s%d-%d.txt", name, rank, step);

 handle = fopen(fname, "w");
 //Double for loop prints the matrix out to 4 decimal places
 for(int i=0; i<row; i++)
 {
  for(int j=0; j<col; j++)
  {
   fprintf(handle, "%20.5f ", matrix[INDEX(i,j,row,col)]);
  }
  fprintf(handle,"\n");
 }
 fclose(handle);
 return;	
}















