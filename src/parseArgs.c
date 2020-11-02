#include <stdlib.h>

#include "parseArgs.h"

unsigned int args_at = 1;
unsigned int args_until;
char **args_vec;
arg *all_args;
unsigned int next_is_flag;
unsigned int flag_at;

void args_setup(unsigned int argc, char *argv[]) {
    args_until = argc;
    args_vec = argv;
    flag_at = 1;
}

arg args_getOne() {

    if(args_at == args_until) {
	return (arg){arg_NONE, .arg={.flag = 0}};
    }

    arg result = {arg_NONE, .arg={.flag=arg_NONE}};

    if(args_vec[args_at][0] == '-') {
	if(args_vec[args_at][1] == '-') {
	    result.type = OPTION;
	    result.arg.argument = &args_vec[args_at][2];
	    args_at++;
	}
	else {
	    result.type = FLAG;
	    result.arg.flag = args_vec[args_at][flag_at];
	    if (args_vec[args_at][flag_at + 1]) {
		next_is_flag = 1;
		flag_at++;
	    }
	    else {
		next_is_flag = 0;
		flag_at = 1;
		args_at++;
	    }
	}
    }
    else {
	    result.type = STRING;
	    result.arg.argument = args_vec[args_at];
	    args_at++;
    }

    return result;
}

arg *args_getAll() {
    all_args = malloc(sizeof(arg) * args_until);
    for (unsigned int a = 0; a < args_until; a++) {
	all_args[a] = args_getOne();
    }
    return all_args;
}

void args_reset() {
    args_at = 0;
}

void args_setArgc(unsigned int num) {
    args_at = num;
}

void args_freeAll() {
    if(all_args != NULL) {
	free(all_args);
    }
    all_args = NULL;
}
