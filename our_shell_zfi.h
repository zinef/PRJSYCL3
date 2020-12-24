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
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include "tar.h"

#define ever ;;
#define MAXCMDs 10
#define MAXENTREE 1024 
#define NBCMD 8
#define NBPARAMS 10
#define NBOPTIONS 10
#define BLOCKSIZE 512
#define BLOCKBITS 9


static volatile int running = 1;

/*working directory*/
char pwd_global[1024]="";//initialisation : répertoire ou le shell se lance
int in_tar=0; 
char tar_actuel[1024]="";
char strTmp[255]; 
char Buffer[BLOCKSIZE];//buffer pour lectures et ecritures (utilisé par certaines fonctions)
int std_out_copy , std_error_copy , std_in_copy; //des variables pour sauvgarder les entrées et sorties
/*la liste des commandes valables dans le Shell */
char *listeDesCommande[NBCMD]={"cd","rm","pwd","mkdir","rmdir","exit","ls","cat"};
/*la structure d'une commande */
struct Commande{
	char nomCommande[30];
	int nbParamsEffect;
	char parametresCommande[NBPARAMS];
	int nbOptionsEffect;
	char *optionsCommande[NBOPTIONS];
};
struct command
{
  const char **argv;
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
int my_mkdir(char *nom_rep);
int verif_exist_rep_in_tar_for_mkdir(char *nomfic,char *path,int *entete_lu,int *entete_a_modifier,int *trouve);
int my_rmdir(char *nom_rep);
int verif_exist_rep_in_tar_for_rmdir(char *nomfic,char *path,int *entete_lu,int *cpt);
int startsWith(const char *pre, const char *str);
int countOccurrences(char * str, char * toSearch);
void chop(char *str, size_t n);
void my_exit();
int dupliquer_proc (int in, int out, struct command *cmd);
int fork_pipes (int n, struct command *cmd);
//rm 
int  open_tar_file(char ch[100]);
int get_file_size(struct posix_header *header);
void seek_n_block(int fd ,int nb_block);
void seek_next_entete (int fd ,struct posix_header *header);
int rm_in_tar(int fd,char file_name[100]);
int rm(char chaine[100]);
//is
void stop(int a);
void get_entete_info(posix_header *header,char buffer[BLOCKSIZE]);
int cmp_name(char str_cmp[100], char str_cmp2[100]);
int open_tar_file_read(char ch[100]);
void get_ch_absolu(char *ch);
void remove_points(char *ch);
void remove_2points(char *ch);
int check_dir(char header_name[100],char dir_name[100]);
int get_dir(int fd,char dir_name[100], posix_header *header);
void permision(struct stat st,char *str);
void write_in_tar(posix_header *header,int option);
void ls_in_tar(int fd,char dir_name[100],int option,char ch[100]);
void write_out_tar(char file_name[100],char dir_name[100],int option);
void ls_out_tar(char dir_name[100],char ch[100],int option);
void ls(char ch[100],int option);
int get_file(int fd,char file_name[100], posix_header *header,char ch[100]);
void cat_in_tar(int fd,char file_name[100],char ch[100]);
void cat_out_tar(char file_name[100],char ch[100]);
void cat(char ch[100]);
int search(char ch[100]);
void end_redct(char ch[100],int fd,char *out_file,int type);
int redercet_stdout(char ch[100],char *out_file,int type);
int redercet_stdin(char ch[100]);
int redirect_res(char ch[100],char *out_file,char type[10]);
void end_redirect(char ch[100],int fd ,char *out_file,char type[10]);
#endif //__OUR_SHELL_ZFI_INCLUDED__
