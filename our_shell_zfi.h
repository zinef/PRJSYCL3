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
#define MAXARGs 10
#define MAXENTREE 1024 
#define NBCMD 10
#define NBPARAMS 10
#define NBOPTIONS 10
#define BLOCKSIZE 512
#define BLOCKBITS 9

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static volatile int running = 1;

/*working directory*/
char pwd_global[1024]="";
int in_tar=0; 
char tar_actuel[1024]="";
char strTmp[255]; 
char Buffer[BLOCKSIZE];//buffer pour lectures et ecritures (utilisé par certaines fonctions)
int std_out_copy , std_error_copy , std_in_copy; //des variables pour sauvgarder les entrées et sorties pour les redirections
/*la liste des commandes valables dans le Shell */
char *listeDesCommande[NBCMD]={"cd","rm","pwd","mkdir","rmdir","exit","ls","cat","cp","mv"};//c'est bien le nom des commandes ,bien evidemment sur quelques une , on peut appliquer des options
/*la structure d'une commande */
struct Commande{
	char nomCommande[30];
	int nbParamsEffect;
	char parametresCommande[NBPARAMS];
	int nbOptionsEffect;
	char *optionsCommande[NBOPTIONS];
};
struct command //structure pour les commandes complexes (i.e. avec pipes)
{
   char **argv;
};
/*les prototypes des fonctions */
int trouverPipe(char *entree,char **commandesSiPipe);
void Initialiser_shell();
int recupEntry(char *ch);
void executerCmdComplexe(char *cmd[]);
void executerCmdSimple(char *cmd[]);
void recupArgs(char *entree,char **listeArgs);
int commandeValide(char **listeArgs);
int decortiquerEntree(char *entree,char **listeArgs,char **commandesSiPipe);
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
void dupliquer_proc (int in, int out, struct command *cmd);
void fork_pipes (int n, struct command *cmd);
//rm 
int  open_tar_file(char ch[100]);
int get_file_size(struct posix_header *header);
void seek_n_block(int fd ,int nb_block);
void seek_next_entete (int fd ,struct posix_header *header);
int rm_in_tar(int fd,char file_name[100],char chaine[100]);
int rm(char chaine[100]);
//ls ,cat ,redirections
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
int parssing_red(char chainne[100],char *listArgsRed[100]);
char* get_type (char symb[8],char before);
int strcmp_red(char *str);
int start_all_redirect(int *fd1,int *fd2,int *fd3,char *out_file1, char *out_file2 ,char *out_file3, char *listArgsRed[100],int *nb_red);
void end_all_redirect(int *fd1, int *fd2, int *fd3, char *out_file1,char *out_file2,char *out_file3, char *listArgsRed[100], int nb_red);
//cp et mv
void recherche_tar(int fd, char nomfic[100],int *trouv, int *entete,int *size_file);
int open_tar_file_rdwr(char ch[100]);
int checkIfValide(char source[100], char destination[100]);
int posix_to_buffer(char ch_in_tar[100],int fdS, int fdD);
int cp_normal(char source[100], char destination[100]);
int cp_source_tar(char source[100], char destination[100]);
int cp_destination_tar(char source[100], char destination[100]);
int cp_source_destination_tar(char source[100], char destination[100]);
int cp (char source[100], char destination[100]);
void mv(char source[100], char destination[100]);
void filter_cmd(char *listArgsRed[100],char *listeArgs[MAXCMDs]);
char *recupere_nom(char *path);
#endif //__OUR_SHELL_ZFI_INCLUDED__
