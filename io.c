#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "io.h"
#include "parse.h"
#include "process.h"
/* Read a line from input file and return as a string object */

#define MAXLINE 1024

char *freadln(FILE *ifile) {
  char line[MAXLINE];		/* allocate line buffer on the stack */
  char *r = NULL;		/* ptr to return string object */
  int len;
  char *s = fgets((char *)line, MAXLINE, ifile);
  if (!s) return s;
  len = strlen(s)+1;
  r = malloc(len);
  strncpy(r,s,len);
  return r;
}

void freeln(char *ln) {
  free(ln);
}

void redirectOut(process* p){
	tok_t* arg=p->argv;
	
	int tokenPlace=isDirectTok(arg,">");
		//printf("tokPlace: %d\n",tokenPlace);
		if(tokenPlace){
			int pos=containsChar(arg[tokenPlace],'>');
			//printf("> pos: %d\n",pos);
			if(pos==0){
				int length=strlen(arg[tokenPlace]);
				if(length==1){
					int newfd=open(arg[tokenPlace+1], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
					if(newfd>-1){
						//dup2(newfd,STDOUT_FILENO);
						p->stdout=newfd;
						int moveUp=tokenPlace;
						arg[moveUp]=NULL;
						arg[moveUp+1]=NULL;
						
						while(arg[moveUp+2]){
							arg[moveUp]=arg[moveUp+2];
							//free(arg[moveUp+2]);
							arg[moveUp+2]=NULL;
							moveUp++;
						}
					}
				}else if(length > 1){
					int newfd=open(arg[tokenPlace]+1, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
					if(newfd>-1){
						//dup2(newfd,STDOUT_FILENO);
						p->stdout=newfd;
						int moveUp=tokenPlace;
						arg[moveUp]=NULL;
						
						while(arg[moveUp+1]){
							arg[moveUp]=arg[moveUp+1];
							//free(arg[moveUp+2]);
							arg[moveUp+1]=NULL;
							moveUp++;
						}
					}
				}
			}
		}
}

void redirectIn(process* p){
	
	tok_t* arg=p->argv;
	int tokenPlace=isDirectTok(arg,"<");
		//printf("tokPlace: %d\n",tokenPlace);
		if(tokenPlace){
			int pos=containsChar(arg[tokenPlace],'<');
			//printf("> pos: %d\n",pos);
			if(pos==0){
				int length=strlen(arg[tokenPlace]);
				//printf("length: %d\n",length);
				if(length==1){
					int newfd=open(arg[tokenPlace+1], O_RDONLY , S_IRUSR | S_IWUSR);
					if(newfd>-1){
						//dup2(newfd,STDIN_FILENO);
						p->stdin=newfd;
						int moveUp=tokenPlace;
						arg[moveUp]=NULL;
						arg[moveUp+1]=NULL;
						
						while(arg[moveUp+2]){
							arg[moveUp]=arg[moveUp+2];
							//free(arg[moveUp+2]);
							arg[moveUp+2]=NULL;
							moveUp++;
						}
					}else{
						fprintf(stderr,"file does not exist or has wrong permissions \n");
					}
				}else if(length > 1){
					int newfd=open(arg[tokenPlace]+1, O_RDONLY , S_IRUSR | S_IWUSR);
					if(newfd>-1){
						//dup2(newfd,STDIN_FILENO);
						p->stdin=newfd;
						int moveUp=tokenPlace;
						arg[moveUp]=NULL;
						
						while(arg[moveUp+1]){
							arg[moveUp]=arg[moveUp+1];
							//free(arg[moveUp+2]);
							arg[moveUp+1]=NULL;
							moveUp++;
						}
					}
				}
			}
		}
}






