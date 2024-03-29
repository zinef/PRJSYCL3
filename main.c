#include "our_shell_zfi.h"
#include "our_shell_zfi.c"



int main(int argc,char **argv){
	
	//Programme principal
	//signal et variables pour gestion des redirections
	signal(SIGINT , stop);
	std_out_copy   = dup(STDOUT_FILENO);
    	std_error_copy = dup(STDERR_FILENO);
    	std_in_copy    = dup(STDIN_FILENO ); 
	char *out_file1 = malloc(sizeof(char*)),*out_file2= malloc(sizeof(char*)),*out_file3= malloc(sizeof(char*));
    	int fd1,fd2,fd3;
   	char *listArgsRed[100];
	//initialisation de listArgsRed
    	for(int j = 0 ; j<100;j++){
        	listArgsRed[j] = malloc(sizeof(char*));
        	listArgsRed[j] = NULL;
    	}

	//Variables pour la récupération des commandes 
	char entree[MAXENTREE];
	char *listeArgs[MAXCMDs];
	char *commandesSiPipe[MAXCMDs];
	for(int j = 0 ; j<MAXCMDs;j++){
        	listeArgs[j] = malloc(sizeof(char*));
        	listeArgs[j] = NULL;
   	}

	//Initialisation du shell
	Initialiser_shell();
	strcpy(pwd_global,getcwd(NULL,0));

	//loop
	for(ever){
		running=1;
		strcpy(entree,"");
		for(int j = 0 ; j<MAXCMDs;j++){
        	listeArgs[j] = NULL;
   		}
   		//Afficher le répértoire courant
		my_pwd_global();
		write(1,ANSI_COLOR_CYAN,strlen(ANSI_COLOR_CYAN));
		write(1,pwd_global,strlen(pwd_global));
		write(1,ANSI_COLOR_RESET,strlen(ANSI_COLOR_RESET));
		//récupérer l'entrée à partir de la ligne de commande 
		int recup = recupEntry(entree) ;
		if(strcmp(entree,"")!=0){
			if((strstr(entree,">") != NULL) || (strstr(entree,"<") != NULL)){//detection des redirections
        			int nb_red = parssing_red(entree,listArgsRed);
    				int i=0;
				if(start_all_redirect(&fd1, &fd2, &fd3, out_file1, out_file2 , out_file3, listArgsRed, &nb_red)){
    				
						char *cmd=strtok(entree,"<>");
						recupArgs(cmd,listeArgs);
						executerCmdSimple(listeArgs);
				}
				end_all_redirect(&fd1, &fd2, &fd3, out_file1 , out_file2 , out_file3, listArgsRed, nb_red);
					
    		}else{
					//Analyse de la commande 
					int cas = decortiquerEntree(entree,listeArgs,commandesSiPipe);
					if (cas == 1){
						executerCmdSimple(listeArgs);
					}
					if( cas == 2){
						executerCmdComplexe(commandesSiPipe);
			
					}
					if(cas == 0){
						write(1,"zfi :command not found \n",strlen("zfi :command not found \n"));	
					}
			}
		}
	}

	return 0;
}
