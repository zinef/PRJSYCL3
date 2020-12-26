#include "our_shell_zfi.h"
#include "our_shell_zfi.c"



int main(int argc,char **argv){
	

	
		/*	signal(SIGINT , stop);
		std_out_copy   = dup(STDOUT_FILENO);
    	std_error_copy = dup(STDERR_FILENO);
    	std_in_copy    = dup(STDIN_FILENO ); */

	//Programme principal
	//Variables pour la récupération des commandes 
	char entree[MAXENTREE] ;
	char *listeArgs[MAXCMDs];
	char *listeArgsPipe[MAXCMDs];
	//char pwd_g[1024]="";
	//Initialisation du shell
	Initialiser_shell();
	strcpy(pwd_global,getcwd(NULL,0));
	for(ever){
		//Afficher le répértoire courant 
		
		my_pwd_global();
		printf("%s",pwd_global);
		//récupérer l'entrée à partir de la ligne de commande 
		int recup = recupEntry(entree) ;
		
		//Analyse de la commande 
		int cas = decortiquerEntree(entree,listeArgs,listeArgsPipe);
		if (cas == 1){
			executerCmdSimple(listeArgs,entree);
			
		}
		if( cas == 2){
			//executerCmdComplexe(listeArgsPipe);
			
		}
		
	}


	return 0;
}
