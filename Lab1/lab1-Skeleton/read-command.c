// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <error.h>
#include <ctype.h>

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
    int wordFlag = 0;
    int spaceFlag = 0;
    const int AND_OPERATOR = 0;
    const int OR_OPERATOR = 1;
    const int PIPE_OPERATOR = 2;
    const int REDIRECT_FROM = 3;
    const int REDIRECT_TO = 4;
    const int OPEN_PAREN = 5;
    const int CLOSE_PAREN = 6;
    const int NEWLINE  = 7;
    const int SPACE = 8;
    
    while(c!=EOF){
	if(c == ' ' && spaceFlag){
		printf("Skipping \n");
        	c = get_next_byte(get_next_byte_argument);
		continue;
	} else if (c == ' '){
        	c = get_next_byte(get_next_byte_argument);
		spaceFlag = 1;
		continue;
	}
	if(spaceFlag && wordFlag){
		wordFlag = 0;
	}
	spaceFlag = 0;
        switch (c) {
            case '<' :
                printf("< \n");
		wordFlag = 0;
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
		wordFlag = 0;
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
		wordFlag = 0;
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
		wordFlag = 0;
                if(andFlag){
			printf("Error \n");
			andFlag = 0;
			orFlag=1;
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
		wordFlag = 0;
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
		wordFlag = 0;
                if(andFlag){
			printf("Error \n");
			andFlag = 0;
		} else if(orFlag) {
			printf("Pipe \n");
			orFlag = 0;
		}
                break;
	    case '\n' : 
	    case ';':
		wordFlag = 0;
		break;
            default :
		if(!isalnum(c) && c!='!'&& c!='%' && c!='+'&& c!=',' && c!='-' && c!='.' &&  c!='/'&&  c!=':'&& c!='@'&& c!='^'&& c!='_'){
			printf("Error \n");
		}else{
			if(andFlag){
				printf("Error \n");
				andFlag =0;
			} else if(orFlag){
				printf("Pipe \n");
				orFlag = 0;
			} else {
				printf("%c \n", c);
				wordFlag = 1;
			}			
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


