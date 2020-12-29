#include "our_shell_zfi.h"


/*Implémentation des fonctionnalités*/


/***
	Initialiser_shell : procédure d'affichage après le lancement du shell

***/

void Initialiser_shell(){
	write(1,ANSI_COLOR_CYAN,strlen(ANSI_COLOR_CYAN));
	write(1,"\033[H\033[J",strlen("\033[H\033[J"));
	write(1,"\n***********************************\n",strlen("\n***********************************\n"));
	write(1,"\n***********OUR SHELL ZFI***********\n",strlen("\n***********OUR SHELL ZFI***********\n"));
	char *id=getenv("USER");
	write(1,"\n \tBienvenu @",strlen("\n \tBienvenu @"));write(1,id,strlen(id));write(1," \n",strlen(" \n"));
	write(1,"\n***********************************\n",strlen("\n***********************************\n"));
	write(1,"\n***********************************\n",strlen("\n***********************************\n"));
	sleep(1);
	write(1,"\033[H\033[J",strlen("\033[H\033[J"));
	write(1,ANSI_COLOR_RESET,strlen(ANSI_COLOR_RESET));
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
	executerCmdComplexe : la fonction dans laquelle on execute les commandes complexes 
	entrées : liste des commandes
	sorties : void ( execution )
***/
void executerCmdComplexe(char *cmd[]){
	int i=0,ret,k,j,n=0;
	char *listArgsRed[100];
	struct command tcmd [MAXCMDs];
	char **c=(char **)malloc(MAXARGs*sizeof(char *));
	//initialisation des tableaux
	for(k=0;k<MAXCMDs;k++){
		tcmd[k].argv = (char **)malloc(MAXARGs*sizeof(char *));
	}
	for(k=0;k<MAXCMDs;k++){
		for(i=0;i<MAXARGs;i++){
			tcmd[k].argv[i]=(char *)malloc(10*sizeof(char));
		}
	}
	for(k=0;k<MAXARGs;k++){
		c[i]=(char *)malloc(10*sizeof(char));
	}

	for(i=0;i<MAXCMDs;i++){
		if((cmd[i] != NULL)&&(strcmp(cmd[i],"")!=0)){
			n++;
			for(k=0;k<MAXARGs;k++){
				c[i]=NULL;
			}
			recupArgs(cmd[i],c);
			for(j=0;j<MAXARGs;j++){
				if(c[j] != NULL){
					sprintf(tcmd[i].argv[j],"%s",c[j]);
				}else{
					break;
				}
			}
		}else{
			break;
		}
	}
	fork_pipes(n,tcmd);
}
/***
	execCommande : la procedure dans laquelle on execute les commandes (sans pipes)
	@ entrées : la commande en forme de sous chaines de caractères 
	@ sorties : void


***/

void executerCmdSimple(char *cmd[]){
	
	
	if(strcmp(cmd[0],"cd") == 0){
		if((cmd[1] != NULL)&&(strcmp(cmd[1],"")!=0)){
			my_cd_global(cmd[1]);
		}else{
			char* id=getenv("USER");
			char env[50]="/home/";
			strcat(env,id);
			my_cd_global(env);
		}
	}else{
		if(strcmp(cmd[0],"exit") == 0){
			my_exit();
		}else{
			if(strcmp(cmd[0],"mkdir") == 0){
				for(int k=1;k<MAXARGs;k++){
					if(cmd[k] != NULL){
						my_mkdir(cmd[k]);
					}
				}
			}else{
				if(strcmp(cmd[0],"rmdir") == 0){
					for(int k=1;k<MAXARGs;k++){
						if(cmd[k] != NULL){
							my_rmdir(cmd[k]);
						}
					}
				}else{
					if(strcmp(cmd[0],"pwd") == 0){
						write(1,pwd_global,strlen(pwd_global));
						write(1,"\n",sizeof("\n"));
					}else{
						if(strcmp(cmd[0],"rm") == 0){
							if(cmd[1] != NULL){
								if(strcmp(cmd[1],"-r")==0){
									if(cmd[2] != NULL)
										rm_r(cmd[2]);
								}else{
									rm(cmd[1]);
								}
							}
						}else{
							if(strcmp(cmd[0],"ls")==0){
								if(cmd[1] != NULL){
									if(strcmp(cmd[1],"-l")==0){
										if(cmd[2] != NULL){
											ls(cmd[2],1);
										}else{
											ls("",1);
										}
									}else{
										ls(cmd[1],0);
									}
								}else{
									ls("",0);
								}
							}else{
								if(strcmp(cmd[0],"cat")==0){
									if(cmd[1] != NULL){
										cat(cmd[1]);
									}else{
										cat("");
									}
								}else{	
									if(strcmp(cmd[0],"cp")==0){
										if(cmd[1]!=NULL && cmd[2] != NULL){
											cp(cmd[1],cmd[2]);
										}
									}else{
										if(strcmp(cmd[0],"mv")==0){
											if(cmd[1]!=NULL && cmd[2] != NULL){
												mv(cmd[1],cmd[2]);
											}
										}else{
										}
									}				
								}
							}
						}
					}
				}
			}
		}
	}
}


/***
	my_pwd_global : la procédure qui affiche le répértoire courant 

***/
char* my_pwd_global(){
		return pwd_global;
}
/***
	my_cd: la procedure qui permet de se déplacer dans un répértoire (un chemin qui n'inclus pas des tar)
	entrées: path
	sorties:void
***/
void my_cd(char *path){
	int flag;
	char *chemin;
		if(path[0] == '/'){
			flag=chdir(path);
			if(flag == 0) {
			
				//chdir(pwd);
				strcpy(pwd_global,"");
				strcpy(pwd_global,path);
				strcpy(tar_actuel,"");
				in_tar=0;
			}else{
				write(1,"cd : No such directory\n",strlen("cd : No such directory\n"));
			}
		}else{
			char pwd[1024];
			getcwd(pwd,sizeof(pwd));
			strcat(pwd,"/");
			chemin=strcat(pwd,path);
			flag=chdir(chemin);
			if(flag == 0) {
				strcpy(pwd_global,"");
				remove_points(chemin);
    			remove_2points(chemin);
    			if((strcmp(chemin,"/") != 0)&&(chemin[strlen(chemin)-1] == '/')){
    				chemin[strlen(chemin)-1]='\0';
    			}
				strcpy(pwd_global,chemin);
				strcpy(tar_actuel,"");
				in_tar=0;
			}else{
				write(1,"cd : No such directory\n",strlen("cd : No such directory\n"));
			}
	}
}
/***
	déplacement_in_tar : fonction pour se déplacer dans le tar (personalisée pour cd)

***/
int deplacement_in_tar(char *path,int entete_a_lire){

	//à modifier
	int fd=open(tar_actuel,O_RDONLY);
	if (fd<0){
		write(1,"No such file or directory\n",strlen("No such file or directory\n"));
		return -1 ;
	}
	//seek
	lseek(fd,entete_a_lire*BLOCKSIZE,SEEK_SET);
	//lecture et bufferisation du répértoire 
	int size;
	struct posix_header *st =malloc(sizeof(struct posix_header));
	char buf[513];
	int n=read(fd,buf,BLOCKSIZE);
	if (n<0){
		write(1,"Erreur dans la lecture\n",strlen("Erreur dans la lecture\n"));
		return -1 ;
	}
	st= (struct posix_header * ) buf;
	sscanf(st->size,"%o",&size);
	char *buf_rep=malloc(size);
	n=read(fd,buf_rep,size);
	if (n<0){
		write(1,"Erreur dans la lecture\n",strlen("Erreur dans la lecture\n"));
		return -1 ;
	}
	//free(buf_rep);
	//free(st);
	
	close(fd);
}
/***
	verif_exist_rep_in_tar: liste des fichiers et répértoire dans un fichier tar
	entrées : nom du fichier tar , chemin dans le tar , un entier pour retourner l'entete lu si c'est le cas 
	sorties : entier
***/
int verif_exist_rep_in_tar(char *nomfic,char *path,int *entete_lu){
	
	int fd=open(nomfic,O_RDONLY);
	if (fd<0){
		write(1,"No such file or directory\n",strlen("No such file or directory\n"));
		return -1;
	}
	if (strcmp(path,"") == 0 ){//déplacement dans la racine d'un tar
		return 1;
	}
	char buf[513];
	int EnteteAlire=0;
	int stop=0;
	int size=0;
	int n=0;
	int ret=0;
	
	struct posix_header *st =malloc(sizeof(struct posix_header));
	while((stop==0)&&((n=read(fd,buf,BLOCKSIZE))>0)){

		st= (struct posix_header * ) buf;
		
		if((st->name)[0] == '\0'){
			stop=1;
		}
		if (strcmp(st->name,path)==0){
			stop=1;
			*entete_lu=EnteteAlire;
		}
		sscanf(st->size,"%o",&size);
		
		if(size==0){
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS);
		}
		else{
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS)+1;
		}

		lseek(fd,EnteteAlire*BLOCKSIZE,SEEK_SET);
	}
	if((stop==1)&&((st->name)[0] != '\0')&&(st->typeflag == '5')){
		
		ret=1;
	}

	if (n<0){
		write(1,"zfi : No such file \n",strlen("zfi : No such file \n"));
		return -1;
	}
	//free(st);
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
	int ret=0,i=0;
	if(in_tar){
		
		if(startsWith("../",path)){
			int n=countOccurrences(path,"../");
			int i=0;
			char tmp[1024]="";
			strcpy(tmp,pwd_global);
			while(n>0){
				chop(path,3);
				i=2;
				while(tmp[strlen(tmp)-i] != '/'){
					i++;
				}
				tmp[strlen(tmp)-i+1]='\0';
				n--;
			}
			if(strstr(tmp,".tar/") == NULL){
					in_tar=0;
					strcpy(tar_actuel,"");
			}
			strcpy(pwd_global,"");
			strcpy(pwd_global,tmp);
			ret=4;
		}else{
			//tester si path existe dans les répértoires du tar actuel 
			if(strstr(pwd_global,".tar/") != NULL){
				i=strlen(strstr(pwd_global,".tar/"));
			}
			char tmp[1024]="";
			strncpy(tmp,&pwd_global[strlen(pwd_global)-i+5],i);
			strcat(tmp,path);
			strcpy(path,"");
			strcpy(path,tmp);
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
			}else{
				write(1,"zfi : cd  :No such directory\n",strlen("zfi : cd  :No such directory\n"));
			}
		}
	}else{

		char pwd[1024]; 
		strcpy(pwd,pwd_global);
		char origin[100]="";
		strcpy(origin,pwd);
		char *chemin=strcat(pwd,"/");
		strcat(chemin,path);
		int flag=chdir(chemin);
		if ((flag < 0 )&&(strstr(chemin,".tar/") == NULL)){
			write(1,"zfi : cd  :No such directory\n",strlen("zfi : cd  :No such directory\n"));
			return -1 ;
		}
		if(flag == 0) {
			ret=1;
			if(strcmp(path,"./")==0)
				ret=4;
			chdir(origin);
		}else{
			//tester si path inclus un tar 
			//on cherche à se déplacer dans un tar 
			if(path[0] != '/'){
				strcpy(path,"");
				strcpy(path,chemin);
				strcpy(chemin_absolu,path);
			}
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
				char parent_dir_of_tar[100]="";
				if(path[0] != '/'){
					strncpy(parent_dir_of_tar,chemin_absolu,strlen(chemin_absolu)-i-1);
				}else{
					strncpy(parent_dir_of_tar,path,strlen(path)-i-1);
				}
				chdir(parent_dir_of_tar);
				ret=verif_exist_rep_in_tar(tmp_tar,sub_path,entete_lu);
				if(ret>0){
					ret++;
					ret++;
					//modification des variable globales tar actuel et in_tar et pwd_global
					strcpy(tar_actuel,"");
					strcpy(tar_actuel,tmp_tar);
					in_tar=1;
					strcpy(pwd_global,"");
					strcpy(pwd_global,path);
				}else{
					chdir(origin);
					strcpy(pwd_global,"");
					strcpy(pwd_global,origin);
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
	int *entete_lu=malloc(sizeof(int*));
	*entete_lu=0;
	char chemin_absolu[100]="";
	int check_path;
	if(path[0] == '/'){//chemin absolue
		if(strstr(path,".tar/") == NULL){//outtar
			my_cd(path);
		}else{//totar
			check_path=verifier_exist_rep(path,entete_lu,chemin_absolu);
		}
	}else{
		//chercher si path est un simple répértoire dans la hièrarchie du répértoire courant
		int prev_in_tar=in_tar;
		check_path=verifier_exist_rep(path,entete_lu,chemin_absolu);
		char wd[1024];
		if(check_path>0 && check_path != 4){

				//si le répértoire existe inclus un tar in_tar=1
				//sinon in_tar=0 et le répértoire est simple (inclus pas un tar)
				//sinon erreur
					int el=*entete_lu;
					//déplacement
					if(prev_in_tar){//wd est dans un tar et l'arrivée est dans un tar
					
						//modification de pwd_global
						// check_path doit être à 1
						
						deplacement_in_tar(chemin_absolu,el);
					}else{
						if(check_path == 3){//premier déplacement vers un répértoire dans le tar 
							//on a l'entete du répértoire dans entete_lu et le tar c'est tar_actuel 
							//modification de pwd_global
							deplacement_in_tar(chemin_absolu,el);
						}else{

							my_cd(path);
						}
					}
		}
		
	}
	//free(entete_lu);
}
/***
	verif_exist_rep_in_tar: liste des fichiers et répértoire dans un fichier tar
	

***/
int verif_exist_rep_in_tar_for_mkdir(char *nomfic,char *path,int *entete_lu,int *entete_a_modifier,int *trouve){
	
	int fd=open(nomfic,O_RDONLY);
	if (fd<0){
		write(1,"zfi : No such file \n",strlen("zfi : No such file \n"));
		return -1 ;
	}
	char buf[513];
	int EnteteAlire=0;
	int stop=0;
	int size=0;
	int n=0;
	int ret=0;
	
	struct posix_header *st =malloc(sizeof(struct posix_header));
	while((stop==0)&&((n=read(fd,buf,BLOCKSIZE))>0)){
		
		st= (struct posix_header * ) buf;
		
		if((st->name)[0] == '\0'){
			stop=1;
		}
		if(((st->name)[0] == '#')&&(*trouve==0)&&(st->typeflag == '5')){
			*trouve=1;
			*entete_a_modifier=EnteteAlire;
		}
		if (strcmp(st->name,path)==0){
			stop=1;
			*entete_lu=EnteteAlire;
		}
		sscanf(st->size,"%o",&size);
		
		*entete_lu=EnteteAlire;
		if(size==0){
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS);
		}
		else{
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS)+1;
		}
		
		lseek(fd,EnteteAlire*BLOCKSIZE,SEEK_SET);
	}
	if((stop==1)&&((st->name)[0] != '\0')&&(st->typeflag == '5')){
		
		ret=1;
	}

	if (n<0){
		write(1,"zfi : No such file \n",strlen("zfi : No such file \n"));
		return -1 ;
	}
	//free(st);
	close(fd);
	return ret;

}
/***
  	my_mkdir : la fonction avec laquelle on crée des répértoires 
  	entrées:nom des répértoire à crée
	sorties: booléen
***/

int my_mkdir(char *nom_rep){

	//récupération de pwd
	char *pwd=my_pwd_global();
	
	int i;
	if(strstr(pwd,".tar/") != NULL){
		i=strlen(strstr(pwd,".tar/"));
	}else{
		i=0;
	}
	char tmp[100];
	strcpy(tmp,nom_rep);
	
	if(i>0){//le chemin ou doit on créer le repértoire inclus un tar 
		char suite[100]="";
		strncpy(suite,&pwd[strlen(pwd)-i+5],i);
		if(strcmp(suite,"") == 0){
			strcat(tmp,"/");
		}else{
			strcat(suite,tmp);
			strcat(suite,"/");
			strcpy(tmp,"");
			strcpy(tmp,suite);
		}
		char tar_file[100]="";
		strncpy(tar_file,pwd,strlen(pwd)-i+4);

		//tester si le répértoire existe déjà dans le tar actuel
		int *entete_a_modifier=malloc(sizeof(int*));
		int *entete_lu=malloc(sizeof(int*));
		int *trouve=malloc(sizeof(int*));
		*trouve=0;
		int ret=verif_exist_rep_in_tar_for_mkdir(tar_file,tmp,entete_lu,entete_a_modifier,trouve); //entete_a_modifier est l'entete qu'on va modifier pour créer le nouveau rep si ret est == 0
		if(ret>0){
			write(1,"zfi :mkdir : cannot create directory ,The file exists \n",strlen("zfi :mkdir : cannot create directory ,The file exists \n"));
			return -1 ;
		}
		char buf[513]="";

		
		//on ajoute le répértoire si ce n'est pas un doublon
		//on doit écrire à l'entete entete_a_modifier si trouve est vrai i.e. trouve=1
		//sinon on ajoute à la fin du fichier à l'entete entete_lu qui pointe vers la fin du fichier si le répértoire n'existe pas 
		int fd=open(tar_file,O_RDWR);
		struct posix_header *st =malloc(sizeof(struct posix_header));
		if(*trouve){//on modifie le bloc entete_a_modifier en remplacant ses caractéristiques par le nouveau rép qu'on veut créer 
			
			lseek(fd,*entete_a_modifier*BLOCKSIZE,SEEK_SET);
			int n=read(fd,buf,BLOCKSIZE);
			if (n<0){
				write(1,"zfi :mkdir : No such file\n",strlen("zfi :mkdir : No such file\n"));
				return -1 ;
			}
			st=(struct posix_header * ) buf;
			mkdir("/tmp/testspecreptomkdir",0700);
			struct stat statbuf;
			int r=stat("/tmp/testspecreptomkdir",&statbuf);
			remove("/tmp/testspecreptomkdir");
    		sprintf(st -> mtime,"%lo",statbuf.st_mtime);
    		sprintf(st -> uid  ,"%o" ,statbuf.st_uid);
    		sprintf(st -> gid  ,"%o" ,statbuf.st_gid);
			strcpy(st->name,"");
			strcpy(st->name,tmp);
			sprintf(st->size,"%o",0);
			st->typeflag='5';
			sprintf(st->mode,"0000700");//drwx------
			strcpy(st->magic,TMAGIC);
			strcpy(st->version,TVERSION);
			set_checksum(st);
			lseek(fd,*entete_a_modifier*BLOCKSIZE,SEEK_SET);
			write(fd,st,sizeof(struct posix_header));
			
		}else{
			//ajout à la fin du fichier tar 
			lseek(fd,*entete_lu*BLOCKSIZE,SEEK_SET);
			st=(struct posix_header * ) buf;
			mkdir("/tmp/testspecreptomkdir",0700);
			struct stat statbuf;
			int r=stat("/tmp/testspecreptomkdir",&statbuf);
			remove("/tmp/testspecreptomkdir");
    		sprintf(st -> mtime,"%lo",statbuf.st_mtime);
    		sprintf(st -> uid  ,"%o" ,statbuf.st_uid);
    		sprintf(st -> gid  ,"%o" ,statbuf.st_gid);
			strcpy(st->name,"");
			strcpy(st->name,tmp);
			sprintf(st->size,"%o",0);
			st->typeflag='5';
			sprintf(st->mode,"0000700");//drwx------
			strcpy(st->magic,TMAGIC);
			strcpy(st->version,TVERSION);
			set_checksum(st);
			int w=write(fd,st,sizeof(struct posix_header));
			if(w<0){
				write(1,"zfi :mkdir : Writing error\n",strlen("zfi :mkdir : Writing error\n"));
				return -1 ;
			}
			
		}
		//free(entete_a_modifier);
		//free(entete_lu);
		//free(trouve);
		//free(st);
		close(fd);
	}else{//le chemin ou doit on créer le répértoire est un répértoire ordinaire 
		//utilisation de la fonction mkdir 
		struct stat *st =malloc(sizeof(struct stat));
		if(stat(tmp,st) == -1){
			mkdir(tmp,0700);
		}else{
			write(1,"zfi :mkdir : cannot create directory ,The file exists \n",strlen("zfi :mkdir : cannot create directory ,The file exists \n"));
			return -1 ;
		}
		//free(st);
	}	
}
/***
	verif_exist_rep_in_tar: liste des fichiers et répértoire dans un fichier tar
	

***/
int verif_exist_rep_in_tar_for_rmdir(char *nomfic,char *path,int *entete_lu,int *cpt){
	
	int fd=open(nomfic,O_RDONLY);
	if (fd<0){
			write(1,"zfi :rmdir : No such file \n",strlen("zfi :rmdir : No such file \n"));
			return -1 ;
	}
	char buf[513];
	int EnteteAlire=0;
	int stop=0;
	int size=0;
	int n=0;
	int ret=0;
	
	struct posix_header *st =malloc(sizeof(struct posix_header));

	while((stop==0)&&((n=read(fd,buf,BLOCKSIZE))>0)){
		st= (struct posix_header * ) buf;
		
		if((st->name)[0] == '\0'){
			stop=1;
			
		}
		if(startsWith(path,st->name)){
			*cpt=*cpt+1;
		}
		if ((strcmp(st->name,path)==0)&&(st->typeflag == '5')){
			*entete_lu=EnteteAlire;
			ret=1;
		}
		sscanf(st->size,"%o",&size);
		if(size==0){
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS);
		}
		else{
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS)+1;
		}

		lseek(fd,EnteteAlire*BLOCKSIZE,SEEK_SET);
	}
	if (n<0){
		write(1,"zfi :rmdir : Writing error \n",strlen("zfi :rmdir : Writing error \n"));
			return -1 ;
	}
	close(fd);
	//free(st);
	return ret;

}
/***
  	my_rmdir : la fonction avec laquelle on suprime des répértoires (vides)  

  	entrées:nom des répértoires à supprimer

	sorties: booléen

***/

int my_rmdir(char *nom_rep){

	//récupération de pwd
	char *pwd=my_pwd_global();
	
	int i;
	if(strstr(pwd,".tar/") != NULL){
		i=strlen(strstr(pwd,".tar/"));
	}else{
		i=0;
	}
	char tmp[100];
	strcpy(tmp,nom_rep);
	
	if(i>0){//le chemin ou doit on supprimer un repértoire inclus un tar 
		char suite[100]="";
		strncpy(suite,&pwd[strlen(pwd)-i+5],i);
		if(strcmp(suite,"") == 0){
			if(tmp[strlen(tmp)-1] != '/')
					strcat(tmp,"/");
		}else{
			strcat(suite,tmp);
			if(tmp[strlen(tmp)-1] != '/'){
				strcat(suite,"/");
			}
			strcpy(tmp,"");
			strcpy(tmp,suite);
		}
		char tar_file[100]="";
		strncpy(tar_file,pwd,strlen(pwd)-i+4);

		//tester si le répértoire existe déjà dans le tar actuel
		int *entete_lu=malloc(sizeof(int*));
		int *cpt=malloc(sizeof(int*));
		*cpt=0;
		int ret=verif_exist_rep_in_tar_for_rmdir(tar_file,tmp,entete_lu,cpt); //entete_a_modifier est l'entete qu'on va modifier pour créer le nouveau rep si ret est == 0
		if(ret==0){
			write(1,"zfi : No such directory\n",strlen("zfi : No such directory\n"));
			return -1; 
		}
		if(*cpt>1){
			write(1,"zfi : rmdir : Not empty directory\n",strlen("zfi : rmdir : Not empty directory\n"));
			return -1;
		}
		//le répértoire existe on verifie si le répértoire est vide , si c'est le cas on le supprime
		
		int fd=open(tar_file,O_RDWR);
		struct posix_header *st =malloc(sizeof(struct posix_header));
		lseek(fd,*entete_lu*BLOCKSIZE,SEEK_SET);
		char buf[513]="";
		int n=read(fd,buf,BLOCKSIZE);
		if (n<0){
			write(1,"zfi : No such file\n",strlen("zfi : No such file\n"));
			return -1;
		}
			
		st=(struct posix_header * ) buf;
		st->name[0]='#'; //supprimer logiquement le repértoire
		set_checksum(st);
		lseek(fd,*entete_lu*BLOCKSIZE,SEEK_SET);
		write(fd,st,sizeof(struct posix_header));
		//free(entete_lu);
		//free(cpt);
		//free(st);
		close(fd);
	}else{//le chemin ou doit on créer le répértoire est un répértoire ordinaire 
		//utilisation de la fonction mkdir 
		struct stat *st =malloc(sizeof(struct stat));
		if(stat(tmp,st) == 0){
			int ret=rmdir(tmp);
			if(ret<0){
				write(1,"zfi: rmdir : Not an empty file \n",strlen("zfi: rmdir : Not an empty file \n"));
			}
		}else{
			write(1,"zfi : No such directory\n",strlen("zfi : No such directory\n"));
			return -1;
		}
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
	
	while(indiceArg<MAXARGs && stop == 0){
		listeArgs[indiceArg] = strsep(&entree," ");
		if(listeArgs[indiceArg] == NULL ) stop=1;
		if(listeArgs[indiceArg] != NULL ){
			if(strlen(listeArgs[indiceArg]) == 0)
				 indiceArg -=1;
		}
		indiceArg++;
	}
}


/***
	commandeValide : la commande qui vérifie si une commande existe dans le shell (i.e. existe dans le tableau des commande ) .
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
		return 1;
	}else{
		return 0;
	}
}

/***
	décortiquerEntree : la procedure avec laquelle on analyse la commande entrée 
	@ entrées : entree , liste arguments , liste des arguments si pipé
	@ sorties : numéro de cas de commande 1 ou 2
	
***/
int decortiquerEntree(char *entree,char **listeArgs,char **commandesSiPipe){
	
	//commandesSiPipe là ou on va récuperer la liste des commande si l'entrée est pipé
	int complexe=0;
	char sep[1]={" "};
	//on teste si l'entree est une commande complexe
	complexe = trouverPipe(entree,commandesSiPipe);
	
	if(!complexe){
		recupArgs(entree,listeArgs);
	}else{
		return 2;
	}
	
	//tester si la commande est une commande valide dans le shell
	if(commandeValide(listeArgs)){
		//retourner 1 si c'est une commande interne au shell et simple sinon 2 si la commande est complexe
		return 1;
	}
}
/***
	recup_ext: la fonction qui récupère l'extention d'un fichier 
	entrées: fichier
	sorties: extension

	on va se contenter de tester l'extention du fichier pour savoir si c'est un tar 
	c'est pas toujours vrai mais on va supposer que y'a pas de fichier non intègre 
	car l'utilisation de la commande tar est interdite on pourras pas tester l'integrité d'un fichier tar autrement
***/
const char *recup_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}
/***
	startsWith : une fonction pour tester si une chaine de caractère commence par une sous chaine
	entrées: les deux chaines 
	sorties: booleen , 1 si vrai 0 sinon

***/
int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : memcmp(pre, str, lenpre) == 0;
}
/***
	countoccurrences : la fnction avec laquelle on calcule le nombre de fois ou toSearch apparait dans str
	entrées : deux chaines de caractères
	sorties : entier (nombre d'occurences)
***/
int countOccurrences(char * str, char * toSearch)
{
    int i, j, found, count;
    int stringLen, searchLen;

    stringLen = strlen(str);      
    searchLen = strlen(toSearch); 

    count = 0;

    for(i=0; i <= stringLen-searchLen; i++)
    {
        
        found = 1;
        for(j=0; j<searchLen; j++)
        {
            if(str[i + j] != toSearch[j])
            {
                found = 0;
                break;
            }
        }

        if(found == 1)
        {
            count++;
        }
    }

    return count;
}
/***
	chop : la fonction avec laquelle on supprime les n premier caractères d'une chaine de caractères 
	entrées : pointeur vers une chaine de caractères et le nombre n de caractères à supprimer
	sorties : void ( modifie directement sur la chaine en question )
***/
void chop(char *str, size_t n)
{
    assert(n != 0 && str != 0);
    size_t len = strlen(str);
    if (n > len)
        return;  // Or: n = len;
    memmove(str, str+n, len - n + 1);
}
/***
	my_exit : la fonction avec laquelle on quitte zfi shell
	entrées: void
	sorties : void
***/
void my_exit(){
	exit(EXIT_SUCCESS);
}
/****
partie rm

*****/
//Une fonction qui ouvre un tarball 
int  open_tar_file(char ch[100])
{
	int j=0;
	char s[100];
	char str2[100];
	strcpy(s,ch);
	int i= strlen(strstr(s,".tar/"));
	strncpy(str2,ch,strlen(ch)-i+4);
	str2[strlen(ch)-i+4]='\0';
	j=open(str2,O_RDWR);
	return j;
}


// Une fonction qui renvoie la taille du fichier 
int get_file_size(struct posix_header *header)
{
	int size;
	sscanf(header-> size, "%o", &size);
	return (size + 512-1)/512;;
}




// Pour sauter de nb_block blocs dans le fichier de descripteur fd  
void seek_n_block(int fd ,int nb_block)
{
	lseek(fd ,nb_block*512,SEEK_CUR);
}

// Passer à la prochaine entete 
void seek_next_entete (int fd ,struct posix_header *header)
{
	seek_n_block(fd,get_file_size(header));
}



int rm_in_tar(int fd,char file_name[100],char chaine[100])
{
	char buf[512];
	int EnteteAlire=0;
	int stop=0;
	int size=0;
	int n=0;
	int ret=0;
	int entete_a_lire;
	struct posix_header *st =malloc(sizeof(struct posix_header));
    char type ,nameToTest[100];

	while((stop==0)&&((n=read(fd,Buffer,BLOCKSIZE))>0)){
		
         get_entete_info (st,Buffer);
		// on teste si on est pas arrivé à la fin 
        if((st->name)[0] == '\0'){
		    stop=1;
		}

		if (strcmp(st->name,file_name)==0){
			stop = 1;
			entete_a_lire = EnteteAlire;
		}

		// on récupère la taille di fichier 
        sscanf(st->size,"%o",&size);
		if(size==0){
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS);// >>BLOCKBITS = /512
		}
		else{
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS)+1; 
		}
		lseek(fd,EnteteAlire*BLOCKSIZE,SEEK_SET);
        
        type = st->typeflag;

        strcpy(nameToTest,st->name); 
        free(st);
        st = malloc(sizeof(posix_header));
	}
    free(st);
    

    if ( type != '0'){
        write(STDERR_FILENO,"rm : ",5 );
        write(STDERR_FILENO,chaine,strlen(chaine));
		write(1," : No such file\n",strlen(" : No such file\n"));
		return -1 ;
	}else if(nameToTest[0] == '\0' || n <= 0 ){
        write(STDERR_FILENO,"rm : ",5 );
        write(STDERR_FILENO,chaine,strlen(chaine));
		write(1," : No such file or directory\n",29);
        return -1;
    }


     // on teste les conditions de sortie 
	if((stop==1)&&((nameToTest)[0] != '\0')){ // on est au bon fichier 
		lseek(fd,0,SEEK_SET); // on se repositionne au debut
		
		struct posix_header *st1 =malloc(sizeof(struct posix_header));
		lseek(fd,entete_a_lire*BLOCKSIZE,SEEK_SET); //on va directement a lentete concernée 
		
		n = read(fd,Buffer,BLOCKSIZE);

		if (n <= 0){
			write(STDERR_FILENO,"rm : ",5 );
            write(STDERR_FILENO,chaine,strlen(chaine));
		    write(STDERR_FILENO," : No such file or directory\n",29);
            free(st1);
			return -1;
		}
			
		get_entete_info( st1,Buffer);
        if(strstr(st1->name,"/")!=NULL){
            int file_name_len = strlen(recupere_nom(st1->name));
            st1->name[strlen(st1->name)-file_name_len]='#';
        }else{
		    st1->name[0]='#'; 
        }
		set_checksum(st1);
		lseek(fd,entete_a_lire*BLOCKSIZE,SEEK_SET);
		write(fd,st1,sizeof(struct posix_header));
		
		//Une suppresion logique 
		//une boucle pour supprimer logiquement (i.e mettre le début des blocs concernés par le fichier à "#" )
		int block= size >> BLOCKBITS; //recuperer le nombre de block de fichier 
		while((block>0)&&((n=read(fd,buf,BLOCKSIZE))>0)){
			write(fd,"#",1);
			block--;  /* supprimer les blocks logiquement */
		}
		free(st1);
	}

    
    return 1;
}



int rm(char chaine[100]) {
    int fd, i;
    char str[100], ch_in_tar[100];
	

    strcpy(str, chaine);

    get_ch_absolu(str);
    remove_points(str);
    remove_2points(str);

    if (strstr(str, ".tar/") == NULL) { // Fonctionnement normal de rm

        struct stat st;
		int ret=stat(str,&st);
		if(ret < 0){
            write(STDERR_FILENO,"rm : ",5 );
            write(STDERR_FILENO,chaine,strlen(chaine));
			write(1," : No such file\n",16);
            
			return -1;
		}
		if((st.st_mode & S_IFMT) == S_IFREG){
        	remove(str);
            return 1;
		}else{
            write(STDERR_FILENO,"rm : ",5 );
            write(STDERR_FILENO,chaine,strlen(chaine));
			write(1," : Not a regular file , Try rm -r\n",34);
			return -1;
		}  
			
    } // Fin du fonctionnement normal de rm
    else {
        fd = open_tar_file_rdwr(str);
        if (fd < 0){ // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
            write(STDERR_FILENO,"rm : ",5 );
            write(STDERR_FILENO,chaine,strlen(chaine));
			write(1," : No such file or directory\n",29);
			return -1;
        }
        // si le chemin vers le tarball existe et que ce dernier est bien ouvert
        // on recupere le chemin jusqu'au tar et on recupere la suite du chemin dans une autre chaine
        i = strlen(strstr(str, ".tar/"));
        strncpy(ch_in_tar, & str[strlen(str) - i + 5], i);
        rm_in_tar(fd, ch_in_tar, chaine );
        close(fd);
    }
    
    return 1;
	
}

/***
fonctions pour execution des pipes

***/
void dupliquer_proc (int in, int out, struct command *cmd){
  pid_t pid;

  if ((pid = fork ()) == 0){
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }

       executerCmdSimple(cmd->argv);//executer_simple command
    }

}


void fork_pipes (int n, struct command *cmd){
	int i;
	pid_t pid,wpid;
	int in, fd [2];
	int status=0;
   //le premier processus doit lire depuis fd 0 
	in = 0;

  //duppliquer les processus
  for (i = 0; i < n - 1; ++i)
    {
      pipe (fd);

      // on écrit dans fd[1] , on garde 'in' de l'itération prendente 
      dupliquer_proc (in, fd [1], cmd + i);

      // pas besoin d'ecrire dans le tube ,le fils ecrira dedans.  
      close (fd [1]);

      // sauvgarder 'in', le fils qui va suivre va lire depuis 'in'.  
      in = fd [0];
    }

  //dernier pipe   
  if (in != 0)
    dup2 (in, 0);
	while ((wpid = wait(&status)) > 0);
  	executerCmdSimple(cmd[i].argv);//executer_simple command 
}

/***
	stop : fonction appeler après la capture du signal Ctrl+c (utiliser pour le cas des commandes sans paramètres comme cat)
***/
void stop(int a){
    running = 0;
}

/***
	get_entete_info : la fonction avec laquelle on récupère les caractéristiqus de l'entete
	entrées: une structure posix_header et un buffer
	sorties: void ( on récupère dans le buffer en entrée)
***/
void get_entete_info(posix_header *header,char buffer[BLOCKSIZE]){
    strncpy(header->name     , buffer      , 100);
    strncpy(header->mode     , &buffer[100], 8  );
    strncpy(header->uid      , &buffer[108], 8  );
    strncpy(header->gid      , &buffer[116], 8  );
    strncpy(header->size     , &buffer[124], 12 );
    strncpy(header->mtime    , &buffer[136], 12 );
    strncpy(header->chksum   , &buffer[148], 8  );
    header->typeflag         =  buffer[156]      ;
    strncpy(header->linkname , &buffer[157], 100);
    strncpy(header->magic    , &buffer[257], 6  );
    strncpy(header->version  , &buffer[263], 2  );
    strncpy(header->uname    , &buffer[265], 32 );
    strncpy(header->gname    , &buffer[297], 32 );
    strncpy(header->devmajor , &buffer[329], 8  );
    strncpy(header->devminor , &buffer[337], 8  );
    strncpy(header->prefix   , &buffer[345], 155);
    strncpy(header->junk     , &buffer[500], 12 );

}
/***
	cmp_name : une fonction utilitaire pour comparer deuc chaines de caractères (en ignorant le "/" à la fin d'un chemin , i.e "name" == "name/")
	entrées: deux chaines de caracatères
	sorties: entier
***/
int cmp_name(char str_cmp[100], char str_cmp2[100]){
    if(strcmp(str_cmp,str_cmp2) == 0){
        return 0;
    }  
    int i;
    if(str_cmp[strlen(str_cmp)-1] == '/'){
        i = strlen(str_cmp2);
        str_cmp2[i] = '/';
        str_cmp2[i+1] = '\0';
        if(strcmp(str_cmp,str_cmp2) == 0){
            str_cmp2[i] = '\0';
            return 0;
        }
        str_cmp2[i] = '\0';
    }
    if(str_cmp2[strlen(str_cmp2)-1] == '/'){
        i = strlen(str_cmp);
        str_cmp[i] = '/';
        str_cmp[i+1] = '\0';
        if(strcmp(str_cmp,str_cmp2) == 0){
            str_cmp[i] = '\0';
            return 0;
        }
        str_cmp[i] = '\0';
    }
    return 1;
}
/***
	open_tar_file_read : une fonction pour ouvrir un fichier tar en lecture
	entrées : path
	sorties : descripteur du fichier ouvert si c'est le cas , sinon un nombre négatif (la valeur de retour de syscall open())
***/
int open_tar_file_read(char ch[100]){
    char s[100];
    char str2[100];
    strcpy(s,ch);
    int i = strlen(strstr(s,".tar/"));
    strncpy(str2,ch,strlen(ch) - i + 4);
    str2[strlen(ch) - i + 4]='\0';
    return open(str2,O_RDONLY);
}
/***
	get_ch_absolu : une fonction pour calculer le chemin absolue à partir d'un chemin relatif donné
	entrées: chaine de caractères (chemin relative)
	sorties: void
***/
void get_ch_absolu(char *ch){
    
    //si ch commance par "/" alors on a entrer un chemin absolue
    if(ch[0]=='/'){
        return;//pas de changement 
    }
    // sinon "ch" est une chemin relatif, donc 
    char res[100];           /// "res" le chemin final serai ici tq :res = pwd + ch
                             
    strcpy(res,pwd_global);         /// res = pwd 
    int i = strlen(res);

    if(res[i-1]!='/'){       /// on verifie que "res" fini par "/" sinon je l'ajoute
        res[i]  ='/';
        res[i+1] = '\0';
    }
    
    strcat(res,ch);          /// res = res + pwd
    strcpy(ch,res);          /// puisque on a un passage par variable donc on met le resultat dans "ch"
}
/***
	remove_points : une fonction utilitaire pour enlever les points dans un chemin
	entrées : chaine de caractères 
	sorties : void
***/
void remove_points(char *ch){
    /// "ch" ne contient pas de "/." on change rien 
    if(strstr(ch,"/.") == NULL){
        return;
    }
    /// on assure que le chemin fini par "/"
    if(ch[strlen(ch)-1]!='/'){
        ch[strlen(ch)+1]='\0';
        ch[strlen(ch)]='/';
    }

    char str[100] , *ch1, ch2[100];
    strcpy(str,ch);                 /// sauvgarder la valeur de "ch" dans "str" 
    ch[0]='\0';                     /// vider "ch" 
    
    /// on va faire une analyse sur le chemin
    ch1 = strtok(str,"/");
    /// on va simplement suprimer tout les "./" de la chaines, et maitre le resultat dans "ch"
    while (1){
        strcpy(ch2,ch1);
        ch1 = strtok (NULL, "/");
        if(ch1 == NULL){break;}
        if(  strcmp(ch2,".") != 0 ){
            strcat(ch,"/");
            strcat(ch ,ch2);
        }
    }

    strcat(ch,"/");
    /// si  le chemin se termine par "." on ignore le "." 
    if( strcmp(ch2,".") != 0 ) strcat(ch ,ch2);
}
/***
	remove_2points : fonction utilitaire pour enlever les deux points ".." dans un chemin
	entrées : une chaine de caractères
	sorties : void
***/
void remove_2points(char *ch){
     /// "ch" ne contien pas de "/." on change rien 
    char copy[100];
    strcpy(copy,ch);
    int i = 0;
    while (strstr(copy,"..") != NULL && i++<10){

    /// on assure que le chemin fini par "/"
        if(copy[strlen(copy)-1]!='/'){
            copy[strlen(copy)+1]='\0';
            copy[strlen(copy)]='/';
        } 

        char str[100] , *ch1, ch2[100];
        strcpy(str,copy);
        ch[0]='\0';
        ch1 = strtok(str,"/");
    /// on va suprimer tout les ".." et la chainne qui precede, et maitre le resultat dans "ch"
        while (1){
            strcpy(ch2,ch1);
            ch1 = strtok (NULL, "/");
            if(ch1 == NULL){break;}
            if( strcmp(ch1,"..") != 0 && strcmp(ch2,"..") !=0 ){
                strcat(ch,"/");
                strcat(ch ,ch2);
            }else if( strcmp(ch1,"..") == 0 && strcmp(ch2,"..") ==0 ){
                strcat(ch,"/");
                strcat(ch ,ch2);
            }
        }
        strcat(ch,"/");
        if(strcmp(ch2,"..")!=0)strcat(ch ,ch2);
        strcpy(copy,ch);
    }
}


//ls
/***
	check_dir : cette fonction permet de verifier si le répértoire "dir_name" contient le fihier nommé "header_name"
	entrées : deux chaines de caractères 
	sorties : un entier ( 1 si le reépertoire "dir_name" contient un fichier nommé "header_name" , -1 sinon )
***/
int check_dir(char header_name[100],char dir_name[100]){
    char name[100],str[100];
    int i;
    if( strlen(dir_name) > 0 ) {
        if ( dir_name[strlen(dir_name)-1] != '/' ){strcat( dir_name,"/\0");}

    }else if(strlen(header_name) == 0){
        return -1;
    }   
    strncpy(name,header_name,strlen(dir_name));
    name[strlen(dir_name)]='\0';
    
    if(cmp_name(dir_name,name) == 0){
            i = strlen(dir_name);
            strncpy(str , &header_name[i] , strlen(header_name) - i);
            str [ strlen(header_name) - i ] = '\0';
            if(strstr(str,"/")==NULL){
                return 1;
            }
            strcpy(name,strstr(str,"/")); 
            name[strlen(strstr(str,"/"))] = '\0';    
            if( strlen(name) < 2 ){
                return 1;
            }
    }
    return -1;
}
/***
	get_dir : cette fonction permet de déplacer le curseur du fichier (tar) vers le répértoire "dir_name"
	entrées : descripteur , nom répértoire , une structure posix_header
	sorties : entier
***/
int get_dir(int fd,char dir_name[100], posix_header *header){
    int i , cpt = 0,nb_read=1;
    nb_read=read(fd,Buffer,BLOCKSIZE);cpt++;
    while (Buffer[0]!='\0' && Buffer[0]!=EOF && nb_read)
    {   
        get_entete_info(header, Buffer);        
        if(cmp_name(header->name,dir_name) == 0){
            return cpt;
        }else{
            free(header);
            header = malloc(sizeof(posix_header));                
            i = get_file_size(header);
            seek_next_entete(fd,header);cpt+=i;
            nb_read=read(fd,Buffer,BLOCKSIZE);cpt++;
        }
    }
    free(header);
    return 0;
}
/***
	permision : fonction pour modifier les droits sur un fichier (fichier régulier ou répértoire)
	entrées: une structure stat , une chaien de caratctère (path)
	sorties:void
***/
void permision(struct stat st,char *str){
    strcat(str, (S_ISDIR(st.st_mode) ) ? "d" : "-");
    strcat(str, (st.st_mode & S_IRUSR) ? "r" : "-");
    strcat(str, (st.st_mode & S_IWUSR) ? "w" : "-");
    strcat(str, (st.st_mode & S_IXUSR) ? "x" : "-");
    strcat(str, (st.st_mode & S_IRGRP) ? "r" : "-");
    strcat(str, (st.st_mode & S_IWGRP) ? "w" : "-");
    strcat(str, (st.st_mode & S_IXGRP) ? "x" : "-");
    strcat(str, (st.st_mode & S_IROTH) ? "r" : "-");
    strcat(str, (st.st_mode & S_IWOTH) ? "w" : "-");
    strcat(str, (st.st_mode & S_IXOTH) ? "x" : "-");
}

/***
	write_in_tar : fonction pour écrire le résultat de ls et ls -l 
	entrées:une structure posix_header , une option (option appartient à {0,1,2,3}
	sorties:void
	// 0 et 2 pour ls 
	// 1 et 3 pour ls -l
	//l'ajout de 2 et 3 est fait pour le cas oou on doit lister le contenu d'un fichier "*.tar" ex : "ls test.tar"
***/
void write_in_tar(posix_header *header,int option){
    if(header->name[0]=='#'){return;}
    char name[100];
    if( option == 2 || option == 3 ){
        strcpy(name,header->name);
        name[strlen(header->name)] = '\0';
    }else{
        
        int i = strlen(strstr(header->name,"/"));
        strncpy(name,&header->name[strlen(header->name) - i + 1],i-1);
        name[i-1] = '\0';
    }
	if(name[0] == '#'){return;} 
    char linkstr[100]="";
    if(option == 1 || option == 3 ){
        struct stat st;
        struct tm *dt;
    
        char str[100];int tail;
            str[0]='\0';
            sscanf(header->mode,"%o",&st.st_mode);
            permision(st,str);
            if(header->typeflag == '5'){str[0]='d';}
            if(header->typeflag == '1'){str[0]='l';strcpy(linkstr,header->linkname);}
            if(header->typeflag == '2'){str[0]='l';strcpy(linkstr,header->linkname);}
        write(STDOUT_FILENO,str,10);
        write(STDOUT_FILENO,"  0  ",5);

            sscanf(header->uid,"%o",&st.st_uid);
            sscanf(header->gid,"%o",&st.st_gid);
            struct passwd *pw = getpwuid(st.st_uid);
            struct group  *gr = getgrgid(st.st_gid);
            sprintf(str,"%s %s", pw->pw_name, gr->gr_name);
        write(STDOUT_FILENO,str,strlen(str));

            sscanf(header->size,"%o",&tail);
            sprintf(str,"%10d", tail);
        write(STDOUT_FILENO,str,strlen(str));
        write(STDOUT_FILENO,"  ",2);

            sscanf(header->mtime,"%lo",&st.st_mtime);
            dt=localtime(&st.st_mtime);
            strcpy(str,asctime(dt));
            str[strlen(str)-1]='\0';
        write(STDOUT_FILENO,str,strlen(str));
        write(STDOUT_FILENO,"  ",2);
    }
    write(STDOUT_FILENO,name,strlen(name));
    if(strlen(linkstr)>0){
        write(STDOUT_FILENO,"  ->  ",6);
        write(STDOUT_FILENO,linkstr,strlen(linkstr));
    }
    write(STDOUT_FILENO,"\n",1);
    return;
}
/***
	ls_in_tar : la fonction qui implémente la commande 'ls' dans les fichiers "*.tar"
	entrées: descripteur du fichier , dir_name , une option , une chaine de carctères
	sorties: void
***/
void ls_in_tar(int fd,char dir_name[100],int option,char ch[100]){
    posix_header *header = malloc(sizeof(posix_header));
    if( strlen(dir_name)== 0 ){option+=2;}
    if( (strlen(dir_name) > 0) && (get_dir(fd,dir_name,header) == 0)){
        write(STDERR_FILENO,"ls: ",5);
        write(STDERR_FILENO,ch,strlen(ch));
        write(STDERR_FILENO,": No such file or directory\n",29);
        return;
    }
    if( (strlen(dir_name) > 0) && (header->typeflag == '0')){
        write_in_tar(header,option);
        return;
    }
    int i;
    while ( 1 ){

        free( header );
        header = malloc(sizeof(posix_header));
        if(read(fd,Buffer,BLOCKSIZE)==0){return;}
        get_entete_info(header,Buffer);

        i = check_dir(header->name,dir_name);
        if( i == 1 ){//si le fichier/rep "header->name" est un fichier/rep du repertoire "dir_name"
            write_in_tar(header,option); // ecrire le nom du fichier avec l option
            seek_next_entete(fd,header); // seek to the next header
        }else
        if( i == -1){//sinon si le fichier/rep "header->name" n'est pas un fichier/rep du repertoire "dir_name"
            seek_next_entete(fd,header);  // seek to the next header    
        }
    }
}
/***
	write_out_tar : fonction qui va ecrire le résultat de ls et ls -l en dehors des tar files
	entrées:nom du fichier , nom du répértoire , option ( 0 pour ls et 1 pour ls -l )
	sorties:void
***/
void write_out_tar(char file_name[100],char dir_name[100],int option){
    char linkstr[100]="";
    if(option == 1){
        char str1[100];
        struct stat st;
        struct tm *dt;
        if(cmp_name(file_name,dir_name)!=0) {
            if(dir_name[strlen(dir_name)-1]!='/'){strcat(dir_name,"/");}
            strcpy(str1,dir_name);
            strcat(str1,file_name);
        }else{
            strcpy(str1,dir_name);
        }

        if(lstat(str1,&st) > -1){///readlink
            if(S_ISLNK(st.st_mode)){
                readlink(str1,linkstr,100);
            }   
        }else{
            //perror(str1);
            return;
        }
       
        char str[100];
            str[0]='\0';
            permision(st,str);
        if(strlen(linkstr)>0){
            str[0]='l';
        }
        write(STDOUT_FILENO,str,10);
        write(STDOUT_FILENO," ",1);

            sprintf(str,"%2lu",st.st_nlink);
        write(STDOUT_FILENO,str,2);
        write(STDOUT_FILENO,"  ",2);

            struct passwd *pw = getpwuid(st.st_uid);
            struct group  *gr = getgrgid(st.st_gid);
            sprintf(str,"%s %s", pw->pw_name, gr->gr_name);
        write(STDOUT_FILENO,str,strlen(str));

            sprintf(str,"%10ld",st.st_size);
        write(STDOUT_FILENO,str,strlen(str));
        write(STDOUT_FILENO,"  ",2);

            dt=localtime(&st.st_mtime);
            strcpy(str,asctime(dt));
            str[strlen(str)-1]='\0';
        write(STDOUT_FILENO,str,strlen(str));
        write(STDOUT_FILENO,"  ",2);

    }        
    write(STDOUT_FILENO,file_name,strlen(file_name));
    if(strlen(linkstr)>0){
        write(STDOUT_FILENO,"  ->  ",6);
        write(STDOUT_FILENO,linkstr,strlen(linkstr));
    }
    write(STDOUT_FILENO,"\n",1);
    
}
/***
	ls_out_tar : la fonction qui implémente le fonctionnement de ls en dehors des fichiers *.tar
	entrées: dir_name , chaine de caractères , une option 
	sorties:void
***/
void ls_out_tar(char dir_name[100],char ch[100],int option){
    struct stat st;
    char str[100];
    if(stat(dir_name, &st)==-1){//tester si le fichier/repertoir existe 
        write(STDERR_FILENO,"ls: ",5);
        write(STDERR_FILENO,ch,strlen(ch));
        write(STDERR_FILENO,": No such file or directory\n",29);
        return;
    }
    if(S_ISDIR(st.st_mode)){//test si "dir_name" est un repertoir
        DIR *dirp = opendir(dir_name);//ouvrir le repertoir 
        struct dirent *entry = malloc(sizeof(entry));
        while((entry=readdir(dirp))){//ecrire tous les fichiers et repertoirs de "dir_name"
            if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){ //ignorer '.' et '..' 
                write_out_tar(entry->d_name,dir_name,option);
            }
        }
        closedir(dirp);
    }else {// si dir_name n'est pas un repertoir
        
        write_out_tar(dir_name,dir_name,option);
    }
    
}
/***
	ls : la fonction qui implémente le fonctionnement de la commande ls
	entrées:path et une option (pour ls et ls -l)
	sorties:void
***/
void ls(char ch[100],int option){
    int fd;int i;
    struct stat sb,sp;    
    char str[100],str2[100];

    strcpy(str,ch);         /// pour ne pas changer le chemin en entrée, copie "ch" dans "str"
    get_ch_absolu(str);     /// construire le chemin absolu du chemin donne
    remove_points (str);    /// suprimer les "."
    remove_2points(str);    /// suprimer les ".."

    /// dans le cas ou le chemin inclue un ficher tar
    if(strstr(str,".tar")!=NULL  ){
        if(strlen(strstr(str,".tar")) <= 5){// le cas ou "ls" est execute sur ficher "tar"

            if(strlen(strstr(str,".tar")) == 5){ /// on suprime "/" si il existe a la fin  
                str[strlen(str)-1] = '\0';
            }
            /// ouvrir le fichier
            fd = open(str,O_RDONLY);
            
            if(fstat(fd, &sb)==-1){ /// messsage d erreur
                write(STDERR_FILENO,"ls: ",5);
                write(STDERR_FILENO,ch,strlen(ch));
                write(STDERR_FILENO,": No such file or directory\n",29);
                return ;
            }

            ls_in_tar(fd,"",option,ch);
            close (fd);
            return;
        }

            //ls d'un repertoire dans un tar
        fd = open_tar_file_read(str);
        if(fstat(fd, &sb)==-1){/// message d erreur
            write(STDERR_FILENO,"ls: ",5);
            write(STDERR_FILENO,ch,strlen(ch));
            write(STDERR_FILENO,": No such file or directory\n",29);
            return ;
        }

        /// ajuster le chemin
        i = strlen(strstr(str,".tar/"));
        strncpy(str2 , &str[strlen(str) - i + 5] , i);
        str2[i]='\0';
        
        /// 
        ls_in_tar(fd,str2,option,ch);
        close(fd);
        return;
    }

    //le cas ou le chemin n'inclue pas des tar 
    ls_out_tar(str,ch,option);
    return;
}
//cat
/***
	get_file : une fonction qui déplace le curseur dans le fichier tar vers le fcihier nommé file_name
	entrées: descripteur , nom de fichier , une structure posix_header
	sorties:un entier
***/
int get_file(int fd,char file_name[100], posix_header *header,char ch[100]){
    int i , cpt = 0;
    read(fd,Buffer,BLOCKSIZE);cpt++;
    while (Buffer[0]!='\0' && Buffer[0]!=EOF)
    {   
        get_entete_info(header, Buffer);        
        if(cmp_name(header->name,file_name) == 0){
            if(header->typeflag == '5'){
                write(STDERR_FILENO,"cat: ",5);
                write(STDERR_FILENO,file_name,strlen(file_name));
                write(STDERR_FILENO,": Is a directory\n",18);
                return 0;
            }
            return cpt;
        }else{
            free(header);
            header = malloc(sizeof(posix_header));                
            i = get_file_size(header);
            seek_next_entete(fd,header);cpt+=i;
            read(fd,Buffer,BLOCKSIZE);cpt++;
        }
    }
    free(header);
    return 0;
}

/***
	cat_in_tar : la fonction qui permet de faire un cat dans un tar
	entrées:descripteur , nom du fichier 
	sorties:void
***/
//Cette procedure est responsable de la comande "cat" dans les tar
void cat_in_tar(int fd,char file_name[100],char ch[100]){
    posix_header *header = malloc(sizeof(posix_header));
    int cpt = get_file(fd,file_name,header,ch);
    if(cpt != 0){
        int i = get_file_size(header);
        cpt += i;
        for(i;i>0;i--){
            read(fd,Buffer,BLOCKSIZE);
            write(STDOUT_FILENO,Buffer,BLOCKSIZE);
        }
        free(header);
    }else{ 
        write(STDERR_FILENO,"cat: ",5);
        write(STDERR_FILENO,ch,strlen(ch));
        write(STDERR_FILENO,": No such file or directory\n",29);
    }
}

/***
	cat_out_tar : fonction qui implémente le fonctionnement de cat en dehors des tar's
	entrées: nom du fichier ,path
	sorties:void 
***/
void cat_out_tar(char file_name[100],char ch[100]){
    int i;
    struct stat sb;    
    int fd = open(file_name,O_RDONLY);

    if(fstat(fd, &sb)==-1){
            write(STDERR_FILENO,"cat: ",5);
            write(STDERR_FILENO,ch,strlen(ch));
            write(STDERR_FILENO,": No such file or directory\n",29);
            return ;
    }     
    while ((i=read(fd, Buffer,sizeof(Buffer)))>0)
    {
        write(STDOUT_FILENO,Buffer,i);
    }
    close(fd);
    return ;
}


/***
	cat :la fonction qui implémente le fonctionnement de la commande cat 
	entrées: un chemin (path)
	sorties: void
***/
void cat(char ch[100]){
    
    ///dans le cas ou "cat" et executer sans arguments 
    if(strlen(ch)==0){
        int cpt;
        int i=1;
        while (running && i){
            i = read(STDIN_FILENO,Buffer,BLOCKSIZE);
            write(STDOUT_FILENO,Buffer,i);
        }
        running = 1;
        return;
    }

    int fd;int i;
    struct stat sb,sp;    
    char str[100],str2[100];

    strcpy(str,ch);        
    get_ch_absolu(str);    /// get le chemin absolue
    remove_points(str);    
    remove_2points(str);
    
    /// le cas ou le chemin n'inclue pas de tar
    if( strstr(str,".tar")==NULL){
            cat_out_tar(str,ch);
            return;
    
    /// le cas ou le chemin pointe sur un fichier tar
    }else if ( strlen (strstr(str,".tar")) <= 5 ){
        write(STDERR_FILENO,"cat: ",5);
        write(STDERR_FILENO,ch,strlen(ch));
        write(STDERR_FILENO,": is directory\n",15);
        return ;
    }

    /// le cas ou le chemin inclue des tar 
    fd = open_tar_file_read(str);
    if(fstat(fd, &sb)==-1){///message d error
        write(STDERR_FILENO,"cat: ",5);
        write(STDERR_FILENO,ch,strlen(ch));
        write(STDERR_FILENO,": No such file or directory\n",29);
        return ;
    }

    /// prendere le reste du chemin
    /// ex : str  == "/aaaa/bbbbb/cccc.tar/ddddd/eeeee"
    ///      str2 == "ddddd/eeeee"
    i = strlen(strstr(str,".tar/"));
    strncpy(str2 , &str[strlen(str) - i + 5] , i);
    str2[i]='\0';
    cat_in_tar(fd,str2,ch);

    close(fd);
    
}
//redirections ">"  ">>"  "<"  "2>"  "2>>"  "2>&1"  "2>>&1"

/***
	search : une fonction pour rechercher dans un tar 
	entrées: path
	sorties:entier
***/
int search(char ch[100]){
    posix_header *header;
    int fd = open_tar_file_read(ch);
    int i;
    if( fd <= 0 ){
        write(1,"zfi : No such file \n",strlen("zfi : No such file \n"));
        return -1;
    }
    char str[100];
    i = strlen(strstr(ch,".tar/"));
    strncpy(str , &ch[strlen(ch) - i + 5] , i);

    while (i = read(fd,Buffer,BLOCKSIZE))
    {
        header = malloc(sizeof(posix_header));
        get_entete_info( header , Buffer );
        if(cmp_name( header->name , str )==0){
            free(header);
            return 1;
        }
        seek_next_entete(fd,header);
        free(header);
    }
    close(fd);
    return 0;
}

/***
	end_redct : end redirection
	entrées: path , descripteur , fichier de sortie , type de redirection
	sorties:void
***/
//int type : numéro de redirection ,      'type == 1' pour '>'    et '>>'   , 'type == 2' pour '2>' et '2>>' , 
//                                        'type == 3' pour '2>&1' et '2>>&1', 'type == 4' pour '<'   
void end_redct(char ch[100],int fd,char *out_file,int type){

    close(fd);
    if(type == 1 ){
        dup2(std_out_copy,STDOUT_FILENO);
    }else if ( type == 2){
        dup2(std_error_copy,STDERR_FILENO);
    }else if ( type == 3 ){
        dup2(std_out_copy  ,STDOUT_FILENO);
        dup2(std_error_copy,STDERR_FILENO);
    }else if( type == 4){
        dup2(std_in_copy  ,STDIN_FILENO);
    }
    
    if(strlen(out_file)==0){return;}

    //////here
    char str[100];
    strcpy(str,ch);
    char ch_in_tar[100];
    strcpy(ch_in_tar,str);
    ch_in_tar[strlen(ch_in_tar)-strlen(recupere_nom(ch_in_tar))]='\0';
    cp(out_file,ch_in_tar);
    out_file="";

}
/***
	redercet_stdout : redirection de la sortie standard
	entrées: nom du fichier de sortie , fichier de sortie , type de redirection
	sorties: entier (std_out_copy) pour sauvgarder dup(1)
	//char out_file : si c'est un tar on va creer un fichier dans /tmp ecrire dedans ,a la fin on recopier le resultat ou on veut
	//int type : numéro de redirection , veux 'type == 1' pour '>'    , 'type == 2' pour '>>'    , 'type == 3' pour '2>' ,
	//                                         'type == 4' pour '2>>' , 'type == 5 pour '2>&1   et 'type == 6' pour '2>>&1'
***/
int redercet_stdout(char ch[100],char *out_file,int type){
    char str[100],str2[100],str3[100];
    struct stat sb;
    int fd,res,i;
            

    strcpy(str,ch);
    get_ch_absolu(str);
    remove_points(str);
    remove_2points(str);


    if(strstr(str,".tar")!=NULL){
        
        res = search(str);
        if( res == -1 ){
            write(STDERR_FILENO,ch,strlen(ch));
            write(STDERR_FILENO,": No such file or directory\n",28);
            return 0;
        }
        
        if( res == 0){
            
                int alpha = strlen(strstr(str,".tar/"))-5;
                char ch_in_tar[100];
                strcpy(ch_in_tar,str);
                ch_in_tar[strlen(ch_in_tar)-strlen(recupere_nom(ch_in_tar))]='\0';

                if((strlen(strstr(ch_in_tar,".tar"))>5) && search(ch_in_tar) != 1){
                    write(STDERR_FILENO,ch,strlen(ch));
                    write(STDERR_FILENO,": No such file or directory\n",28);
                    return 0;
                }
                
        }

        strcpy(str3,"/tmp/");
        
        if (res == 1){
            if( type == 2 || type == 4 || type == 6){
                cp(ch,str3);
            }
            rm(ch);
        }
        
        strcat(str3,recupere_nom(str));
        strcpy(out_file,str3);
    
    }else{
            
        strcpy(str3,str);
        out_file[0]='\0';
    }

         
    if( type == 1 || type == 3 || type == 5){
        fd = open(str3,O_TRUNC );
        if( fd > 0 ){ close(fd); }
        fd = open(str3,O_WRONLY | O_CREAT,0664);
    }else{
        fd = open(str3,O_RDWR | O_CREAT,0664);
        lseek(fd,0,SEEK_END);
    }

    if(fd <= 0 ){ 
        perror(ch); 
        return 0; 
    }
    

    if(type == 1 || type == 2){
        dup2( fd , STDOUT_FILENO );
        return fd;
    }

    if( type == 3 || type == 4 ){
        dup2( fd , STDERR_FILENO );
        return fd;
    }

    if( type == 5 || type == 6){
        dup2( fd , STDOUT_FILENO );
        dup2( fd , STDERR_FILENO );
        return fd;
    }
}

/***
	redercet_stdin : redirection de l'entrée standard
	entrées: path
	sorties:entier
***/
int redercet_stdin(char ch[100]){
    char str[100],str2[100],str3[100];
    struct stat sb;
    int fd,res,i;
 
    strcpy(str,ch);
    get_ch_absolu(str);
    remove_points(str);
    remove_2points(str);

    ///dans le cas ou le chemin inclue des tar
    if(strstr(str,".tar")!=NULL){
             
        res = search(str);
        if( res == -1 || res == 0){
            write(STDERR_FILENO,ch,strlen(ch));
            write(STDERR_FILENO,": No such file or directory\n",28);
            return 0;
        }

        strcpy(str3,"/tmp/");
        i = strlen(strstr(str,".tar/"));
        strncpy(str2 , &str[strlen(str) - i + 5] , i);
        
        cp(str,str3);

        strcat(str3,recupere_nom(ch));

    /// si le chemin n'inclue pas des tar
    }else{
        strcpy(str3,str);
    }

   
    fd = open(str3,O_RDONLY);

    if(fd <= 0 ){ 
        write(STDERR_FILENO,ch,strlen(ch));
        write(STDERR_FILENO,": No such file or directory\n",28);
        return 0; 
    }
    
    dup2( fd , STDIN_FILENO );
    return fd;
}

/***
	redirect_res : fonction qui gère les entrées et sorties 
	entrées: le chemin du fichier à écrire , chemin du fichier ou ecrire , type de redirection 
	sorties:entier
***/
int redirect_res(char ch[100],char *out_file,char type[10]){
    if ( strcmp( type , ">" ) == 0 ){
        return redercet_stdout( ch , out_file , 1);
    }
    if ( strcmp( type , ">>" ) == 0 ){
        return redercet_stdout( ch , out_file , 2);
    }
    if ( strcmp( type , "2>" ) == 0 ){
        return redercet_stdout( ch , out_file , 3);
    }
    if ( strcmp( type , "2>>" ) == 0 ){
        return redercet_stdout( ch , out_file , 4);
    }
    if ( strcmp( type , "2>&1" ) == 0 ){
        return redercet_stdout( ch , out_file , 5);
    }
    if ( strcmp( type , "2>>&1" ) == 0 ){
        return redercet_stdout( ch , out_file , 6);
    }
    if ( strcmp( type , "<" ) == 0 ){
    	out_file = "";
        return redercet_stdin( ch );
    }

}

/***
	end_redirect : cette fonction gère la fin des redirections
	entrées:path , descripteur , fichier de sortie , type
	sorties:void
***/       
void end_redirect(char ch[100],int fd ,char *out_file,char type[10]){
    if ( strcmp( type , ">" ) == 0 || strcmp( type , ">>" ) == 0 ){
        end_redct( ch ,fd , out_file , 1);
        return;
    }
    if ( strcmp( type , "2>" ) == 0 || strcmp( type , "2>>" ) == 0 ){
        end_redct( ch ,fd , out_file , 2);
        return;
    }
    if ( strcmp( type , "2>&1" ) == 0 || strcmp( type , "2>>&1" ) == 0){
        end_redct( ch ,fd , out_file , 3);
        return;
    }
    if ( strcmp( type , "<" ) == 0 ){
        end_redct( ch ,fd , "" , 4);
        return;
    }
}
/***
	get_type : fonction utilitaire pour le parsing des commandes avec redirections
	entrées:symbole de redirection et un caractère 
	sorties: un pointeur vers une chaine de cractères
***/
char* get_type (char symb[8],char before ){

    if (strstr(symb,">>&1") != NULL){
        if(before == '2'){
            return "2>>&1";
        }else{
            return ">>&1";
        }
    }else if(strstr(symb,">>") != NULL){
        if(before == '2'){
            return "2>>";
        }else{
            return ">>";
        }
    }else{
        if(before == '2'){
            return "2>";
        }else{
            return ">";
        }
    }
}
/***
	parsing_red : fonction pour le parsing des redirections
	entrées:path , chaine de caractères , une liste des arguments de la commande comportant des redirections 
	sorties: int
***/
int parssing_red(char chainne[100],char *listArgsRed[100]){
    int nb_red = 0;    /// le nombre de redirection
    int check_symb = 0;/// pour voir quelle et le type de redirection
    
    if(strstr(chainne,">") == NULL && strstr(chainne,"<") == NULL){
        return nb_red;
    }

    int nb_args = 0;
    char str[100],*res;
    
    strcpy(str,chainne);
    res = strtok(str,"<> ");
    
    while (res != NULL)
    {   
        check_symb += strlen(res);
        if(strcmp(res,"2")!=0){
            listArgsRed[nb_args] = res;
            nb_args++;
        }
        while (chainne[check_symb] == ' ')
        {
            check_symb++;
        }
        
        if(chainne [ check_symb ] == '<'){
            listArgsRed[nb_args] = "<";
            nb_args++;
            nb_red++;
            check_symb++;

        }else if(chainne [ check_symb ] == '>' ){              
            char symb[8];
            char before = chainne [check_symb-1];
            strncpy(symb,&chainne[check_symb],4);
            listArgsRed[nb_args] = get_type(symb,before);
            nb_args++;
            nb_red++;
            check_symb++;
        }
        while (chainne[check_symb] == ' ')
        {
            check_symb++;
        }
        res = strtok(NULL," <>");
    }
    
    return nb_red;
}
/***
	strcmp_red : fonction utilitaire pour comparaison de chaines pour les redirections
	entrées:chaine
	sorties:entier
***/
int strcmp_red(char *str){
    if(strcmp(str,"<") == 0 || strcmp(str,">") == 0 || strcmp(str,"2>") == 0){
        return 0;
    }
    if( strcmp(str,">>") == 0 || strcmp(str,"2>>") == 0 || strcmp(str,"2>&1") == 0 || strcmp(str,"2>>&1") == 0){
        return 0;
    }
    return 1;
}
/***
	start_all_redirect : fonction pour lancer toutes les redirections pour les commandes avec redirection
	entrées:path , descripteurs de fichiers ,fichiers pour redirections , liste arguments commande redirection , nombre de redirections
	sorties:int 
***/
int start_all_redirect(int *fd1,int *fd2,int *fd3,char *out_file1, char *out_file2 ,char *out_file3, char *listArgsRed[100],int *nb_red){ 
    int i = 0;
    char copy1[100],copy2[10];
    if(*nb_red > 0){
        strcpy(copy1,listArgsRed[i]);
        while ( listArgsRed[i+1] !=NULL && strcmp_red(copy1) )  {i++;strcpy(copy1,listArgsRed[i]);}
        if( listArgsRed[i+1] != NULL ){
            strcpy(copy2,copy1);
            strcpy(copy1,listArgsRed[i+1]);
            if((*fd1 = redirect_res(copy1 ,out_file1 , copy2))<=0){
                write(STDERR_FILENO,copy1,strlen(copy1)); ///// a ajouter
                write(STDERR_FILENO,":  No such file or directory\n",29);//// a ajouter
                *nb_red = 0;//// a ajouter
                return 0;
            }
        }
    }
    i++;
    if(*nb_red > 1){
        strcpy(copy1,listArgsRed[i]);
        while ( listArgsRed[i+1] !=NULL && strcmp_red(copy1) )  {i++;strcpy(copy1,listArgsRed[i]);
        if( listArgsRed[i+1] != NULL ){
            strcpy(copy2,copy1);
            strcpy(copy1,listArgsRed[i+1]);
            if((*fd2 = redirect_res(copy1 ,out_file2 , copy2))<=0){
                write(STDERR_FILENO,copy1,strlen(copy1));//// a ajouter
                write(STDERR_FILENO,":  No such file or directory\n",29);/// a ajouter
                *nb_red = 1;  /// a ajouter
                return 0;
            }
        }
    }      
    i++;
    if(*nb_red > 2){
        strcpy(copy1,listArgsRed[i]);
        while ( listArgsRed[i+1] !=NULL && strcmp_red(copy1) )  {i++;strcpy(copy1,listArgsRed[i]);}
        if( listArgsRed[i+1] != NULL ){
            strcpy(copy2,copy1);
            strcpy(copy1,listArgsRed[i+1]);
            if((*fd3 = redirect_res(copy1 ,out_file3 , copy2))<=0){
                write(STDERR_FILENO,copy1,strlen(copy1));
                write(STDERR_FILENO,":  No such file or directory\n",29);
                *nb_red = 2;
                return 0;
      }
    }
    }      
    return 1;
	}
}
/***
	end_all_redirect : fonction pour terminer les redirections 
	entrées:descripteurs , fichiers de redirection , liste des arguments de la commande en question , nombre de redirections
	sorties:void
***/
void end_all_redirect(int *fd1, int *fd2, int *fd3, char *out_file1,char *out_file2,char *out_file3, char *listArgsRed[100], int nb_red){
    int i = 0;
    char copy1[100],copy2[10];
    if(nb_red > 0){
        strcpy(copy1,listArgsRed[i]);
        while ( listArgsRed[i+1] !=NULL && strcmp_red(copy1) )  {i++;strcpy(copy1,listArgsRed[i]);}
        if( listArgsRed[i+1] != NULL ){
            strcpy(copy2,copy1);
            strcpy(copy1,listArgsRed[i+1]);
            end_redirect(copy1,*fd1,out_file1,copy2);
            *fd1 = 0;
        }
    }      
    
    i++;
    if(nb_red > 1){
        strcpy(copy1,listArgsRed[i]);
        while ( listArgsRed[i+1] !=NULL && strcmp_red(copy1) )  {i++;strcpy(copy1,listArgsRed[i]);}

        if( listArgsRed[i+1] != NULL ){
            strcpy(copy2,copy1);
            strcpy(copy1,listArgsRed[i+1]);
            end_redirect(copy1,*fd2,out_file2,copy2);
            *fd2 = 0;
        }
    }      
    
    i++;
    if(nb_red > 2){
        strcpy(copy1,listArgsRed[i]);

        while ( listArgsRed[i+1] !=NULL && strcmp_red(copy1) )  {i++;strcpy(copy1,listArgsRed[i]);}

        if( listArgsRed[i+1] != NULL ){
            strcpy(copy2,copy1);
            strcpy(copy1,listArgsRed[i+1]);
            end_redirect(copy1,*fd3,out_file3,copy2);
            *fd3 = 0;
        }
    }      
}
//cp et mv 
/***
	recupere_nom : fonction utilitaire pour la manipulation des chemins
***/
char *recupere_nom(char *path)
{
    char *pend,str[100];
    strcpy(str,path);
    if(str[strlen(str)-1]=='/'){
        str[strlen(str)-1] = '\0';
    }
    if((pend=strrchr(str, '/'))!=NULL){
        return pend +1 ;
    }else{
	    return path;
    }
}
/***
	recherche_tar :une extension d'une fonction qui fait de la recherche dans un tarball
	entrées : descripteur fichier ,nom fichier à rechercher ,booléen , entete lu si c'est le cas , taille du fichier si trouv=1
	sories : void
***/
void recherche_tar(int fd, char nomfic[100],int *trouv, int *entete,int *size_file){

    int size=0,EnteteAlire=0;
    
    struct posix_header *st = malloc(sizeof(struct posix_header));
    
    while( read(fd,Buffer,BLOCKSIZE)){
		
        get_entete_info(st, Buffer);
		// on teste si on est pas arrivé à la fin 
        if((st->name)[0] == '\0'){
                
                lseek(fd,-BLOCKSIZE,SEEK_CUR);
                *entete = EnteteAlire; //sauvegarde derniere entete destination        
                *trouv=0;//// on a rien trouver
                free(st);
                return;
		}

        
		if (strcmp(st->name, nomfic)==0){

            sscanf(st->size,"%o",&size);
            
            *size_file = size;        
			*entete=EnteteAlire;
			*trouv= 1;//// on a trouver unn fichoer ab=vec le meme nom
            free(st);
            return;
		}
       
		// on récupère la taille du fichier 
		
        size = get_file_size(st);
		if(size==0){
			EnteteAlire += size;// >>BLOCKBITS = /512
		}
		else{
			EnteteAlire += size;//((size + BLOCKSIZE  ) >> BLOCKBITS)+1; 
		}

		lseek(fd,size*BLOCKSIZE,SEEK_CUR);
	} 
}
/***
	open_tar_file_rdwr : fonction pour ouvrir un tarball en lecture et ecriture 
	entrées : nom du fichier
	sorties : descripteur du fichier
***/
int open_tar_file_rdwr(char ch[100]){
    char s[100];
    char str2[100];
    strcpy(s,ch);
    int i = strlen(strstr(s,".tar/"));
    strncpy(str2,ch,strlen(ch) - i + 4);
    str2[strlen(ch) - i + 4]='\0';
    return open(str2,O_RDWR);
}
/***
 checkIfValide : fonction utilitaire pour la manipulation des chemins
***/
int checkIfValide(char source[100], char destination[100]){
    //1-on verifie qu'on a recu deux arguments 
   

    if(source == NULL){
        write(STDERR_FILENO,"cp : missing file operand\n",26);
        return 0;
    }else if (destination == NULL){
        write(STDERR_FILENO,"cp : missing destination file operand after '",45);
        write(STDERR_FILENO,source,strlen(source));
        write(STDERR_FILENO,"'\n",2);
        return 0;
    }

    //2- on vérifie si la source= destination le fichier ne sera pas copier comme  on aura juste une duplication. 
    char str[100];
    if(strlen(destination) <= strlen(source)){

        strncpy(str,source,strlen(destination));

        if(strcmp(str,destination) == 0){
            write(STDERR_FILENO,"cp : ",5);
            write(STDERR_FILENO,source,strlen(source));
            write(STDERR_FILENO,"' and '",7);
            write(STDERR_FILENO,str,strlen(str));
            write(STDERR_FILENO,"' are in the same file\n",23);
            
            return 0;
        }
    }

    return 1;

}

/***
	posix_to_buffer : fonction pour ecrire les carcatéristiques d'une entête posix_header 
***/
int posix_to_buffer(char ch_in_tar[100],int fdS, int fdD){

    posix_header *header = malloc(sizeof( posix_header));
    //récupération des caractéristiques du fichier source
    struct stat  statbuf ;
    struct tm *dt;
    char str[100];

    if ( fstat(fdS, &statbuf) < 0) {
        write(1,"zfi : No such file \n",strlen("zfi : No such file \n"));
        return 0;
    }
    
    
    strcpy(header -> name, ch_in_tar);
    header -> typeflag = '0';
    sprintf(header -> size ,"%lo" ,statbuf.st_size);
    sprintf(header -> mode ,"%o" ,statbuf.st_mode);
    sprintf(header -> mtime,"%lo",statbuf.st_mtime);
    sprintf(header -> uid  ,"%o" ,statbuf.st_uid);
    sprintf(header -> gid  ,"%o" ,statbuf.st_gid);
    strcpy(header -> magic, TMAGIC);
    strcpy(header -> version, TVERSION);
    set_checksum(header);

    //écriture de l'entete
    write(fdD, header, BLOCKSIZE) ;

    return 1;
}
/***
	cp_destination_tar : une partie de la fonction cp (qui traite la copie dans le cas ou la destination est un tarball)
	entrées: source et destination 
	sorties: int 
***/
int cp_destination_tar(char source[100], char destination[100]) {

    char sourceAbs[100],destinationAbs[100];

    /// construire le chemin absolue de source
    strcpy(sourceAbs,source);
    get_ch_absolu(sourceAbs);
    remove_points(sourceAbs);
    remove_2points(sourceAbs);

    /// construire le chemin absolue de destination
    strcpy(destinationAbs,destination);
    get_ch_absolu(destinationAbs);
    remove_points(destinationAbs);
    remove_2points(destinationAbs);
    if(destinationAbs[strlen(destinationAbs)-1]!='/'){
        int i = strlen(destinationAbs);
        destinationAbs [ i ] = '/';
        destinationAbs [ i + 1] = '\0';
    }



    //2-  on essaie d'ouvrir le tar de destination pour savoir s'il existe 
    int fdS, fdD, nbtotal = 0;

    fdS = open(sourceAbs, O_RDONLY);
    if (fdS < 0) {
        write(STDERR_FILENO,"cp : cannot stat '",5);
        write(STDERR_FILENO,source,strlen(source));
        write(STDERR_FILENO,"': No such file or directory\n",29);
        return 0;
    }

    fdD = open_tar_file_rdwr(destinationAbs);
    if (fdD < 0) {
        // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
        write(STDERR_FILENO,"cp: cannot create regular file '",32);
        write(STDERR_FILENO,destination,strlen(destination));
        write(STDERR_FILENO,"': Permission denied\n",21);
        return 0;
    }
    //  dans ce cas le tarball est bien ouvert <=> il existe et le chemin est correcte  

    //3- on récupere le nom du fichier dans la source ainsi ou il doit etre mis

    int i = strlen(strstr(destinationAbs,".tar/"));

	char ch_in_tar[100]="";
	strncpy(ch_in_tar,&destinationAbs[strlen(destinationAbs)-i+5],i);

    //2- on récupere le nom du fichier dans la source
   

    // maintenant dans nom fic, on a le chemin apartir du tar vers le fichier a copier 

    //4- maintenant on effectue une recherche du fichier dans le tarball pour savoir si il nexiste pas   
    int trouv = 0, entete = 0, size1 = 0;
    recherche_tar(fdD, ch_in_tar, & trouv, & entete, & size1);

    if (trouv == 0 && strlen(ch_in_tar)>1) {
        close(fdD);
        write(STDERR_FILENO,"cp: cannot create regular file '",32);
        write(STDERR_FILENO,destination,strlen(destination));
        write(STDERR_FILENO,"': Permission denied\n",21);
        return 0;
    }
    
    strncpy(ch_in_tar,&destinationAbs[strlen(destinationAbs)-i+5],i);
    strcat(ch_in_tar, recupere_nom(sourceAbs));

    trouv = 0; entete = 0; size1 = 0; 
    lseek(fdD,0,SEEK_SET);
    recherche_tar(fdD, ch_in_tar, & trouv, & entete, & size1);
    //write(1,ch_in_tar,strlen(ch_in_tar));
    if(trouv == 1){
        char str[100];
        strcpy(str,destinationAbs);
        strcat(str,recupere_nom(sourceAbs));
        close(fdD);
        
        rm(str);
        fdD = open_tar_file_rdwr(destinationAbs);
        trouv = 0; entete = 0; size1 = 0; 
        recherche_tar(fdD, ch_in_tar, & trouv, & entete, & size1);
        if(trouv == 1){
            return 0;
        }
    }
    
    
    if(posix_to_buffer(ch_in_tar, fdS, fdD ) == 0){
        
        return 0;
    }

    
    int nb_read;
    while (nb_read = read(fdS, Buffer, BLOCKSIZE)) {
        write(fdD, Buffer, nb_read);
    }
    close(fdS);
    close(fdD);
    return 1;
}

/***
	cp_source_destination_tar : une partie de la fonction cp (qui traite la copie dans le cas ou la source et la destination est un tarball)
	entrées: source et destination 
	sorties: int 
***/

int cp_source_destination_tar(char source[100], char destination[100]) {

    char fake_distination[100]="/tmp";

    if(cp_source_tar( source, fake_distination)){

        strcat(fake_distination,"/");                   ////  "/tmp/"
        strcat(fake_distination,recupere_nom(source));  ////  "/tmp/nom_fichier"

        if(cp_destination_tar( fake_distination, destination) == 0){
            remove(fake_distination);
            return 0;
        }
        remove(fake_distination);
        return 1;
    }
    return 0;
}

int cp_normal(char source[100], char destination[100]){
    
    char sourceAbs[100],destinationAbs[100];

    /// construire le chemin absolue de source
    strcpy(sourceAbs,source);
    get_ch_absolu(sourceAbs);
    remove_points(sourceAbs);
    remove_2points(sourceAbs);

    /// construire le chemin absolue de destination
    strcpy(destinationAbs,destination);
    get_ch_absolu(destinationAbs);
    remove_points(destinationAbs);
    remove_2points(destinationAbs);
    if(destinationAbs[strlen(destinationAbs)-1]!='/'){
        int i = strlen(destinationAbs);
        destinationAbs [ i ] = '/';
        destinationAbs [ i + 1] = '\0';
    }


    if(checkIfValide(sourceAbs,destinationAbs) == 0){
        return 0;
    }
    

    int fd1,fd2;
    fd1 = open(sourceAbs, O_RDONLY);
    if (fd1 < 0) {
        write(STDERR_FILENO,"cp : cannot stat '",5);
        write(STDERR_FILENO,source,strlen(source));
        write(STDERR_FILENO,"': No such file or directory\n",29);
        return 0;
    }

    //4- Si le fichier source est bien ouvert, on ouvre le fichier destination avec les modes d'ecriture et de création 

    char dest_file[100];
    
    strcpy(dest_file,destinationAbs);
    strcat(dest_file,recupere_nom(sourceAbs));

    if((strcmp(sourceAbs,dest_file)==0) )
    {
        write(STDERR_FILENO,"cp : '",5);
        write(STDERR_FILENO,source,strlen(source));
        write(STDERR_FILENO,"' and '",7);
        write(STDERR_FILENO,destination,strlen(destination));
        write(STDERR_FILENO,"' are the same file\n",20);
        return 0;
    }

    fd2 = open(dest_file, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd2 < 0) {
        write(STDERR_FILENO,"cp: cannot create regular file '",32);
        write(STDERR_FILENO,destination,strlen(destination));
        write(STDERR_FILENO,"': Permission denied\n",21);
        return 0;
    }

    //5- Si le fichier destination est bien ouvert, tout est bien passé, on commence le transfert des données de source vers destination 
    int nbread;
    while (nbread = read(fd1, Buffer, sizeof(Buffer))) {

        write(fd2, Buffer, nbread);
    }

    //6- On ferme les fichiers 

    close(fd1);
    close(fd2);
    return 1;
}
/***
	cp_source_tar : une partie de la fonction cp (qui traite la copie dans le cas ou la source est un tarball)
	entrées: source et destination 
	sorties: int 
***/
int cp_source_tar(char source[100], char destination[100]) {

    char sourceAbs[100],destinationAbs[100];

    /// construire le chemin absolue de source
    strcpy(sourceAbs,source);
    get_ch_absolu(sourceAbs);
    remove_points(sourceAbs);
    remove_2points(sourceAbs);

    /// construire le chemin absolue de destination
    strcpy(destinationAbs,destination);
    get_ch_absolu(destinationAbs);
    remove_points(destinationAbs);
    remove_2points(destinationAbs);
    if(destinationAbs[strlen(destinationAbs)-1]!='/'){
        int i = strlen(destinationAbs);
        destinationAbs [ i ] = '/';
        destinationAbs [ i + 1] = '\0';
    }


    int fdS ,trouv = 0, entete = 0, size1 = 0, fdD,  nblus = 0, size = 0;
    //1-  on essaie d'ouvrir le tar pour savoir s'il existe 
    fdS = open_tar_file_read(sourceAbs);
    if (fdS < 0) {
        // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
        write(STDERR_FILENO,"cp : cannot stat '",5);
        write(STDERR_FILENO,source,strlen(source));
        write(STDERR_FILENO,"': No such file or directory\n",29);
        return 0; // on sort 
    }


    //  dans ce cas le tarball est bien ouvert <=> il existe et le chemin est correcte  
	int i = strlen(strstr(sourceAbs,".tar/"));

	char ch_in_tar[100]="";
	strncpy(ch_in_tar,&sourceAbs[strlen(sourceAbs)-i+5],i);

    //2- on récupere le nom du fichier dans la source
    char chemin[100] = "";
    strcpy(chemin, destinationAbs);
    strcat(chemin, recupere_nom(sourceAbs));

    if ((strcmp(sourceAbs, chemin) == 0) ) {
        close(fdS);
        write(STDERR_FILENO,"cp : '",5);
        write(STDERR_FILENO,source,strlen(source));
        write(STDERR_FILENO,"' and '",7);
        write(STDERR_FILENO,destination,strlen(destination));
        write(STDERR_FILENO,"' are the same file\n",20);
        return 0;
    }

    //3- maintenant on effectue une recherche du fichier dans le tarball pour recuperer ses informations  

    recherche_tar(fdS, ch_in_tar, & trouv, & entete, & size1);
    
    if (trouv == 0) {
        close(fdS);
        write(STDERR_FILENO,"cp : cannot stat '",5);
        write(STDERR_FILENO,source,strlen(source));
        write(STDERR_FILENO,"': No such file or directory\n",29);
        return 0;
    }    
    
    //-4 on a toruvé le fichier a copier et tout va bien donc creer la destination , 
    //    et le fichier destination n'est pas dans un tarball on le creer s'il existe pas ou on l'ecrase 
    
    fdD = open(chemin, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fdD < 0) {
        close(fdS);
        write(STDERR_FILENO,"cp: cannot create regular file '",32);
        write(STDERR_FILENO,destination,strlen(destination));
        write(STDERR_FILENO,"': Permission denied\n",21);
        return 0;
    }

    //6-on copie le contenu 

    int block = 1+(size1 >> BLOCKBITS); //recuperer le nombre de block de fichier
    for (int k = 0; k < block; k++) {
        //on recupere les block du contenu et on les insère au fichier 
        if ((nblus = read(fdS, Buffer, BLOCKSIZE)) > 0) {
            write(fdD, Buffer, nblus);
        }
    }
    close(fdS);
    close(fdD);
    return 1;
}

/***
	cp_ : fonction qui implémente le fonctionnement de cp
	entrées: source et destination 
	sorties:void
***/
int cp (char source[100], char destination[100]){

    char sourceAbs[100],destinationAbs[100];

    strcpy(sourceAbs,source);
    get_ch_absolu(sourceAbs);
    remove_points(sourceAbs);
    remove_2points(sourceAbs);

    /// construire le chemin absolue de destination
    strcpy(destinationAbs,destination);
    get_ch_absolu(destinationAbs);
    remove_points(destinationAbs);
    remove_2points(destinationAbs);
    
    if(strstr(sourceAbs, ".tar") == NULL){
        struct stat st;
        stat(sourceAbs, &st);
        if( S_ISDIR(st.st_mode)){
            write(STDERR_FILENO,"cp ; omitting directory '",25);
            write(STDERR_FILENO,source,strlen(source));
            write(STDERR_FILENO,"'\n",2);
            return 0;
        }
    }
    if(strstr(destinationAbs, ".tar") == NULL){
        struct stat st;
        stat(destinationAbs, &st);
        if(! S_ISDIR(st.st_mode)){
            write(STDERR_FILENO,"cp ; destination is not a directory '",37);
            write(STDERR_FILENO,destination,strlen(destination));
            write(STDERR_FILENO,"'\n",2);
            return 0;
        }
    }


    if ((strstr(sourceAbs, ".tar") == NULL) && (strstr(destinationAbs, ".tar") == NULL)) {
            // 1.1- source comporte pas tar --> destination comporte pas tar
            return cp_normal(source, destination);
            
    }

    if ((strstr(sourceAbs, ".tar") == NULL) && (strstr(destinationAbs, ".tar") != NULL)) {
            // 1.2- source comporte pas tar --> destination  tar
            return cp_destination_tar(source, destination);
            
    }
    
    if(strstr(sourceAbs,".tar") != NULL && strlen(strstr(sourceAbs,".tar")) <= 5){
            write(STDERR_FILENO,"cp : omitting directory '",25);
            write(STDERR_FILENO,source,strlen(source));
            write(STDERR_FILENO,"'\n",2);
            return 0;
    }
    
    if ((strstr(sourceAbs, ".tar") != NULL) && (strstr(destinationAbs, ".tar") == NULL)) {
            // 1.3- source  tar --> destination comporte pas tar
            return cp_source_tar(source, destination);
            
    }

    if ((strstr(sourceAbs, ".tar") != NULL) && (strstr(destinationAbs, ".tar") != NULL)) {
            // 1.4- source comporte tar --> destination comporte  tar 
            return cp_source_destination_tar(source, destination);
            
    }
    return 0;
}

void filter_cmd(char *listArgsRed[100],char *listeArgs[MAXCMDs]){
	int i=0;
	while((listArgsRed[i] != NULL)&&(strstr(listArgsRed[i],">") == NULL) && (strstr(listArgsRed[i],"<") == NULL)){
		listeArgs[i]=listArgsRed[i];
		i++;
	}
	listeArgs[i]=NULL;
}

//mv
void mv(char source[100], char destination[100]){
    if(cp(source, destination)){
        rm(source);
    }
}
/***



***/
int check_for_rep(char *nomfic,char *path,int *entete_lu,int *cpt){
	
	int fd=open(nomfic,O_RDONLY);
	if (fd<0){
			write(1,"zfi :rmdir : No such file \n",strlen("zfi :rmdir : No such file \n"));
			return -1 ;
	}
	char buf[513];
	int EnteteAlire=0;
	int stop=0;
	int size=0;
	int n=0;
	int ret=0;
	
	struct posix_header *st =malloc(sizeof(struct posix_header));

	while((stop==0)&&((n=read(fd,buf,BLOCKSIZE))>0)){
		st= (struct posix_header * ) buf;
		
		if((st->name)[0] == '\0'){
			stop=1;
			
		}
		if(startsWith(path,st->name)&&(st->typeflag == '5')){
			*cpt=*cpt+1;
			stop=1;
		}
		if ((strcmp(st->name,path)==0)&&(st->typeflag == '5')){
			*entete_lu=EnteteAlire;
			ret=1;
		}
		sscanf(st->size,"%o",&size);
		if(size==0){
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS);
		}
		else{
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS)+1;
		}

		lseek(fd,EnteteAlire*BLOCKSIZE,SEEK_SET);
	}
	if (n<0){
		write(1,"zfi :rmdir : Writing error \n",strlen("zfi :rmdir : Writing error \n"));
			return -1 ;
	}
	close(fd);
	//free(st);
	return ret;

}
/***
	rm_r : la fonction qui implémente le fonctionnement de la commande rm -r 
	entrées : le fichier à supprimer 
	sorties : entier 
***/
int rm_r(char *nom_rep){

	//récupération de pwd
	char *pwd=my_pwd_global();
	
	int i;
	if(strstr(pwd,".tar/") != NULL){
		i=strlen(strstr(pwd,".tar/"));
	}else{
		i=0;
	}
	char tmp[100];
	strcpy(tmp,nom_rep);
	
	if(i>0){//le chemin ou doit on supprimer un repértoire inclus un tar 
		char suite[100]="";
		strncpy(suite,&pwd[strlen(pwd)-i+5],i);
		if(strcmp(suite,"") == 0){
			if(tmp[strlen(tmp)-1] != '/')
					strcat(tmp,"/");
		}else{
			strcat(suite,tmp);
			if(tmp[strlen(tmp)-1] != '/'){
				strcat(suite,"/");
			}
			strcpy(tmp,"");
			strcpy(tmp,suite);
		}
		char tar_file[100]="";
		strncpy(tar_file,pwd,strlen(pwd)-i+4);

		//tester si le répértoire existe déjà dans le tar actuel

		int *entete_lu=malloc(sizeof(int*));
		int *cpt=malloc(sizeof(int*));
		*cpt=0;
		int ret=check_for_rep(tar_file,tmp,entete_lu,cpt); //entete_a_modifier est l'entete qu'on va modifier pour créer le nouveau rep si ret est == 0
		if(ret==0){
			write(1,"zfi : No such directory\n",strlen("zfi : No such directory\n"));
			return -1; 
		}
		//le répértoire existe on verifie si le répértoire est vide , si c'est le cas on le supprime
		
		int fd=open(tar_file,O_RDWR);
		struct posix_header *st =malloc(sizeof(struct posix_header));
		lseek(fd,*entete_lu*BLOCKSIZE,SEEK_SET);
		char buf[513]="";
		int n=read(fd,buf,BLOCKSIZE);
		if (n<0){
			write(1,"zfi : No such file\n",strlen("zfi : No such file\n"));
			return -1;
		}
			
		st=(struct posix_header * ) buf;
		st->name[0]='#'; //supprimer logiquement le repértoire
		set_checksum(st);
		lseek(fd,*entete_lu*BLOCKSIZE,SEEK_SET);
		write(fd,st,sizeof(struct posix_header));
		close(fd);
	}else{//le chemin ou doit on créer le répértoire est un répértoire ordinaire 
		//utilisation de la fonction mkdir 
		struct stat *st =malloc(sizeof(struct stat));
		if(stat(tmp,st) == 0){
			int ret=remove_directory(tmp);
		}else{
			write(1,"zfi : rm :No such directory\n",strlen("zfi : No such directory\n"));
			return -1;
		}
	}	
}

/***
	remove_directory : une fonction pour la suppression d'un répértoire non vide (pour rm -r dans le cas normal )
	entrées : path
	sorties : entier
***/
int remove_directory(char *path) {
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d) {
      struct dirent *p;

      r = 0;
      while (!r && (p=readdir(d))) {
          int r2 = -1;
          char *buf;
          size_t len;

          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
             continue;

          len = path_len + strlen(p->d_name) + 2; 
          buf = malloc(len);

          if (buf) {
             struct stat statbuf;

             snprintf(buf, len, "%s/%s", path, p->d_name);
             if (!stat(buf, &statbuf)) {
                if (S_ISDIR(statbuf.st_mode))
                   r2 = remove_directory(buf);
                else
                   r2 = unlink(buf);
             }
             free(buf);
          }
          r = r2;
      }
      closedir(d);
   }

   if (!r)
      r = rmdir(path);

   return r;
}
