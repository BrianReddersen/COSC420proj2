#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

struct matrix{
	int rows;
	int cols;
	short *data;
} matrix;

//builds an adjacency matrix for tht first 1000 entries in the citation file
int main(){
	int i = 0;
	int mode = 0;
	char *line = malloc(255*sizeof(char));
	size_t bufsiz;
	FILE *stream = fopen("arxiv-citations.txt", "r");
	FILE *out = fopen("indexes", "w");
	int n = 0;
	for (i = 0; i < 1656538; i){		//replace this for loop with a while(!eof) to get the indexes of every paper instead of the first 1k
// 	while((n = getline(&line, &bufsiz, stream)) != -1){
// 		printf("%d\n", i);
		getline(&line, &bufsiz, stream);
		printf("%d\n", i);
		if ((strcmp(line, "-----\n") && strcmp(line, "+++++\n")) && !mode){
			mode = 1;
			fprintf(out, "%d:%s", i, line);
		}
		if (!strcmp(line, "+++++\n")){
			i++;
			mode = 0;
		}
	}
	fclose(stream);
	fclose(out);
	free(line);
}
