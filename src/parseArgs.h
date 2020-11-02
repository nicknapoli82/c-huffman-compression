#ifndef ARGS_DEF
#define ARGS_DEF

enum ARG_IS {arg_NONE, FLAG, OPTION, STRING};

typedef struct {
    enum ARG_IS type;
    union {
	char flag;
	char *argument;
    } arg;
} arg;

/* Just init for the args parser. Run before using the subsequent function */
void args_setup(unsigned int argc, char *argv[]);

/* 
Consumes one argument in the argv vector, and returns what it is as an arg
Strips all '-' or '--' from the argument. 
 */
arg args_getOne();

/*
Similar to getOne. Returns a vector of args. Requires the use of freeAll to
clean up after itself.
 */
arg *args_getAll();

/* reset simply sets internal argc tracker back to 0 */
void args_reset();

/* sets the argc tract to num */
void args_setArgc(unsigned int num);

/* clean up memory for internal args_all */
void args_freeAll();

#endif
