#include "our_shell_zfi.h"
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>

/*Implémentation des fonctionnalités*/


/***
	Initialiser_shell : procédure d'affichage après le lancement du shell

***/

void Initialiser_shell(){
	printf("\033[H\033[J");
	printf("\n***********************************\n");
	printf("\n***********OUR SHELL ZFI***********\n");
	char *id=getenv("USER");
	printf("\n \tBienvenu @%s \n",id);
	printf("\n***********************************\n");
	printf("\n***********************************\n");
	sleep(2);
	printf("\033[H\033[J");

}

/***
	recupEntry : la fonction qui récupère l'entrée et la met dans l'historique des commandes 
	@ entrées :  Chaine de caractères 
	@ sorties :  Un Booleen (soit 0 soit 1 ) (ici on le représente avec un entier)

	pour ajouter la commande à l'historique on va devoir utiliserla fonction add_history()
	de la bibliothèque readline.h 
		
***/

int recupEntry(char *ch){
	
	char *buffer;
	int ret=0;
	buffer=readline("\n>> ");
	if(strlen(buffer) > 0 ){
		add_history(buffer);
		strcpy(ch,buffer);
		ret=1;
	}
	return ret;
}


/***
	execCommande : la procedure dans laquelle on execute les commandes (sans pipes)
	@ entrées : la commande en forme de sous chaines de caractères 
	@ sorties : void


***/

void execCommande(char *cmd[]){
	
	//création d'un processus fils
	pid_t pid = fork();

	if (pid <0){
		perror("erreur dans la création du processus fils \n");
		exit(errno);
	}else {
		if(pid == 0){
			//éxecution de la commande voulu par la famille des exec (execlp , execvp , execl , execv ...)
			//TODO: à compléter plus tard , les commandes à executer ici sont celles spécialisées dans la manipulation des tarballs
			exit(0);
		}else{
			
			//attendre que le fils termine son execution
			wait(NULL);
		}
	}
}

/***
	my_pwd : la procédure qui affiche le répértoire courant
	
***/
void my_pwd(){
	char pwd[1024]; 
    getcwd(pwd, sizeof(pwd)); 
    printf("\nRépertoire : %s \n", pwd); 

}

/***
	execCommandePipe : la procedure dans laquelle on execute les commandes avec pipes
	@ entrées : la commande en forme de sous chaines de caractères , et les pipes aussi
	@ sorties : void

***/

//à completer





