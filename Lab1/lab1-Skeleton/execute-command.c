// UCLA CS 111 Lab 1 command execution

#include "alloc.h"
#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

void do_command (command_t c);
void execute_and_operator (command_t c);
void execute_or_operator (command_t c);
void execute_pipe_operator (command_t c);
void execute_sequence_operator (command_t c);
void execute_simple_command (command_t c);
void execute_subshell_command (command_t c);

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, int time_travel)
{
  /*	FIXME: Replace this with your implementation.  You may need to
     	add auxiliary functions and otherwise modify the source code.
    	You can also use external functions defined in the GNU C Library.  */
	do_command(c);
}

void
do_command (command_t c)
{
	switch(c->type)
	{
		case AND_COMMAND:
			execute_and_operator(c);
			break;
		case OR_COMMAND:
			execute_or_operator(c);
			break;
		case SIMPLE_COMMAND:
			execute_simple_command(c);
			break;
		default:
			error(1,0,"Command not found. Try again fucker.");
			break;
	} 
}

void 
execute_and_operator (command_t c)
{
	do_command(c->u.command[0]);

	if(c->u.command[0]->status == 0)
	{
		do_command(c->u.command[1]);
		c->status = c->u.command[1]->status;
	}
	else
	{
		c->status = c->u.command[0]->status;
	}	
}

void 
execute_or_operator (command_t c)
{
	do_command(c->u.command[0]);

	if(!(c->u.command[0]->status))
	{
		c->status = c->u.command[0]->status;
	}
	else
	{
		do_command(c->u.command[1]);
		c->status = c->u.command[1]->status;
	}	
}

void
execute_pipe_operator (command_t c)
{
	int fd[2];
	int numbytes;
	pid_t childpid;
	pipe(fd);
	childpid = fork();

	if(childpid == -1)
	{
		error(1,0,"You forked up the pipeline man");
	}
	else if(childpid == 0)
	{
	//	dup2(fd[0],0);
		close(fd[1]);
		do_command(c->u.command[1]);
		close(fd[0]);
		exit(c->u.command[1]->status);
	}
	else
	{
		pid_t parentpid;
		parentpid = fork();

		if(parentpid == -1)
		{
			error(1,0,"You forked up the pipeline man");
		}
		else if(parentpid == 0)
		{
			close(fd[0]);
			do_command(c->u.command[0]);
			close(fd[1]);
			exit(c->u.command[0]->status);	
		}
		else
		{
			close(fd[0]);
			close(fd[1]);
			int completion_status;	
		}
	}	
}

void
execute_sequence_operator (command_t c)
{
	// IMPLEMENT
}

void
execute_simple_command (command_t c)
{
	pid_t pid = fork();
	
	if(pid > 0)
	{
		int status = 0;
		pid_t test = waitpid(pid,&status,0);
		if(test == -1)
			error(1,0,"Child process failed");
		c->status = WEXITSTATUS(status);
	}
	else if(pid == 0)
	{
		int fd_in,fd_out;
		if(c->input)
		{
			fd_in = open(c->input, O_RDONLY, 0666);
			if(fd_in == -1)
				error(1,0,"Input file could not be opened");
			if(dup2(fd_in,0) == -1)
				error(1,0,"Input redirect failed");
			close(fd_in);
		}
		if(c->output)
		{
			fd_out = open(c->output, O_WRONLY|O_CREAT|O_TRUNC,0666);
			if(fd_out == -1)
				error(1,0,"Output file could not be opened");
			if(dup2(fd_out,1) == -1)
				error(1,0,"Output redirect failed");
			close(fd_out);
		}
		execvp(c->u.word[0],c->u.word);
		exit(c->status);
	}
	else
	{
		error(1,0,"You forked up");
	}
}

void
execute_subshell_command (command_t c)
{
	// IMPLEMENT
}
