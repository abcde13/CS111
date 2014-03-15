#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>

//Open access file
FILE* openfile(const char * filename);

//Close access file
void closefile(FILE * ofile);

void check();
int match_regex (regex_t * r,const char * to_match);
int compile_regex (regex_t * r, const char * regex_text);
