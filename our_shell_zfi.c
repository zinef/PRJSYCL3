#include "our_shell_zfi.h"


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

void executerCmdSimple(char *cmd[]){
	
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
	executerCmdComplexe : la fonction avec laquelle on éxecute les commandes avec pipes
	//TODO

***/

/***
	my_pwd : la procédure qui affiche le répértoire courant
	
***/
void my_pwd(){
	char pwd[1024]; 
    getcwd(pwd, sizeof(pwd)); 
    printf("\nRépertoire : %s \n", pwd); 

}



/***
	trouverPipes : la fonction qui trouve les pipes et retournes les commandes séparées 
	@ entrées : entrée de ligne de commande et un tableau dans lequel on va récupérer les commandes séparées
	@ sorties : booléen 


***/
int trouverPipe(char *entree,char **commandesSiPipe){
	
	int indiceCmd=0;
	//parcours de la chaine de caractères et recherche de "|" pour pouvoir séparer les commandes
	int stop=0;

	while((indiceCmd <= MAXCMDs) && (stop == 0)){
		commandesSiPipe[indiceCmd] = strsep(&entree,"|");
		if(commandesSiPipe[indiceCmd] == NULL) {stop=1;}
			
		indiceCmd++;
	}
	//test s'il existe au moins un pipe "|"
	if(commandesSiPipe[1] == NULL){
		return 0;	
	}else{
		return 1;
	}
}

/***

	recupArgs : fonction pour récuperer les arguments d'une commande
	@entrées : entrée 
	@sorties : void


***/
void recupArgs(char *entree,char **listeArgs){

	int indiceArg=0;
	int stop=0;
	
	while(indiceArg<MAXCMDs && stop == 0){
		listeArgs[indiceArg] = strsep(&entree," ");
		if(listeArgs[indiceArg] == NULL ) stop=1;
		//if(strlen(listeArgs[indiceArg]) == 0) indiceArg -=1;
		indiceArg++;
	}
	
}
/***





***/


/***
	commandeValide : la commande qui vérifie si une commande existe dans le shell (i.e. existe dans le tableau des commande ) et execute cette dernière .
	@entrées :commande sous forme de chaine de caractère  
	@sorties :booléen

***/
int commandeValide(char **listeArgs){
	int exist=0;
	int indiceCommande=0;
	int i=0;

	while(i<NBCMD && exist == 0){
		if (strcmp(listeArgs[0] , listeDesCommande[i]) == 0){
			exist = 1;
			indiceCommande=i;
		}
		i++;
	}
	if(exist){
		//TODO:éxecution de la commande
		printf("la commande existe dans le shell\n");
		return 1;
	}else{
		printf("la commande n'existe pas dans le shell\n");
		return 0;
	}
}

/***
	décortiquerEntree : la procedure avec laquelle on analyse la commande entrée 
	@ entrées : entree , liste arguments , liste des arguments si pipé
	@ sorties : numéro de cas de commande 1 ou 2
	
***/
int decortiquerEntree(char *entree,char **listeArgs,char **listeArgsPipe){
	
	//là ou on va récuperer la liste des commande si l'entrée est pipé
	char *commandesSiPipe[MAXCMDs];

	//on teste si l'entree est une commande complexe
	int complexe = trouverPipe(entree,commandesSiPipe);
	if(complexe){//i.e il existe au moins un pipe
		//analyse de chaque commande seul pour pouvoir déterminer tous les arguments
		recupArgs(commandesSiPipe[0], listeArgs );
		for(int i=1 ;i<MAXCMDs;i++){
			if (commandesSiPipe[i] != NULL){
				recupArgs(commandesSiPipe[i], listeArgsPipe );
			}else{
				break;
			}
		}
	}else{
		recupArgs(entree,listeArgs);
	}
	
	//tester si la commande est une commande valide dans le shell
	if(commandeValide(listeArgs)){
		//retourner 1 si c'est une commande interne au shell et simple sinon 2 si la commande est complexe
		return 1+complexe;
	}
}




