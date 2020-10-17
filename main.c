#include "our_shell_zfi.h"
#include "our_shell_zfi.c"
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <readline/readline.h> 
#include <readline/history.h> 
#include <errno.h>

int main(int argc,char **argv){
	
//	Initialiser_shell();
	char *string;	
//	int res=recupEntry(string);
//	printf("returned value is :%d and the command was : %s \n",res,string);
	my_pwd();	
	return 0;
}
