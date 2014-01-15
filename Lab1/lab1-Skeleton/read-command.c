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
const int AND_OPERATOR = 0; const int OR_OPERATOR = 1;
const int PIPE_OPERATOR = 2;
const int REDIRECT_FROM = 3;
const int REDIRECT_TO = 4; const int OPEN_PAREN = 5;
const int CLOSE_PAREN = 6;
const int NEWLINE  = 7;
const int SEMICOLON  = 8;
const int SPACE = 9;
int place = 0;
int oplace = 0;
int addedLast = 0;
int parenFlag = 0;
int anyflag = 0;

struct command_stream
{
    command_t *forest_pointer;
    int size;
} cs;

void add_to_stack(int constant, char ** word, int length);
int compareOperator(int first, int second);
void pop(int and);
void push(command_t* cmd, int and);
command_t * operators;
command_t * operands;
void createTree(command_t * operator, command_t * operandRight, command_t  *operandLeft);
int isand = 0;

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
    int wordcount = 0;
    int size = 100; 
    int buffcount = 0;
    cs.forest_pointer = malloc(size);
    char * buff = malloc(sizeof(char) * size);
    char ** words = malloc(sizeof(char *) * size);
    operators =  malloc(sizeof(command_t*) * size);
    operands =  malloc(sizeof(command_t*) * size);
    
    while(1){
	if(c==EOF){
		break;
	}
	if(c == '#' /*&& buff[0] == '\0'*/ && !anyflag){
		while(c!= '\n'){
			printf("In comment");
			c = get_next_byte(get_next_byte_argument);
		}
	} else if(c == '#' /*&&  buff[0] == '\0'*/ && anyflag){
		printf("THERE WAS A # ERROR");
	}
	
	if (c == ' ' && wordFlag && !spaceFlag){
		words[wordcount] = buff;
		wordcount++;
		buff = malloc(sizeof(char) * size);
		anyflag = 0;
		buffcount = 0;
		spaceFlag = 1;
		c = get_next_byte(get_next_byte_argument);
		continue;
	} else if(c == ' ' && spaceFlag){
        	c = get_next_byte(get_next_byte_argument);
		continue;
	} else if( c== ' '){
		anyflag = 0;
		printf("RETARDED \n");
        	c = get_next_byte(get_next_byte_argument);
		continue;
	}
		
	spaceFlag = 0;
        switch (c) {
            case '<' :
		anyflag = 1;
		if(wordFlag){
			anyflag = 0;
			wordFlag = 0;
			if(buff[0] != '\0'){
				words[wordcount] = buff;
				wordcount++;
			}
			add_to_stack(-1,words,wordcount);
			addedLast= 1;
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
		if(andFlag){
			printf("Error 1 \n");
			andFlag = 0;
		} else if(orFlag){
			orFlag = 0;
			add_to_stack(PIPE_OPERATOR,NULL,-1);
		}
		add_to_stack(REDIRECT_FROM,NULL,-1);
			
                break;
            case '>' :
		anyflag = 1;
		if(wordFlag){
			anyflag = 0;
			wordFlag = 0;
			if(buff[0] != '\0'){
				words[wordcount] = buff;
				wordcount++;
			}
			add_to_stack(-1,words,wordcount);
			addedLast= 1;
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
                if(andFlag){
			printf("Error 2 \n");
			andFlag = 0;
		} else if(orFlag){
			add_to_stack(PIPE_OPERATOR,NULL,-1);
			orFlag = 0;
		}
		add_to_stack(REDIRECT_TO,NULL,-1);
                break;
            case '&' :
		anyflag = 1;
		//printf("%c \n", c);
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			anyflag = 0;
			addedLast= 1;
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
		anyflag = 1;
                //printf("%c \n", c);
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			addedLast= 1;
			anyflag = 0;
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
                if(andFlag){
			printf("Error 3 \n");
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
		anyflag = 1;
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			wordcount++;
			anyflag = 0;
			add_to_stack(-1,words,wordcount);
			addedLast= 1;
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
                if(andFlag){
			printf("Error 4 \n");
			andFlag = 0;
		} else if(orFlag){
			add_to_stack(PIPE_OPERATOR,NULL,-1);
			orFlag = 0;
		}
		add_to_stack(OPEN_PAREN,NULL,-1);
                break;
            case ')' :
		anyflag = 1;
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			anyflag = 0;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			addedLast= 1;
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
                if(andFlag){
			printf("Error 5 \n");
			andFlag = 0;
		} else if(orFlag) {
			add_to_stack(PIPE_OPERATOR,NULL,-1);
			orFlag = 0;
		}
		add_to_stack(CLOSE_PAREN,NULL,-1);
                break;
	    case '\n' : 
	    case ';':
		anyflag = 1;
		if(wordFlag){
			wordFlag = 0;
			words[wordcount] = buff;
			anyflag = 0;
			wordcount++;
			add_to_stack(-1,words,wordcount);
			addedLast= 1;
			buff = malloc(sizeof(char) * size);
			words = malloc(sizeof(char *) * size);
			wordcount = 0;
			buffcount = 0;
		}
		if(orFlag){
			add_to_stack(PIPE_OPERATOR,NULL,-1);	
			orFlag = 0;
		} else if (andFlag){
			printf("Error 6 \n");
			andFlag = 0;
		}  
		if(c == ';'){
			add_to_stack(SEMICOLON,NULL,-1);
		} else {
			add_to_stack(NEWLINE,NULL,-1);
		}
		break;
            default :
		if(!isalnum(c) && c!='!'&& c!='%' && c!='+'&& c!=',' && c!='-' && c!='.' &&  c!='/'&&  c!=':'&& c!='@'&& c!='^'&& c!='_'){
			printf("Error 7 \n");
		}else{
			if(andFlag){
				printf("Error 8 \n");
				andFlag =0;
			} else if(orFlag){
				add_to_stack(PIPE_OPERATOR,NULL,-1);
				orFlag = 0;
				//printf("%c \n", c);
				if(buffcount == size){
					size = size + 100;
					buff = realloc(buff,sizeof(size));
				}
			        buff[buffcount] = c;
				buffcount++;
				wordFlag = 1;
			} else {
				//printf("%c \n", c);
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
		printf("Error 9");
	} else if (wordFlag){
		wordFlag = 0;
		words[wordcount] = buff;
		wordcount++;
		add_to_stack(-1,words,wordcount);
		addedLast= 1;
		buff = malloc(sizeof(char) * size);
		words = malloc(sizeof(char *) * size);
		wordcount = 0;
		buffcount = 0;
	}

	printf("NEARING THE END \n");
	if(place!=0){
		pop(0);
	}
	while(place != 0){
		if(oplace > 1){
			printf("AT END \n");
			printf("%d \n",(operators[place-1])->type);
			printf("%d \n",(operands[oplace-1])->type);
			printf("%d \n",(operands[oplace-2])->type);
			createTree(&(operators[place-1]), &(operands[oplace-1]),&(operands[oplace-2]));
		} else {
			printf("Not enough operands 1\n");
			printf("%d \n",place);
			place--;
		}
	}
	if(oplace==1){
		cs.forest_pointer[cs.size] = operands[oplace-1];
		cs.size++;
		pop(1);
	}
	
	int i = 0;
	while(i < cs.size){
		print_command(cs.forest_pointer[i]);
		i++;
	}
	//print_command(oper
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
	cmd->output = 0;
	cmd->input = 0;
	if(word != NULL){
		cmd->type = SIMPLE_COMMAND;
		cmd->u.word = malloc(sizeof(char*)*length);
		cmd->u.word = word;
		command_t * k = &cmd; push(k,1); //printf("\n");
	}
	else {
		if(constant == AND_OPERATOR){
			cmd->type = AND_COMMAND;
			command_t * k = &cmd;
			
			push(k,0);
			//printf("I have an AND \n");		
		} else if(constant == OR_OPERATOR){
			cmd->type = OR_COMMAND;
			command_t * k = &cmd;
			push(k,0);
			//printf("I have an OR \n");
		}
		 else if(constant == PIPE_OPERATOR){
			cmd->type = PIPE_COMMAND;
			command_t * k = &cmd;
			push(k,0);
			//printf("I have an PIPE \n");
		}  else if(constant == REDIRECT_FROM){
			cmd->type = REDIRECT_FROM_COMMAND;
			command_t * k = &cmd;
			push(k,0);
			//printf("I have an < \n");
		}  else if(constant == REDIRECT_TO){
			cmd->type = REDIRECT_TO_COMMAND;
			command_t * k = &cmd;
			push(k,0);
			//printf("I have an > \n");
		}  else if(constant == NEWLINE){
			cmd->type = NEWLINE_COMMAND;
			command_t * k = &cmd;
			//printf("I have an newline/; \n");
			push(k,0);
		}  else if(constant == SEMICOLON){
			cmd->type = SEQUENCE_COMMAND;
			command_t * k = &cmd;
			push(k,0);
			//printf("I have an semicolon/; \n");
		}  else if(constant == SPACE){
			//printf("I have an space \n");
		}  else if(constant == CLOSE_PAREN){
			//printf("I have an ) \n");
			cmd->type = END_SUBSHELL_COMMAND;
			command_t * k = &cmd;
			push(k,0);
		}  else if(constant == OPEN_PAREN){
			//printf("I have an ( \n");
			cmd->type = START_SUBSHELL_COMMAND;
			command_t * k = &cmd;
			push(k,0);
			
		}
	}
}

void push(command_t * cmd, int and){
	printf("%p and type %d \n", *cmd, (*cmd)->type);
	if(!and){
		if((*cmd)->type == START_SUBSHELL_COMMAND){
			parenFlag++;
		}
		if(place != 0 && (*cmd)->type == END_SUBSHELL_COMMAND){
			if(!addedLast){
				printf("OPERATOR PRECEDES )");
				return;
			}
			while((*(operators[place-1])).type != START_SUBSHELL_COMMAND){
				if(oplace > 1){
					printf("DOING PAREN, TREE \n");
					createTree(&(operators[place-1]), &(operands[oplace-1]),&(operands[oplace-2]));
				} else {
					printf("Not enough operands for parentheses\n");
					place--;
				}
				if(place == 0){
					printf("NEVER FOUND (");
					return;
				}
			}
			pop(0);
			parenFlag--;
			command_t cmd = malloc(sizeof(100));
			cmd->output = malloc(sizeof(char*));
			cmd->input = malloc(sizeof(char*));
			cmd->output = 0;
			cmd->input = 0;
			cmd->type = SUBSHELL_COMMAND;
			cmd->u.subshell_command = (operands[oplace-1]);
			(operands[oplace-1]) = cmd;
			print_command((operands[oplace-1]));
			return;
		} else if(place == 0 && (*cmd)->type == END_SUBSHELL_COMMAND){
			printf("ERRoR, can't have ) first");
			return;
		}
			
		if(place != 0 && (*cmd)->type == SEQUENCE_COMMAND){
			while(place != 0){
				if(oplace > 1){
					printf("HIT SEMICOLON, TREE \n");
					createTree(&(operators[place-1]), &(operands[oplace-1]),&(operands[oplace-2]));
				} else {
					printf("Not enough operands 2\n");
					place--;
				}
			}
			cs.forest_pointer[cs.size] = operands[oplace-1];
			cs.size++;
			pop(1);
			return;
		}
		if(place==0 && oplace==0 && (*cmd)->type == NEWLINE_COMMAND){
			return;
		} else 	if(place!=0 && (*cmd)->type == NEWLINE_COMMAND){
			if((*(operators[place-1])).type == NEWLINE_COMMAND){
				return;
			} else if((*(operators[place-1])).type == REDIRECT_FROM_COMMAND || (*(operators[place-1])).type == REDIRECT_TO_COMMAND){
				printf("ERROR FROM REDIRECT AND NEWLINE \n");
				return;	
			}
		} else if(place!=0 && (*(operators[place-1])).type == NEWLINE_COMMAND){
			if((*cmd)->type == END_SUBSHELL_COMMAND){
				pop(0);
				printf("POPPED BECAUSE OF CLOSE PAREN \n");				
			} else if(!addedLast && (*cmd)->type ==START_SUBSHELL_COMMAND){
				pop(0);
				printf("POPPED BECAUSE OF OPEN PAREN \n");
			} else if(addedLast && (*cmd)->type ==START_SUBSHELL_COMMAND){
				pop(0);
				printf("TREATING NEWLINE AS SEMICOLON \n");
				while(place != 0){
					if(oplace > 1){
						printf("HIT SEMICOLON, TREE \n");
						createTree(&(operators[place-1]), &(operands[oplace-1]),&(operands[oplace-2]));
					}else {
						printf("Not enough operands 3\n");
						place--;
					}

				}
				cs.forest_pointer[cs.size] = operands[oplace-1];
				cs.size++;
				pop(1);
			}

			
		} else if (place!=0 && (*(*cmd)).type != START_SUBSHELL_COMMAND && ((*(operators[place-1])).type == REDIRECT_FROM_COMMAND || (*(operators[place-1])).type == REDIRECT_TO_COMMAND)){
			//printf("SYNTAX BAD. GOTTA DO REDIRECT SHIT");
		}
		while((place!=0 && !parenFlag && !compareOperator((*(operators[place-1])).type,(*(*cmd)).type)) 
			|| ((*(*cmd)).type == END_SUBSHELL_COMMAND &&  (*(operators[place-1])).type!=START_SUBSHELL_COMMAND)){
			//printf("HI \n");
			//printf("oplace: %d place %d \n", oplace, place);
//			printf("bottom of stack: %d \n ",(*(operators[0])).type);
			if(oplace > 1){
				printf("HIT SEMICOLON, TREE \n");
				createTree(&(operators[place-1]), &(operands[oplace-1]),&(operands[oplace-2]));
			} else {
				printf("%d %d parenFlag \n", parenFlag, place);
				printf("Not enough operands 4\n");
				place--;
			}

		}
		if((*(*cmd)).type == END_SUBSHELL_COMMAND){
			pop(0);
			return;
		}
				
		operators[place] = *cmd;
		place++;
		if((*cmd)->type != NEWLINE_COMMAND){
			addedLast = 0;
		}
		printf("ADDEDLAST IS: %d \n", addedLast);
//		printf("bottom of pushed stack: %d \n ",(*(operators[0])).type);
//		printf("TYPE: %d \n",(*cmd)->type);
		//printf("%d %d \n", cmd->type, place);
	} else {
		if(place!=0 && (*cmd)->type == NEWLINE_COMMAND){
			if((*(operators[place-1])).type == REDIRECT_FROM_COMMAND || (*(operators[place-1])).type == REDIRECT_TO_COMMAND){
				printf("ERROR FROM REDIRECT AND NEWLINE \n");
				return;	
			}
		} else if(place!=0 && (*(operators[place-1])).type == NEWLINE_COMMAND){
			if(!addedLast){
				pop(0);
				printf("POPPED BECAUSE OF WORD AND NEWLINE \n");
			}else if(addedLast){
				pop(0);
				printf("TREATING NEWLINE AS SEMICOLON \n");
				while(place != 0){
					if(oplace > 1){
						printf("HIT SEMICOLON, TREE \n");
						createTree(&(operators[place-1]), &(operands[oplace-1]),&(operands[oplace-2]));
					} else {
						printf("Not enough operands 5\n");
						place--;
					}
				}
				cs.forest_pointer[cs.size] = operands[oplace-1];
				cs.size++;
				pop(1);
			}

		}
		operands[oplace] = *cmd;
		oplace++;
		addedLast = 1;
		printf("ADDEDLAST IS: %d \n", addedLast);
		if(place!= 0 && ((*(operators[place-1])).type == REDIRECT_FROM_COMMAND || (*(operators[place-1])).type == REDIRECT_TO_COMMAND)){
			command_t command = malloc(sizeof(command_t));
			command->type = SIMPLE_COMMAND;
			command->input = malloc(sizeof(char*));
			command->output = malloc(sizeof(char*));
			if((*(operators[place-1])).type == REDIRECT_TO_COMMAND){
				command->output=*((*(operands[oplace-1])).u.word);
				command->input = 0;
			} else {
				command->input=*((*(operands[oplace-1])).u.word);
				command->output = 0;
			}
					
			
			command->u.word = (*(operands[oplace-2])).u.word;
			pop(1);
			pop(1);
			pop(0);
			command_t * k = &command;
			push(k,1);
			
		}
		//printf("OPERAND %d %d \n", cmd->type, oplace);
	}
		
		
}
void pop(int and){
	if(!and){
//		free(operators[place]);	
//		printf("popping: %d  place: %d \n",(*(operators[place-1])).type, place);
		operators[place-1] = 0;
		place--;
		//printf("%d Popped operator \n", place);
	} else {
//		free(operands[oplace]);	
		operands[oplace-1] = 0;
		oplace--;
	}

	
}

int compareOperator(int first, int second)
{
    int result = 0;
    if(second==PIPE_COMMAND || second==START_SUBSHELL_COMMAND || second == NEWLINE_COMMAND || second==REDIRECT_FROM_COMMAND|| second==REDIRECT_TO_COMMAND)
    {
        result = 1;
    }
    return result;
}
		

void createTree(command_t * operator, command_t * operandRight, command_t*  operandLeft){
	(*(operator))->u.command[0] = *operandLeft;	
	(*(operator))->u.command[1] = *operandRight;	
	(operands[oplace-2]) = *operator;
	print_command((operands[oplace-2]));
	pop(1);
	pop(0);	
}
