#include "our_shell_zfi.h"
#include "our_shell_zfi.c"




int main(int argc,char **argv){
/*
	//Programme principal
	//Variables pour la récupération des commandes 
	char entree[MAXENTREE] ;
	char *listeArgs[MAXCMDs];
	char *listeArgsPipe[MAXCMDs]
	
	//Initialisation du shell
	Initialiser_shell();
	
	for(ever){
		//Afficher le répértoire courant 
		my_pwd()
		//récupérer l'entrée à partir de la ligne de commande 
		int recup = recupEntry(entree) ;
		
		//Analyse de la commande 
		int cas = décortiquerEntree(entree,listeArgs,listeArgsPipe)
		if (cas == 2){
			executerCmdSimple(listeArgs);
		}
		if( cas == 3){
			executerCmdComplexe(listeArgsPipe);
		}
		
	}

*/
	
/*	
	//test de la fonction trouverPipe	
	char *commandesSiPipe[10]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	

	char entree[100];
	strcpy(entree,"ls -l |grep aaa |echo bbb");
	int ret= trouverPipe(entree,commandesSiPipe);
	printf("%d\n",ret);
	for(int i=0 ;i<3;i++){
		if(commandesSiPipe[i] != "")
			printf("%s\n",commandesSiPipe[i]);
	}
*/
/*	
	//test de la fonction recupArgs
	char entree[100];
	strcpy(entree,"ls -l -a");
	char *listeArgs[4]={NULL,NULL,NULL,NULL};
	recupArgs(entree,listeArgs);
	for(int i=0 ;i<4;i++){
		if(listeArgs[i] != NULL)
			printf("%s \n",listeArgs[i]);
	}
*/

/*
	//test de la fonction commande valide
	char entree[100];
	strcpy(entree,"ls -la");//commande existante
	char *listeArgs[4]={NULL,NULL,NULL,NULL};
	recupArgs(entree,listeArgs);
	int ret=commandeValide(listeArgs);
	strcpy(entree,"echo hello");//commande non existante
	recupArgs(entree,listeArgs);
	ret=commandeValide(listeArgs);
*/
/*
	//test de la fonction decortiquer 
	char *listeArgsPipe[4]={NULL,NULL,NULL,NULL};
	char *listeArgs[4]={NULL,NULL,NULL,NULL};
	char entree[100];
	strcpy(entree,"ls -l|grep aaa");
	int cas =decortiquerEntree(entree,listeArgs,listeArgsPipe);
	printf("le cas est le %d \n",cas);
	if (cas != 0){
		for(int i=0 ;i<4;i++){
			if(listeArgs[i] != NULL)
				printf("%s \n",listeArgs[i]);
		}
		for(int i=0 ;i<4;i++){
			if(listeArgsPipe[i] != NULL)
				printf("%s \n",listeArgsPipe[i]);
		}
	}
*/
	return 0;
}
