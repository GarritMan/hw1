#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#define INPUT_STRING_SIZE 80
#define MAX_DIR_STRING_SIZE 255
#define PATH_SPLITTER ":"
#include "io.h"
#include "parse.h"
#include "process.h"
#include "shell.h"

char *cdir=NULL;

void terminal_cmd(tok_t arg[]){
	int ee=0;
	pid_t id=fork();
	
	if(id==0){
		
		redirectOut(arg); // can find this function in io.c
		redirectIn(arg);
		//Really struggling with redirecting input, but i think i may have a fairly decent solution.
		
		
		if(containsChar(arg[0],'/')>-1){
			execv(arg[0],arg);
		}else{
			path_name *pth=splitPaths(getenv("PATH"));
	
			int i;
			for(i=0;pth[i];i++){
				char* temp=malloc(strlen(pth[i])+strlen(arg[0])+2);
				temp=strcpy(temp,pth[i]);
				//printf("temp: %s\n",temp);
				execv(strcat(strcat(temp,"/"),arg[0]),arg);
				
				free(temp);
			}
			
		}
		
		//system("cowsay sorry china I cant find a program to run");
		fprintf(stdout,"failed to run the file, check that it exists!\n");
		exit(1);
	}else{
		wait(&ee);
		//printf("ee: %d\n",ee);
	}
}

void set_dir(){
	
	free(cdir);
	char *temp=malloc(MAX_DIR_STRING_SIZE);
	if((temp=getcwd(temp,MAX_DIR_STRING_SIZE))==NULL ){
  		int i=2;
  		while(errno==ERANGE && temp==NULL){
  			
  			temp=realloc(temp,MAX_DIR_STRING_SIZE*i);
  			temp=getcwd(temp,MAX_DIR_STRING_SIZE*i);
  			i++;
  			//printf("err: %d , i: %d\n, %s\n",errno,i,temp);
  		}
  	}
  	int len=strlen(temp)+1;
  	cdir=malloc(len);
  	strncpy(cdir,temp,len);
  	free(temp);
} 

int cmd_quit(tok_t arg[]) {
  printf("Bye\n");
  exit(0);
  return 1;
}

int cmd_help(tok_t arg[]);

int cmd_cd(tok_t arg[]);

/* Command Lookup table */
typedef int cmd_fun_t (tok_t args[]); /* cmd functions take token array and return int */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_quit, "quit", "quit the command shell"},
  {cmd_cd,"cd","change current working directory"}
};

int cmd_help(tok_t arg[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    printf("%s - %s\n",cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
}

int cmd_cd(tok_t arg[]){
	char temp[MAX_DIR_STRING_SIZE];
	if(arg[0]){
		strcpy(temp,arg[0]);
		char *spacer="\ ";
		int i;
		for(i=1;arg[i];i++){
			strcat(temp,spacer);
			strcat(temp,arg[i]);
		
		}
	}
	//printf("TEST: %s\n",temp);
	
	if(chdir(temp)==0){
		set_dir();
		return 1;
	}else{
		fprintf(stdout,"no such file or directory :\\\n");
		return 0;
	}
	
	
}

int lookup(char cmd[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)) return i;
  }
  return -1;
}

void init_shell()
{
  /* Check if we are running interactively */
  shell_terminal = STDIN_FILENO;

  /** Note that we cannot take control of the terminal if the shell
      is not interactive */
  shell_is_interactive = isatty(shell_terminal);

  if(shell_is_interactive){

    /* force into foreground */
    while(tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp()))
      kill( - shell_pgid, SIGTTIN);

    shell_pgid = getpid();
    /* Put shell in its own process group */
    if(setpgid(shell_pgid, shell_pgid) < 0){
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &shell_tmodes);
  }
  /** YOUR CODE HERE */
}

/**
 * Add a process to our process list
 */
void add_process(process* p)
{
  /** YOUR CODE HERE */
}

/**
 * Creates a process given the inputString from stdin
 */
process* create_process(char* inputString)
{
  /** YOUR CODE HERE */
  return NULL;
}



int shell (int argc, char *argv[]) {
  char *s=NULL; //= malloc(INPUT_STRING_SIZE+1);			/* user input string */
  
  tok_t *t;			/* tokens parsed from input */
  int lineNum = 0;
  int fundex = -1;
  pid_t pid = getpid();		/* get current processes PID */
  pid_t ppid = getppid();	/* get parents PID */
  pid_t cpid, tcpid, cpgid;

  init_shell();

  printf("%s running as PID %d under %d\n",argv[0],pid,ppid);
  
  lineNum=0;
  
  set_dir();
  
  fprintf(stdout, "%d:%s $ ", lineNum,cdir);
  while ((s = freadln(stdin))){
  	
    t = getToks(s); /* break the line into tokens */
    fundex = lookup(t[0]); /* Is first token a shell literal */
    if(fundex >= 0) cmd_table[fundex].fun(&t[1]);
    else {
    	if(t[0]){
    		terminal_cmd(t);	
    	}
    	
    	//execl(t[0],t[0],NULL);
    	//system(t[0]);
      	//fprintf(stdout, "This shell only supports built-ins. Replace this to run programs as commands.\n");
    }
    free(s);
    freeToks(t);
    fprintf(stdout, "%d:%s $ ", lineNum,cdir);
  }
  return 0;
}
