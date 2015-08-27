#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "parse.h"

/*          Get tokens from a line of characters */
/* Return:  new array of pointers to tokens */
/* Effects: token separators in line are replaced with NULL */
/* Storage: Resulting token array points into original line */

#define TOKseparator " \n:"
#define PATH_SPLITTER ":"

path_name *splitPaths(const char *pathList){
	int i;

	char *c;
	char *p=malloc(strlen(pathList)+1);
	strcpy(p,pathList);
	
  	path_name *paths = malloc(20*sizeof(path_name));
  	for (i=0; i<20; i++) paths[i] = NULL;     
  

  	c = strtok(p,PATH_SPLITTER);	 
  	for (i=0; c && (i < 20); i++) {
  		
    	paths[i] = c;
    	c = strtok(NULL,PATH_SPLITTER);	
  	}
  	free(p);
  	return paths;
}

tok_t *getToks(char *line) {
  int i;
  char *c;
	//printf("line: %p\n",line);
  tok_t *toks = malloc(MAXTOKS*sizeof(tok_t));
  for (i=0; i<MAXTOKS; i++) toks[i] = NULL;     /* empty token array */
  
	//printf("tok ptr: %p\n",toks);
  c = strtok(line,TOKseparator);	 /* Start tokenizer on line */
  for (i=0; c && (i < MAXTOKS); i++) {
    //printf("char place : %p\n",c);
    toks[i] = c;
    c = strtok(NULL,TOKseparator);	/* scan for next token */
  }
  return toks;
}

void freeToks(tok_t *toks) {
  free(toks);
}

void fprintTok(FILE *ofile, tok_t *t) {
  int i;
  for (i=0; i<MAXTOKS && t[i]; i++) {
    fprintf(ofile,"%s ", t[i]);
  }
  fprintf(ofile,"\n");
}

int containsChar(tok_t t,char a){
	int ans=-1;
	int i;
	for(i=0;t[i];i++){
		if(t[i]==a){
			ans=i;
			break;
		}
	}
	return ans;
}





/* Locate special processing character */
int isDirectTok(tok_t *t, char *R) {
  int i;
  for (i=0; i<MAXTOKS-1 && t[i]; i++) {
    if (strncmp(t[i],R,1) == 0) return i;
  }
  return 0;
}
