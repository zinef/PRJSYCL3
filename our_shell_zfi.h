#ifndef __OUR_SHELL_ZFI_INCLUDED__
#define __OUR_SHELL_ZFI_INCLUDED__

#include <stdio.h> 
#include <string.h>  
#include <unistd.h> 
#include <stdlib.h> //stdlib pour les fonctions exit et getenv
#include <sys/types.h> 
#include <sys/wait.h> 
/*L'utilisation des deux bibliothèques ci-dessous n'est en aucun cas une utilisation définitif , c'est juste une utilisation 
pour le débougage des premières fonctionnalités qui regroupe essentiellement des fonctionnalités pour manipuler n'importe quel shell
et en cas particulier un shell pour les archives Tar (notre premier rendu consiste en les fonctions qui vont manipuler par la suite les 
commandes spécialisées dans les fichiers *.tar), c'est pour répondre à spécification de sauvgarde de l'historique de commande fournies par
ces deux bibliothèques */
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>

#define ever ;;
#define MAXCMDs 10
#define MAXENTREE 1024 
#define NBCMD 4
#define NBPARAMS 10
#define NBOPTIONS 10

/*la liste des commandes valables dans le Shell */
char *listeDesCommande[NBCMD]={"cd","ls","pwd","mkdir"};
/*la structure d'une commande */
struct Commande{
	char nomCommande[30];
	int nbParamsEffect;
	char parametresCommande[NBPARAMS];
	int nbOptionsEffect;
	char *optionsCommande[NBOPTIONS];
};

/*les prototypes des fonctions */
int trouverPipe(char *entree,char **commandesSiPipe);
void Initialiser_shell();
int recupEntry(char *ch);
void executerCmdSimple(char *cmd[]);
void my_pwd();
void recupArgs(char *entree,char **listeArgs);
int commandeValide(char **listeArgs);
int decortiquerEntree(char *entree,char **listeArgs,char **listeArgsPipe);


#endif //__OUR_SHELL_ZFI_INCLUDED__
