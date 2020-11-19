#ifndef __OUR_SHELL_ZFI_INCLUDED__
#define __OUR_SHELL_ZFI_INCLUDED__

#include <stdio.h> 
#include <string.h>  
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
/*L'utilisation des deux bibliothèques ci-dessous n'est en aucun cas une utilisation définitive , c'est juste une utilisation 
pour le débougage des premières fonctionnalités qui regroupent essentiellement des fonctionnalités pour manipuler n'importe quel shell
et en cas particulier un shell pour les archives Tar (notre premier rendu consiste en les fonctions qui vont manipuler par la suite les 
commandes spécialisées dans les fichiers *.tar), c'est pour répondre à la spécification de sauvgarde de l'historique de commande fournies par
ces deux bibliothèques */
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tar.h"

#define ever ;;
#define MAXCMDs 10
#define MAXENTREE 1024 
#define NBCMD 4
#define NBPARAMS 10
#define NBOPTIONS 10
#define BLOCKSIZE 512
#define BLOCKBITS 9

/*working directory*/
char pwd_global[1024]="./";//initialisation : répertoire ou le shell se lance
int in_tar=0; 
char tar_actuel[1024]="test.tar";
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
void my_pwd_global();
void my_cd(char *fic);
void my_cd_global(char *path);
const char *recup_ext(const char *filename);
int verif_exist_rep_in_tar(char *nomfic,char *path,int *entete_lu);

#endif //__OUR_SHELL_ZFI_INCLUDED__
