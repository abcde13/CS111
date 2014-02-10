// UCLA CS 111 Lab 1 command execution
#define _GNU_SOURCE

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
void do_thread (void* c);
void print_dependency_matrix ();
int runnable (pthread_t thread);
void execute_and_operator (command_t c);
void execute_or_operator (command_t c);
void execute_pipe_operator (command_t c);
void execute_sequence_operator (command_t c);
void execute_simple_command (command_t c);
void execute_subshell_command (command_t c);
void rmv_dependencies (pthread_t thread);
void add_dependencies (command_t cmd, pthread_t thread);

int num_threads = 0;
pthread_mutex_t num_threads_lock = PTHREAD_MUTEX_INITIALIZER;

int dependency_table[10000][10000];

int
command_status (command_t c)
{
  return c->status;
}

void
print_dependency_matrix ()
{
	int i,j;
	printf("%i",num_threads);
	for (i = 0; i < num_threads; i++)
	{
		for(j = 0; j < num_threads; j++)
		{
			printf("%i ",dependency_table[i][j]);
		}
		printf("\n");
	}
}
	

void
execute_command (command_t c, int time_travel)
{
  /*	FIXME: Replace this with your implementation.  You may need to
     	add auxiliary functions and otherwise modify the source code.
    	You can also use external functions defined in the GNU C Library.  */
	

	if(time_travel){
		int i,j;
		for (i = 0; i < 1000; i++)
		{
			for(j = 0; j < 1000; j++)
			{
				dependency_table[i][j] = 0;
			}
		}
		pthread_t tid;
		int status = pthread_create(&tid, NULL, (void *) &do_thread, c);
		if(!status)
		{
			pthread_mutex_lock(&num_threads_lock);
			num_threads++;
			add_dependencies(c,tid);
			print_dependency_matrix();	
			pthread_mutex_unlock(&num_threads_lock);
		}
	} 
	else	
	{
		do_command(c);
	}
}

int
runnable (pthread_t thread)
{
	return 1;	
}

void
rmv_dependencies (pthread_t thread)
{
}

void
add_dependencies (command_t cmd, pthread_t thread)
{
}

void
do_thread ( void* c)
{
	c = (command_t) c;
	while(!runnable(pthread_self()))
	{
		pthread_yield();
	}
	do_command(c);
	pthread_mutex_lock(&num_threads_lock);
	num_threads--;
	pthread_mutex_unlock(&num_threads_lock);
	rmv_dependencies(pthread_self());
//	free(c);
	pthread_exit(0);	
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
		case PIPE_COMMAND:
			execute_pipe_operator(c);
			break;
		case SIMPLE_COMMAND:
			execute_simple_command(c);
			break;
		case SUBSHELL_COMMAND:
			execute_subshell_command(c);
			break;
		case SEQUENCE_COMMAND:
			execute_sequence_operator(c);
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
	pipe(fd);

	pid_t l_pid;
	l_pid = fork();

	if(l_pid == -1)
	{
		error(1,0,"You forked up the pipeline man");
	}
	else if(l_pid == 0)
	{
		close(1);
		dup2(fd[1],1);
		close(fd[0]);
		close(fd[1]);

		do_command(c->u.command[0]);
		exit(c->u.command[0]->status);
	}
	
	pid_t r_pid;
	r_pid = fork();

	if(r_pid == -1)
	{
		error(1,0,"You forked up the pipeline man");
	}
	else if(r_pid == 0)
	{
		close(0);
		dup2(fd[0],0);
		close(fd[0]);
		close(fd[1]);

		do_command(c->u.command[1]);
		exit(c->u.command[1]->status);	
	}
	
	close(fd[0]);
	close(fd[1]);
	int status_left_child,status_right_child;
	
	waitpid(l_pid,&status_left_child,0);
	waitpid(r_pid,&status_right_child,0);
	
	c->status = status_right_child;	
}

void
execute_sequence_operator (command_t c)
{
	// IMPLEMENT
	do_command(c->u.command[0]);
	do_command(c->u.command[1]);
	c->status = c->u.command[1]->status;
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
		if(execvp(c->u.word[0],c->u.word) < 0)
		{
			error(1,0,"Error in execution of simple command \n");
			return;
		}
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
	do_command(c->u.subshell_command);
	c->status = c->u.subshell_command->status;	// IMPLEMENT
}
