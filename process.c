#include "process.h"
#include "shell.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include "parse.h"
#include "io.h"
/**
 * Executes the process p.
 * If the shell is in interactive mode and the process is a foreground process,
 * then p should take control of the terminal.
 */
void launch_process(process *p)
{
	if(shell_is_interactive){
		setpgid(p->pid,p->pid);
		if(!p->background){
			tcsetpgrp(shell_terminal,p->pid);
		}
	
	
		signal (SIGINT, SIG_DFL);
		signal (SIGQUIT, SIG_DFL);
		signal (SIGTSTP, SIG_DFL);
		signal (SIGTTIN, SIG_DFL);
		signal (SIGTTOU, SIG_DFL);
	}
	
	dup2(p->stdout,STDOUT_FILENO);
	dup2(p->stdin,STDIN_FILENO);
		
	if(containsChar(p->argv[0],'/')>-1){
		execv(p->argv[0],p->argv);
	}else{
		path_name *pth=splitPaths(getenv("PATH"));
	
		int i;
		for(i=0;pth[i];i++){
			char* temp=malloc(strlen(pth[i])+strlen(p->argv[0])+2);
			temp=strcpy(temp,pth[i]);
			//printf("temp: %s\n",temp);
			execv(strcat(strcat(temp,"/"),p->argv[0]),p->argv);
				
			free(temp);
		}
			
	}
	
	//free(p);
	//system("cowsay sorry china I cant find a program to run");
	fprintf(stderr,"failed to run the file, check that it exists!\n");
	exit(EXIT_FAILURE);
}

/* Put a process in the foreground. This function assumes that the shell
 * is in interactive mode. If the cont argument is true, send the process
 * group a SIGCONT signal to wake it up.
 */
void
put_process_in_foreground (process *p, int cont)
{
  /** YOUR CODE HERE */
}

/* Put a process in the background. If the cont argument is true, send
 * the process group a SIGCONT signal to wake it up. */
void
put_process_in_background (process *p, int cont)
{
  /** YOUR CODE HERE */
}
