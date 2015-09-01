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

int cmd_help(tok_t arg[]);
int cmd_cd(tok_t arg[]);
process * create_process(tok_t* arg);
void add_process(process* p);
void desc_process(process* p);

void terminal_cmd(tok_t arg[]){
	
	int status;
	process *p=create_process(arg);
	add_process(p);
	latest_process=p;
	
	
	pid_t id=fork();
	
	if(id==0){
		
		latest_process->pid=getpid();
		launch_process(latest_process);
		
	}else if(id>0){
		latest_process->pid=id;
		//desc_process(latest_process);
		
		if(latest_process->background){
			waitpid(id,&(latest_process->status),WNOHANG | WUNTRACED);
		}else{
			waitpid(id,&(latest_process->status),WUNTRACED);
			tcsetattr(shell_terminal,TCSADRAIN,&shell_tmodes);
			tcsetpgrp(shell_terminal,shell_pgid);
			
		}
		
		if(WEXITSTATUS(latest_process->status)==EXIT_FAILURE){
			if(latest_process->prev){
				latest_process=p->prev;
				free(p);	
			}else{
				first_process=latest_process=NULL;
				free(p);
			}
			
		}
		
	}else if(id<0){
		fprintf(stderr,"failed to fork\n");
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

void init_shell(){
  /* Check if we are running interactively */
  shell_terminal = STDIN_FILENO;

  /** Note that we cannot take control of the terminal if the shell
      is not interactive */
  shell_is_interactive = isatty(shell_terminal);

  if(shell_is_interactive){
	
    /* force into foreground */
    while(tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp())){
    	
      	kill( - shell_pgid, SIGTTIN);
	}
	
	signal (SIGINT, SIG_IGN);//interrupt : C-c
    signal (SIGQUIT, SIG_IGN);/*C-\, same as SIGINT, makes core dump, (like user detects error) */
    signal (SIGTSTP, SIG_IGN);//C-z
    signal (SIGTTIN, SIG_IGN);//sent if tries to read from terminal when in background
    signal (SIGTTOU, SIG_IGN);//same as previous but writing to terminal
    
	
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
void add_process(process* p){
	if(!first_process){
			
		first_process=p;
		first_process->prev=NULL;
		
	}else{
		
		latest_process->next=p;
		p->prev=latest_process;
			
	}
}

/**
 * Creates a process given the inputString from stdin
 */
process* create_process(tok_t * arg){
	process * P=malloc(sizeof(process));
	P->argv=arg;
	P->tmodes=shell_tmodes;
	
	P->stdin=STDIN_FILENO;
	P->stdout=STDOUT_FILENO;
	P->stderr=STDERR_FILENO;
	P->completed=0;
	P->stopped=0;
	P->background=0;
	
	redirectOut(P);
	redirectIn(P);
	
	int tokPos;
	if((tokPos=isDirectTok(arg,"&"))){
		
		if(strlen(arg[tokPos])==1){
			P->background=1;
			int moveUp=tokPos;
			arg[moveUp]=NULL;
						
			while(arg[moveUp+1]){
				arg[moveUp]=arg[moveUp+1];
				
				arg[moveUp+1]=NULL;
				moveUp++;
			}
		}
	}
	
	int arg_count=0;
	while(P->argv[arg_count]){
		arg_count++;
	}
	
	P->argc=arg_count;
	
	return P;
}

void desc_process(process *p){
	fprintf(stdout,"pid: %d,proc ptr: %p,prev: %p\n",p->pid,p,p->prev);
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
    //printf("args s: %s\n",s);
    fundex = lookup(t[0]); /* Is first token a shell literal */
    if(fundex >= 0) cmd_table[fundex].fun(&t[1]);
    else {
    	if(t[0]){
    		terminal_cmd(t);	
    	}
      	//fprintf(stdout, "This shell only supports built-ins. Replace this to run programs as commands.\n");
    }
    free(s);
    freeToks(t);
    fprintf(stdout, "%d:%s $ ", lineNum,cdir);
  }
  return 0;
}
