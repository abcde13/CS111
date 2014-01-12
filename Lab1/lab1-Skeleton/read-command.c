// UCLA CS 111 Lab 1 command reading

#include "command.h" 
#include "command-internals.h"
#include <stdio.h>
#include <error.h>
 #include <ctype.h>
#include <stdlib.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

const int AND_OPERATOR = 0;
const int OR_OPERATOR = 1;
const int PIPE_OPERATOR = 2;
const int REDIRECT_FROM = 3; const int REDIRECT_TO = 4;
const int OPEN_PAREN = 5;
const int CLOSE_PAREN = 6;
const int NEWLINE  = 7;
const int SPACE = 8;
int place = 0;

struct command_stream
{
    command_t **forest_pointer;
};

void add_to_stack(int constant, char ** word, int length);
int compareOperator(int first, int second);
void pop();
void push(command_t cmd);
command_t ** operators;

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //error (1, 0, "command reading not yet implemented");	 
    int c = get_next_byte(get_next_byte_argument); int andFlag = 0;
    int orFlag = 0;
    int wordFlag = 0;
    int spaceFlag = 0;
    int wordcount = 0;
    int size = 100; 
    int buffcount = 0;
    char * buff = malloc(sizeof(char) * size);
    char ** words = malloc(sizeof(char *) * size);
    operators =  malloc(sizeof(command_t*) * size);
    
    while(1){
	if(c==EOF){
		break;
	}
	if(c == ' ' && spaceFlag){
		printf("Skipping \n"); c = get_next_byte(get_next_byte_argument);
		continue;
	} else if (c == ' ' && wordFlag){
			words[wordcount] = buff;
			wordcount++;
			buff = malloc(sizeof(char) * size);
			buffcount = 0;
        		c = get_next_byte(get_next_byte_argument);
			continue;
	} else if(c == ' ' && !wordFlag){
        	c = get_next_byte(get_next_byte_argument);
		continue;
	}
	spaceFlag = 0;
        switch (c) {
            case '<' :
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
		if(andFlag){
			printf("Error \n");
			andFlag = 0;
		} else if(orFlag){
			orFlag = 0;
			add_to_stack(PIPE_OPERATOR,NULL,-1);
		}
		add_to_stack(REDIRECT_FROM,NULL,-1);
                break;
            case '>' :
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
                if(andFlag){
			printf("Error \n");
			andFlag = 0;
		} else if(orFlag){
			add_to_stack(PIPE_OPERATOR,NULL,-1);
			orFlag = 0;
		}
		add_to_stack(REDIRECT_TO,NULL,-1);
                break;
            case '&' :
		printf("%c \n", c);
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
                if(andFlag){
			andFlag = 0;
			add_to_stack(AND_OPERATOR,NULL,-1);
		} else {
			andFlag = 1;
		} 
		if(orFlag){ 
			orFlag = 0;
			add_to_stack(PIPE_OPERATOR,NULL,-1);
		}
		break;
            case '|' :
                printf("%c \n", c);
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
                if(andFlag){
			printf("Error \n");
			andFlag = 0;
			orFlag=1;
		} else {
			if(orFlag){
				orFlag = 0;
				add_to_stack(OR_OPERATOR,NULL,-1);
			} else {
				orFlag = 1;
			}
		}
                break;
            case '(' :
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
                if(andFlag){
			printf("Error \n");
			andFlag = 0;
		} else if(orFlag){
			add_to_stack(PIPE_OPERATOR,NULL,-1);
			orFlag = 0;
		}
		add_to_stack(OPEN_PAREN,NULL,-1);
                break;
            case ')' :
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
                if(andFlag){
			printf("Error \n");
			andFlag = 0;
		} else if(orFlag) {
			add_to_stack(PIPE_OPERATOR,NULL,-1);
			orFlag = 0;
		}
		add_to_stack(CLOSE_PAREN,NULL,-1);
                break;
	    case '\n' : 
	    case ';':
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
		if(orFlag){
			add_to_stack(PIPE_OPERATOR,NULL,-1);	
			orFlag = 0;
		} else if (andFlag){
			printf("Error");
			andFlag = 0;
		}  
		add_to_stack(NEWLINE,NULL,-1);
		break;
            default :
		if(!isalnum(c) && c!='!'&& c!='%' && c!='+'&& c!=',' && c!='-' && c!='.' &&  c!='/'&&  c!=':'&& c!='@'&& c!='^'&& c!='_'){
			printf("Error \n");
		}else{
			if(andFlag){
				printf("Error \n");
				andFlag =0;
			} else if(orFlag){
				add_to_stack(PIPE_OPERATOR,NULL,-1);
				orFlag = 0;
				printf("%c \n", c);
				if(buffcount == size){
					size = size + 100;
					buff = realloc(buff,sizeof(size));
				}
			        buff[buffcount] = c;
				buffcount++;
				wordFlag = 1;
			} else {
				printf("%c \n", c);
				if(buffcount == size){
					size = size + 100;
					buff = realloc(buff,sizeof(size));
				}
			        buff[buffcount] = c;
				buffcount++;
				wordFlag = 1;
			}			
		}
			
                break;
        }
        c = get_next_byte(get_next_byte_argument);
    }
	if(orFlag){
		add_to_stack(PIPE_OPERATOR,NULL,-1);	
	} else if (andFlag){
		printf("Error");
	} else if (wordFlag){
		wordFlag = 0;
		words[wordcount] = buff;
		wordcount++;
		add_to_stack(-1,words,wordcount);
		buff = malloc(sizeof(char) * size);
		words = malloc(sizeof(char *) * size);
		wordcount = 0;
		buffcount = 0;
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


void add_to_stack(int constant, char ** word, int length){
	
	command_t cmd = malloc(sizeof(100));
	cmd->output = malloc(sizeof(char*));
	cmd->input = malloc(sizeof(char*));
	if(word != NULL){
		cmd->type = SIMPLE_COMMAND;
		cmd->u.word = malloc(sizeof(char*)*length);
		cmd->u.word = word;
		cmd->output = 0;
		cmd->input = 0;
		int i = 0;
		int j = 0;
		while(j!= length){
			i = 0;
			while((cmd->u.word[j])[i] != '\0'){
		  		printf("%c ", (cmd->u.word[j])[i]);
		  		i++;
			}
			j++;
		}
		printf("\n");
	}
	else {
		if(constant == AND_OPERATOR){
			cmd->type = AND_COMMAND;
			cmd->output = 0;
			cmd->input = 0;
			
			push(cmd);
			printf("I have an AND \n");		
		} else if(constant == OR_OPERATOR){
			cmd->type = OR_COMMAND;
			cmd->output = 0;
			cmd->input = 0;
			push(cmd);
			printf("I have an OR \n");
		} else if(constant == PIPE_OPERATOR){
			cmd->type = PIPE_COMMAND;
			cmd->output = 0;
			cmd->input = 0;
			push(cmd);
			printf("I have an PIPE \n");
		}  else if(constant == REDIRECT_FROM){
			printf("I have an < \n");
		}  else if(constant == REDIRECT_TO){
			printf("I have an > \n");
		}  else if(constant == NEWLINE){
			cmd->type = SEQUENCE_COMMAND;
			printf("I have an newline/; \n");
		}  else if(constant == SPACE){
			printf("I have an space \n");
		}  else if(constant == CLOSE_PAREN){
			printf("I have an ) \n");
		}  else if(constant == OPEN_PAREN){
			printf("I have an ( \n");
		}
	}
}

void push(command_t cmd){
	while(place!=0 && !compareOperator((*(operators[place-1]))->type,cmd->type) ){
		pop();
	}
			
	operators[place] = &cmd;
	place++;
	printf("%d %d \n", cmd->type, place);
}
void pop(){
	free (operators[place]);
	place--;
	printf("%d Pop \n", place);
	
}

int compareOperator(int first, int second)
{
    int result = 0;
    if(second==PIPE_COMMAND)
    {
        result = 1;
    }
    return result;
}
		
