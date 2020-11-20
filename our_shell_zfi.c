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
	my_pwd_global : la procédure qui affiche le répértoire courant , dans le cas des fichier tar

***/
void my_pwd_global(){
	if (in_tar){
		printf("Répertoire : %s \n",pwd_global);
	}else{
		my_pwd();
	}
}
/***
	my_cd: la procedure qui permet de se déplacer dans un répértoire (un chemin qui n'inclus pas des tar)
	entrées: path
	sorties:void
***/
void my_cd(char *path){
		char pwd[1024];
	    getcwd(pwd,sizeof(pwd));
		strcat(pwd,"/");
		char *chemin=strcat(pwd,path);
		int flag=chdir(chemin);
		if(flag ==0) {
			
			chdir(pwd);
			strcpy(pwd_global,"");
			strcpy(pwd_global,chemin);
			strcpy(tar_actuel,"");
			in_tar=0;
		}
}
/***
	déplacement_in_tar : fonction pour se déplacer dans le tar (personalisée pour cd)


***/
void deplacement_in_tar(char *path,int entete_a_lire){

	//à modifier
	int fd=open(tar_actuel,O_RDONLY);
	if (fd<0){
		perror("erruer dans l'ouverture");
		exit(errno);
	}
	//seek
	lseek(fd,entete_a_lire*BLOCKSIZE,SEEK_SET);
	//lecture et bufferisation du répértoire 
	int size;
	struct posix_header *st =malloc(sizeof(struct posix_header));
	char buf[513];
	int n=read(fd,buf,BLOCKSIZE);
	if (n<0){
		perror("erreur dans la lecture");
		exit(errno);
	}
	st= (struct posix_header * ) buf;
	printf("nom rép :%s\n",st->name);
	sscanf(st->size,"%o",&size);
	char *buf_rep=malloc(size);
	n=read(fd,buf_rep,size);
	if (n<0){
		perror("erreur dans la lecture");
		exit(errno);
	}
	close(fd);
}
/***
	verif_exist_rep_in_tar: liste des fichiers et répértoire dans un fichier tar
	

***/
int verif_exist_rep_in_tar(char *nomfic,char *path,int *entete_lu){
	
	int fd=open(nomfic,O_RDONLY);
	if (fd<0){
		perror("erruer dans l'ouverture");
		exit(errno);
	}
	char buf[513];
	int EnteteAlire=0;
	int stop=0;
	int size=0;
	int n=0;
	int ret=0;
	
	struct posix_header *st =malloc(sizeof(struct posix_header));
	while((stop==0)&&((n=read(fd,buf,BLOCKSIZE))>0)){
		//printf("entete à lire = %d\n",EnteteAlire);
		st= (struct posix_header * ) buf;
		//printf("nom fichier = %s\n",st->name);
		
		if((st->name)[0] == '\0'){
			stop=1;
			printf("c'est Carré !!\n");
		}
		if (strcmp(st->name,path)==0){
			stop=1;
			*entete_lu=EnteteAlire;
		}
		sscanf(st->size,"%o",&size);
		//printf("taille du fichier = %d\n",size);
		if(size==0){
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS);
		}
		else{
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS)+1;
		}

		lseek(fd,EnteteAlire*BLOCKSIZE,SEEK_SET);
	}
	if((stop==1)&&((st->name)[0] != '\0')&&(st->typeflag == '5')){
		//printf("success\n");
		ret=1;
	}

	if (n<0){
		perror("erreur dans la lecture");
		exit(errno);
	}
	close(fd);

	return ret;

}
/***
	strrev : fonction qui renverse l'ordre d'une chaine de caractère
***/
char *strrev(char *str) {
	  
      char *p1, *p2;
      strcpy(strTmp,str);
 
      if (! strTmp || ! *str) return strTmp;
      for (p1 = strTmp, p2 = strTmp + strlen(str) - 1; p2 > p1; ++p1, --p2) {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return strTmp;
} 
/***
	verifier_exist_rep: la fonction avec laquelle on verifie si le sous répértoire donnée en paramètre existe sois dans un répértoire ordinaire ou bien dans un répértoire archivée
	entrées: path
	sorties: <=0 s'il n'existe pas ,1 répértoire simple ,2 répértoire incluant un tar ,3 répértoire incluant un tar pour la première fois

***/
int verifier_exist_rep(char path[100],int *entete_lu,char chemin_absolu[100]){
	int ret=0;
	if(in_tar){
		//tester si path existe dans les répértoires du tar actuel 
		
		ret=verif_exist_rep_in_tar(tar_actuel,path,entete_lu);
		if(ret) {
			ret++;
			int i=strlen(strstr(pwd_global,".tar/"));
			int k=0;
			int j;
			for(j=strlen(pwd_global) - i + 5 ;j<strlen(pwd_global) - i + 5+ strlen(path);j++){
				pwd_global[j]=path[k];
				k++;
			}
			pwd_global[j+1]='\0';
		}
	}else{
		char *pwd= getcwd(NULL, 0);
		char origin[100]="";
		strcpy(origin,pwd);
		char *chemin=strcat(pwd,"/");
		strcat(chemin,path);
		int flag=chdir(chemin);
		if(flag == 0) {
			ret=1;
			chdir(origin);
		}else{
			//tester si path inclus un tar 
			//on cherche à se déplacer dans un tar 
			strcpy(path,"");
			strcpy(path,chemin);
			strcpy(chemin_absolu,path);
			int i=strlen(strstr(path,".tar/"));
			if(i>0){
				char sub_path[100]="";
				strncpy(sub_path,&path[strlen(path) - i + 5], i);
				//récupération du nom du fichier tar
				char tmp_tar[100]="";
				char *rev;
				int j=0;
				while(path[strlen(path) -i-1] != '/'){
					tmp_tar[j]=path[strlen(path) -i - 1];
					i++;
					j++;
				}
				rev=strrev(tmp_tar);
				strcpy(tmp_tar,rev);
				strcat(tmp_tar,".tar");
				
				ret=verif_exist_rep_in_tar(tmp_tar,sub_path,entete_lu);
				if(ret){
					ret++;
					ret++;
					//modification des variable globales tar actuel et in_tar et pwd_global
					strcpy(tar_actuel,"");
					strcpy(tar_actuel,tmp_tar);
					in_tar=1;
					strcpy(pwd_global,"");
					strcpy(pwd_global,path);
				}
			}
		}
	}
	return ret;
}
/***

	my_cd_global: la procedure avec laquelle on change de répértoire de travaille 
	entrées : chaine de caractère , path
	sorties : void
***/

void my_cd_global(char *path){
	//chercher si path est un simple répértoire dans la hièrarchie du répértoire courant , sinon si il existe dans un des tar de cette hièrarchie
	int *entete_lu=malloc(sizeof(int));
	char chemin_absolu[100]="";
	int check_path=verifier_exist_rep(path,entete_lu,chemin_absolu);
	char wd[1024];
	
	//si le répértoire existe inclus un tar in_tar=1
	//sinon in_tar=0 et le répértoire est simple (inclus pas un tar)
	//sinon erreur
	if(check_path>0){
		//déplacement
		if(in_tar){//wd est dans un tar et l'arrivée est dans un tar
			//déplacement vers le répértoire et récupération du contenu dans un buffer pour pouvoir faire des opérations sur ce dernier
			//modification de pwd_global
			// check_path doit être à 1
			deplacement_in_tar(chemin_absolu,*entete_lu);
		}else{
			if(check_path == 3){//premier déplacement vers un répértoire dans le tar //on a l'entete du répértoire dans entete_lu et le tar c'est tar_actuel
				//déplacement vers le répértoire et récupération du contenu dans un buffer pour pouvoir faire des opérations sur ce dernier
				//modification de pwd_global
				deplacement_in_tar(chemin_absolu,*entete_lu);
			}else{

				my_cd(path);
			}
		}
	}else{
		perror("Le chemin n'existe pas");
	}
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
/***
	recup_ext: la fonction qui récupère l'extention d'un fichier 
	entrées: fichier
	sorties: extension

	on va se contenter de tester l'extention du fichier pour savoir si c'est un tar 
	c'est pas toujours vrai mais on va supposer que y'a pas de fcihier non intègre 
	car l'utilisation de la commande tar est interdite on pourras pas tester l'integrité d'un fichier tar autrement
***/
const char *recup_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}


