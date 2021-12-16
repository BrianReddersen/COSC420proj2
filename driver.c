#include "arxiv.h"
#include "matrix.h"
int main(){
	char inBuff[255];
	while(1){
		printf("Input a String\n");
		fgets(inBuff,255,stdin);
		// exits program on user input of ctr+D so loop can end
		if(feof(stdin)){
			printf("\nEOF detected Ending program\n");
			break;
		}else{
			//gets rid of new line character at the end of the sting that fgets keeps
			inBuff[strcspn(inBuff,"\n")] = 0;
			//calls the search any function which searches the database of papers for any of the given string(s) passed
			//can handle inputs like "Matrix Multiplication" it will search the database for Abstracts with matrix or multiplication
			search_any(inBuff);
			//resets inBuff back to default state
			memset(inBuff,0,255);
			printf("Enter another String\n");
			fgets(inBuff,255,stdin);
			//gets rid of new line character at the end of the sting that fgets keeps
			inBuff[strcspn(inBuff,"\n")] = 0;
			//calls the search all function which searches the database of papers for all of the given string(s) passes
			//can handle inputs like "Matrix Multiplication" it will search the database for Abstracts with matrix or multiplication
			search_all(inBuff);
		}
	}
	return 0;
}
