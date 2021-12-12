#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>


int main()
{
 int i = 0;
 int n = 0;
 int mode = 0;
 char* line = malloc(255*sizeof(char));
 size_t bufsiz;
 FILE *stream = fopen("arxiv-metadata.json", "r");
 FILE *out = fopen("indexes", "w");

 for(i=0; i<1628118; i)
 {
  if(getline(&line, &bufsiz, stream) == EOF) break;
  
  if(!mode)
  { 
   mode = 1;
   fprintf(out, "%d:%s", i, line);
   printf("%d:%s", i, line);
  }
  if(!strcmp(line, "++++++\n"))
  {
   i++;
   mode = 0;
   printf("%d\n", i);
  }

 }

 
 fclose(stream);
 fclose(out);
 free(line);

return 0;
} 
