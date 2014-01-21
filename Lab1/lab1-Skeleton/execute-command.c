// UCLA CS 111 Lab 1 command execution

#include "alloc.h"
#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <errno.h>
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
	//	error (1, 0, "command execution not yet implemented");
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
	printf("%d",c->u.command[0]->status);

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
	// IMPLEMENT
}

void
execute_sequence_operator (command_t c)
{
	// IMPLEMENT
}

void
execute_simple_command (command_t c)
{
//	pid_t pid = fork();
	
//	if(!pid)
//	{
		execvp(c->u.word[0],c->u.word);
		exit(c->status);
//	}
//	else
//	{
//		error(1,0,"You forked up");
//	}
}

void
execute_subshell_command (command_t c)
{
	// IMPLEMENT
}
