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

struct arg_struct
{
	command_t cmd;
	int thread_index;
};

void do_command (command_t c);
void do_thread (void* c);
void print_dependency_matrix ();
int runnable (int thread_id);
void execute_and_operator (command_t c);
void execute_or_operator (command_t c);
void execute_pipe_operator (command_t c);
void execute_sequence_operator (command_t c);
void execute_simple_command (command_t c);
void execute_subshell_command (command_t c);
void rmv_dependencies (int finished_thread_id);
void find_threads (command_t cmd);
void extract_dependencies (command_t cmd, int thread_id);
void locate_dependencies (char * file, int thread_id);
int analyze_subtree (char * file, command_t cmd);

int num_threads = 0;

int dependency_table[10000][10000];

command_t pthread_list[10000];
int index_counter = 0;

int
command_status (command_t c)
{
  return c->status;
}

void
print_dependency_matrix ()
{
	int i,j;
	for (i = 0; i < num_threads; i++)
	{
		for(j = 0; j < num_threads; j++)
		{
			printf("%i ",dependency_table[i][j]);
		}
		printf("\n");
	}
}

int 
num_sequence_commands(command_t c)
{
	if(c->type != SEQUENCE_COMMAND) {
		return 0;
	}
	else
	{
		return (1+num_sequence_commands(c->u.command[0])+num_sequence_commands(c->u.command[1]));
	}
}
	

void
execute_command (command_t c, int time_travel)
{
  /*	FIXME: Replace this with your implementation.  You may need to
     	add auxiliary functions and otherwise modify the source code.
    	You can also use external functions defined in the GNU C Library.  */
	

	num_threads = num_sequence_commands(c)+1;
	if(time_travel){
		int i,j,k;
		index_counter = 0;
		for (i = 0; i < num_threads; i++)
		{
			for(j = 0; j < num_threads; j++)
			{
				dependency_table[i][j] = 0;
			}
		}
		find_threads(c);
		for (k = 0; k < index_counter; k++)
		{
			extract_dependencies(pthread_list[k], k);
		}
		//print_dependency_matrix();	
		int * ind_commands = malloc(index_counter*sizeof(int));
		int num_threads_left = index_counter;
		while (num_threads_left > 0) {
			int runnable_threads = 0;
			
//			print_dependency_matrix();				
			for (k = 0; k < index_counter; k++)
			{
				if(runnable(k))
				{
					ind_commands[runnable_threads] = k;
					dependency_table[k][k] = -1;
					runnable_threads++;
					num_threads_left--;
				}
			}
//			print_dependency_matrix();				
			pthread_t * threads = malloc(runnable_threads*sizeof(pthread_t));
			struct arg_struct* args = malloc(runnable_threads*sizeof(struct arg_struct));

			for(k = 0; k < runnable_threads; k++)
			{
				args[k].thread_index = k;
				args[k].cmd = pthread_list[ind_commands[k]]; 
			}
//			print_dependency_matrix();
			for(k = 0; k < runnable_threads; k++)
			{
				pthread_create(&threads[k],NULL, (void*) &do_thread, (void*)&args[k]);
			}
			//print_dependency_matrix();			
			for (k = 0; k < runnable_threads; k++)
			{
				pthread_join(threads[k],NULL);
			}
//			print_dependency_matrix();
			for (k = 0; k < runnable_threads; k++)
			{
				int column = ind_commands[k];

				for(j = 0; j < index_counter; j++)
				{
					if(dependency_table[j][column] == 1)
					{
						dependency_table[j][column] = 0;
					}
				}
				//print_dependency_matrix(); 
			}
		//	print_dependency_matrix();
		}
	} 
	else	
	{
		do_command(c);
	}
//	num_threads = 0;
//	index_counter = 0;
}

void
find_threads (command_t cmd)
{
	if(cmd->type != SEQUENCE_COMMAND)
	{
		pthread_list[index_counter] = cmd;
		index_counter++;
	}
	
	else
	{
		find_threads(cmd->u.command[0]);
		find_threads(cmd->u.command[1]);
	}
}

int
analyze_subtree (char * file, command_t cmd)
{
	if(cmd->type == SIMPLE_COMMAND)
	{
		//fprintf(stderr,"%s\n",cmd->input);
		if(cmd->input)
		{
			if(strcmp(cmd->input,file) == 0)
			{
				//dep_threads++;
				return 1;	
			}
		}
		return 0;
	}

	else
	{
		switch(cmd->type)
		{
			case SUBSHELL_COMMAND:
				return analyze_subtree(file,cmd->u.subshell_command);
			default:
				return analyze_subtree(file,cmd->u.command[0]) + analyze_subtree(file,cmd->u.command[1]);
		}
	}
	return 0;
}

void
locate_dependencies (char * file, int thread_id)
{
	int k = thread_id+1;
	int dependency_found;
	for(; k < index_counter; k++)
	{
		dependency_found = analyze_subtree(file, pthread_list[k]);
		if(dependency_found) {
			dependency_table[k][thread_id] = 1;	
		}
	}
}

void
extract_dependencies (command_t cmd, int thread_id)
{
	if(cmd->type == SIMPLE_COMMAND)
	{
		if(cmd->output)
		{
			locate_dependencies(cmd->output, thread_id);	
		}
		return;	
	}
	else
	{
		switch(cmd->type)
		{
			case SUBSHELL_COMMAND:
				extract_dependencies(cmd->u.subshell_command,thread_id);
				break;
			default:
				extract_dependencies(cmd->u.command[0],thread_id);
				extract_dependencies(cmd->u.command[1],thread_id);
		}
	}
}

void
rmv_dependencies (int finished_thread_id)
{
/*	int i = 0;
	for(; i < index_counter; i++)
	{
		if(dependency_table[i][finished_thread_id] == 1)
			dep_threads--;
		dependency_table[i][finished_thread_id] = 0;
//		print_dependency_matrix();
	}
	dependency_table[finished_thread_id][finished_thread_id] = -1;*/
}

int
runnable (int thread_id)
{
	int i = 0;
	for(; i < index_counter; i++)
	{
		if(dependency_table[thread_id][i] == 1 || dependency_table[thread_id][i] == -1)
			return 0;
	}
	return 1;	
}

void
do_thread (void* c)
{
	struct arg_struct * args = (struct arg_struct *)c;
	execute_command(args->cmd,0);
//	free(c);
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
