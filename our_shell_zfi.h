#ifndef __OUR_SHELL_ZFI_INCLUDED__
#define __OUR_SHELL_ZFI_INCLUDED__

#include <stdio.h> 
#include <string.h>  
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include "tar.h"

#define ever ;;
#define MAXCMDs 10
#define MAXENTREE 1024 
#define NBCMD 6
#define NBPARAMS 10
#define NBOPTIONS 10
#define BLOCKSIZE 512
#define BLOCKBITS 9

/*working directory*/
char pwd_global[1024]="";//initialisation : répertoire ou le shell se lance
int in_tar=0; 
char tar_actuel[1024]="";
char strTmp[255]; 
/*la liste des commandes valables dans le Shell */
char *listeDesCommande[NBCMD]={"cd","rm","pwd","mkdir","rmdir","exit"};
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
void recupArgs(char *entree,char **listeArgs);
int commandeValide(char **listeArgs);
int decortiquerEntree(char *entree,char **listeArgs,char **listeArgsPipe);
char* my_pwd_global();
void my_cd(char *fic);
void my_cd_global(char *path);
const char *recup_ext(const char *filename);
int verifier_exist_rep(char path[100],int *entete_lu,char chemin_absolu[100]);
int verif_exist_rep_in_tar(char *nomfic,char *path,int *entete_lu);
char *strrev(char *str);
int deplacement_in_tar(char *path,int entete_a_lire);
//void process_pwd_global();
int my_mkdir(char *nom_rep);
int verif_exist_rep_in_tar_for_mkdir(char *nomfic,char *path,int *entete_lu,int *entete_a_modifier,int *trouve);
int my_rmdir(char *nom_rep);
int verif_exist_rep_in_tar_for_rmdir(char *nomfic,char *path,int *entete_lu,int *cpt);
int startsWith(const char *pre, const char *str);
int countOccurrences(char * str, char * toSearch);
void chop(char *str, size_t n);
void my_exit();
//rm 
int  open_tar_file(char ch[100]);
int get_file_size(struct posix_header *header);
void seek_n_block(int fd ,int nb_block);
void seek_next_entete (int fd ,struct posix_header *header);
void rm_in_tar(int fd,char file_name[100]);
int rm(char chaine[100]);
#endif //__OUR_SHELL_ZFI_INCLUDED__
