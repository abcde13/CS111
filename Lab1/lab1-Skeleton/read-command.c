// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct command_stream
{
    command_t **forest_pointer;
};

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //error (1, 0, "command reading not yet implemented");	
    int c = get_next_byte(get_next_byte_argument);
    int andFlag = 0;
    int orFlag = 0;
    
    while(c!=EOF){
        switch (c) {
            case '<' :
                printf("< \n");
		if(andFlag){
			printf("Error \n");
			andFlag = 0;
		} else if(orFlag){
			printf("Pipe \n");
			orFlag = 0;
		}
                break;
            case '>' :
		printf("> \n");
                if(andFlag){
			printf("Error \n");
			andFlag = 0;
		} else if(orFlag){
			printf("Pipe \n");
			orFlag = 0;
		}
                break;
            case '&' :
		printf("%c \n", c);
                if(andFlag){
			andFlag = 0;
			printf("And \n");
		} else {
			andFlag = 1;
		}
		if(orFlag){
			printf("Pipe \n");
			orFlag = 0;
		}
		break;
            case '|' :
                printf("%c \n", c);
                if(andFlag){
			printf("Error \n");
			andFlag = 0;
		} else {
			if(orFlag){
				orFlag = 0;
				printf("Or \n");
			} else {
				orFlag = 1;
			}
		}
                break;
            case '(' :
                printf("( \n");
                if(andFlag){
			printf("Error \n");
			andFlag = 0;
		} else if(orFlag){
			printf("Pipe \n");
			orFlag = 0;
		}
                break;
            case ')' :
                printf(") \n");
                if(andFlag){
			printf("Error \n");
			andFlag = 0;
		} else if(orFlag) {
			printf("Pipe \n");
			orFlag = 0;
		}
                break;
            default :
                if(andFlag){
			printf("Error \n");
		} else if(orFlag){
			printf("Pipe \n");
			orFlag = 0;
		}
                break;
        }
        c = get_next_byte(get_next_byte_argument);
    }
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
