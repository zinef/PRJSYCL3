#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

/* ... */

#define OLDGNU_MAGIC "ustar  "  /* 7 chars and a null */

/* ... */



#define BLOCKBITS 9
#define BLOCKSIZE 512
char pwd[100];
char Buffer[512];
typedef struct posix_header posix_header;
//int in_tar;
char *pwd_global;
char *nomf;

// La définition de la structure 
struct posix_header 
{
char name[100];
char mode[8];
char uid[8];
char gid[8];
char size[12];
char mtime[12];
char chksum[8];
char typeflag ;
char linkname[100];
char magic[6];
char version[2];
char uname[32];
char gname[32];
char devmajor[8];
char devminor[8];
char prefix[155];
char junk[12];
};


/* Compute and write the checksum of a header, by adding all
   (unsigned) bytes in it (while hd->chksum is initially all ' ').
   Then hd->chksum is set to contain the octal encoding of this
   sum (on 6 bytes), followed by '\0' and ' '.
*/

void set_checksum(struct posix_header *hd) {
  memset(hd->chksum,' ',8);
  unsigned int sum = 0;
  char *p = (char *)hd;
  for (int i=0; i < BLOCKSIZE; i++) { sum += p[i]; }
  sprintf(hd->chksum,"%06o",sum);
}



char *recupere_nom(char *path)
{

char *pend;
if((pend=strrchr(path, '/'))!=NULL){
 return pend +1 ;
}else{
	return path;
}

}






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




void recherche_tar(int fd, char nomfic[100],int *trouv, int *entete,int *size_file){

int stop=0,p=0,size=0,EnteteAlire=0;
char buff[512]="";

 struct posix_header *st =malloc(sizeof(struct posix_header));
 while((stop==0)&&((p=read(fd,buff,BLOCKSIZE))>0)){
		
                st= (struct posix_header *) buff;
		// on teste si on est pas arrivé à la fin 
                if((st->name)[0] == '\0'){
	            stop=1;
                    *entete = EnteteAlire; //sauvegarde derniere entete destination        
                   *trouv=0;
                
		}
		if (strcmp(st->name, nomfic)==0){
			stop=1;
                        
			*entete=EnteteAlire;
                     *trouv= 1;
		}
		// on récupère la taille du fichier 
                sscanf(st->size,"%o",&size);
                *size_file=size;
		if(size==0){
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS);// >>BLOCKBITS = /512
		}
		else{
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS)+1; 
		}

		lseek(fd,EnteteAlire*512,SEEK_SET);
	} 




}











void cp_destination_tar(char source[100], char destination[100]) { 
int fd1,fd2,errno,nbread,nblus,nbtotal=0;
int fd,i,j,n,m;
int erreur=0;
char sr[100],ds[100],str[100],str1[100], str2[100],nom[100],nomfic[100],str3[100];
char buff[512];
char buf[512];
int stop=0,stop2=0,p,EnteteAlire=0,entete_a_lire;
int size=0;char origine[100];

       
        // 1.2- source comporte pas tar --> destination  tar
       
        
       
        //2-  on essaie d'ouvrir le tar de destination pour savoir s'il existe 
        fd2= open_tar_file(destination);
        if(fd2<0){
        // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
         printf("le chemin destination que vous avez introduit n'est pas    valide");
         exit(EXIT_FAILURE);
        }
     


       //  dans ce cas le tarball est bien ouvert <=> il existe et le chemin est correcte  

      



       //3- on récupere le nom du fichier dans la source 
         
        nomf=recupere_nom(source);
        strcpy(nomfic,nomf);
        int z=strlen(strstr(destination,".tar/"));
        char suite[100]="";
	strncpy(suite,&destination[strlen(destination)-z+5],z);
	if(strcmp(suite,"") != 0){
		
			strcat(suite,nomfic);
			strcpy(nomfic,"");
			strcpy(nomfic,suite);
	}
        // maintenant dans nom fic, on a le chemin apartir du tar vers le fichier a copier 


       //4- maintenant on effectue une recherche du fichier dans le tarball pour savoir si il nexiste pas   
         
         int tmp;
         struct posix_header *st =malloc(sizeof(struct posix_header));
	 while((stop==0)&&((n=read(fd2,buf,BLOCKSIZE))>0)){
		
                st= (struct posix_header *) buf;
		// on teste si on est pas arrivé à la fin 
                if((st->name)[0] == '\0'){
			stop=1;
			tmp=EnteteAlire;
		}
		if (strcmp(st->name, nomfic)==0){
			stop=1;
			entete_a_lire=EnteteAlire;
		}
		// on récupère la taille du fichier 
                sscanf(st->size,"%o",&size);
		if(size==0){
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS);// >>BLOCKBITS = /512
		}
		else{
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS)+1; 
		}

		lseek(fd2,EnteteAlire*BLOCKSIZE,SEEK_SET);
	} 
	// on teste les conditions de sortie 


	if((stop==1)&&((st->name)[0] != '\0')){
        // on est au bon fichier
	printf("Le fichier source %s existe déja dans la destination",source);
	exit(EXIT_FAILURE); 
	}else{

	
        if ((stop==1)&& ((st->name)[0] == '\0')) { 
        
        // on a pas trouvé le fichier a la destination on se positionne a la derniere entete lue pour commencer la copie 
	lseek(fd2,tmp*BLOCKSIZE,SEEK_SET); 
	

        fd1=open(source,O_RDONLY);
        if(fd1<0){ 
          if (errno==ENOENT)
          	printf("\nVotre chemin n'est pas valide, veuillez réessayer ! \n");
        }
        
        //récupération des caractéristiques du fichier source
	struct stat *statbuf=malloc(sizeof(struct stat*));
	int ret=fstat(fd1,statbuf);
	    if (ret<0){
		exit(EXIT_FAILURE);
	    }
	
        
	//création de l'entête du fichier à ajouter 
        int sizefile=statbuf->st_size;
	char buf[512]="";
	char* uid=getenv("USER");
	struct posix_header *st =malloc(sizeof(struct posix_header*));

	st=(struct posix_header * ) buf;
	strcpy(st->name,nomfic);
	sprintf(st->size,"%o",sizefile);
	st->typeflag='0';
	sprintf(st->mode,"0000700");
	//time(&t);
	//sprintf(st->mtime, "%ld",t);
	sprintf(st->uid,"%d",getuid());
	sprintf(st->gid,"%d",getuid());
	strcpy(st->uname,uid);
	strcpy(st->gname,uid);
	strcpy(st->magic,TMAGIC);
	strcpy(st->version,TVERSION);
	set_checksum(st);
	//écriture de l'entete
	int w=write(fd2,st,BLOCKSIZE);
	if(w<0){
		perror("err dans l'écriture");
		exit(EXIT_FAILURE);
	}
	int nblus;
	while((nblus=read(fd1,buff,BLOCKSIZE))>0){
		write(fd2,buff,BLOCKSIZE);
	}
	

}
	
}	
close(fd1);close(fd2); 	
}



	

void cp_source_destination_tar(char source[100], char destination[100]) { 
int fd1,fd2,errno,nbread,nblus,nbtotal=0;
int fd,i,j,n,tmp,m;
int erreur=0;
char sr[100],ds[100],str[100],str1[100],src[100], str2[100],nom[100],nomfic[100],str3[100];
char buff[512];
char buf[512];
int stop=0,stop2=0,p,EnteteAlire=0,entete_a_lire;
int size_src=0,size=0;char origine[100];


        strcpy(src, source);
        //1-  on essaie d'ouvrir les tars pour savoir s'ils existent 
        fd1= open_tar_file(source);
        if(fd1<0){
        // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
         printf("le chemin que vous avez introduit n'est pas valide");
         exit(EXIT_FAILURE); // on sort 
          }
         
        fd2= open_tar_file(destination);
        if(fd2<0){
        // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
         printf("le chemin que vous avez introduit n'est pas valide");
         exit(EXIT_FAILURE); // on sort 
          }  
           
       
       
       //2- on récupere le chemin ou il devra etre a la destination
         char nomfic2[100]="";
        nomf=recupere_nom(source);   // on recupere le nom tout seul 
        strcpy(nomfic,nomf);         // nomfic= nom du fichier 
        int z=strlen(strstr(destination,".tar/"));
        char suite[100]="";  // dans le tar destination 
	strncpy(suite,&destination[strlen(destination)-z+5],z);
	if(strcmp(suite,"") != 0){
		
			strcat(suite,nomfic);
			strcpy(nomfic,"");
			strcpy(nomfic,suite);
	}
       // nomfic= /dst/f1.txt destinaion
       
       
       // 3- on recupere le chemin du fichier a la source 
       // on cherche le chemin vers le fichier dans la source 
 
        int l=strlen(strstr(src,".tar/"));
        char suite_source[100]="";  
	strncpy(suite_source,&src[strlen(src)-l+5],l);
 
	strcpy(nomfic2,suite_source);
       // nomfic2= src/f1.txt
    
      

       // 4 on recherche le fichier source
       int trouv=0,entete=0,size1=0;
       int trouv2=0,entete2=0,size2=0;
       recherche_tar(fd1,nomfic2,&trouv,&entete,&size1);
       int block= size1 >> BLOCKBITS; 
       if (trouv==0){
         printf("\n Le fichier source n'existe pas veuillez verifier votre chemin\n");
         exit(EXIT_FAILURE);
       }
       if (trouv==1){
         char suite_dest[100]="", exist_dest[100]="";
         strcpy(exist_dest,destination);
         int l=strlen(strstr(exist_dest,".tar/"));
         strncpy(suite_dest,&exist_dest[strlen(exist_dest)-l+5],l);
         int trouv3=0,entete3,size3;
         recherche_tar(fd2,suite_dest,&trouv3,&entete3,&size3);
         if (trouv3==0){ 
            printf("le chemin destination est invalide \n");
            exit(EXIT_FAILURE);
             }
         else{       
                recherche_tar(fd2,nomfic,&trouv2,&entete2,&size2);
                if (trouv2==1) {
                     printf("\n Le fichier existe deja a la destination \n");
                     exit(EXIT_FAILURE);
                }
                if (trouv2==0) {
                   lseek(fd1,entete*BLOCKSIZE,SEEK_SET);     
                   lseek(fd2,entete2*BLOCKSIZE,SEEK_SET);
                   strcpy(buff, "");
                   char buffer[512]="";
                   struct posix_header *s3=malloc(sizeof(struct posix_header*));
                   //6-on copie l'enetete dans le nv fichier 
                    
	           if (nbread= read(fd1, buffer, BLOCKSIZE)>0){
	                  s3=(struct posix_header*)buffer;
                          strcpy(s3->name , nomfic);
	                  set_checksum(s3);
	                  write(fd2,s3,BLOCKSIZE);
	           }
                   //7-on copie le contenu du fichier source vers dest
                   for (int k=0;k<block;k++){ 
                   //on recupere les block eton les insère au fichier 
	               if ((nblus= read(fd1, buffer, BLOCKSIZE))>0){
	                    write(fd2,buffer,BLOCKSIZE);
	               }
	           }
             
                }

        }  } 
  strcpy(nomfic,"");strcpy(nomfic2,"");
  close(fd1);close(fd2);   
     
 }




void cp_normal (char source[100], char destination[100]){
char buff[512],origine[100];
int fd1,fd2,nbread;


 //1-on verifie qu'on a recu deux arguments 
   if(source== NULL || destination==NULL )
     {
     printf("\nSyntaxe erreur: cp source_du_fichier destination_du_fichier\n");
     printf("Erreur dans les paramètres\n");
     exit (EXIT_FAILURE);
     }      
   
   //2- on vérifie si la source= destination le fichier ne sera pas copier comme  on aura juste une duplication. 

   if(strcmp(source,destination)==0)
     {
     printf( "le fichier %s existe déja à la destination \n",source);
     exit(EXIT_FAILURE);
     }
   
     char chemin[100]="";  
 
        nomf=recupere_nom(source);   // on recupere le nom tout seul 
        strcpy(chemin, destination);
        strcat(chemin,nomf);
   if((strcmp(source,destination)==0) || (open(chemin,O_RDONLY))!=-1)
     {
     printf( "le fichier %s existe déja à la destination \n",nomf);
     exit(EXIT_FAILURE);
     }
      
  //3- si les parametres sont bien entrés et different on ouvre le fichier source 

    fd1=open(source,O_RDONLY);
       if(fd1<0){ 
          if (errno==ENOENT){
          printf("\nVeuillez vérifier le nom du fichier \n");
          }
       }
       

  //4- Si le fichier source est bien ouvert, on ouvre le fichier destination avec les modes d'ecriture et de création 
  
 getcwd( origine, sizeof(origine));   // on sauvgarde le chemin actuel
 chdir(destination); // on a vers la destination
 char *n = recupere_nom(source);
 fd2= open(n,O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
 if (fd2<0){
    printf("\nErreur dans l'ouverture du fichier %s \n errno : %d \n", destination,errno);
    exit(EXIT_FAILURE);
 }
 
  chdir(origine); //on reviens vers l'origine 
    


  //5- Si le fichier destination est bien ouvert, tout est bien passé, on commence le transfert des données de source vers destination 
  
    while((nbread= read(fd1, buff, sizeof(buff)))&& nbread>0){
     if (write(fd2, buff,nbread) <0){
       printf("\nErreur dans la copie des fichiers, veuillez réesayer\n");
       exit(EXIT_FAILURE);
      }
    }

  //6- On ferme les fichiers 
  
  if (close(fd1)==-1)
  printf("Erreur dans la fermeture du fichier du fichier %s ",source);
  if (close(fd2)==-1)
  printf("Erreur dans la fermeture du fichier du fichier %s ", destination);


}





void cp_source_tar(char source[100], char destination[100]){

int fd1,fd2,errno,nbread,nblus,nbtotal=0;
int delete,reussi,fd,i,j,n,m;
int erreur=0;
char sr[100],ds[100],str[100],str1[100], str2[100],nom[100],nomfic[100];
char buff[512]="";
char buf[512]="";
int stop=0,stop2=0,p,EnteteAlire=0,entete_a_lire;
int size=0;char origine[100];

               
       //2-  on essaie d'ouvrir le tar pour savoir s'il existe 
        fd1= open_tar_file(source);
        if(fd1<0){
        // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
         printf("le chemin que vous avez introduit n'est pas valide");
         exit(EXIT_FAILURE) ;// on sort 
          }
         //  dans ce cas le tarball est bien ouvert <=> il existe et le chemin est correcte  
       
       
       //3- on récupere le nom du fichier dans la source
         char *str3 = recupere_nom(source);
         strcpy(nomfic,str3);
        
        char chemin[100]="";  
        
        nomf=recupere_nom(source);   // on recupere le nom tout seul 
        strcpy(chemin, destination);
        strcat(chemin,nomf);
   if((strcmp(source,destination)==0)|| (open(chemin,O_RDONLY))!=-1)
     {
     printf( "le fichier %s existe déja à la destination \n",nomf);
     exit(EXIT_FAILURE);
     }
      
      
       //4- maintenant on effectue une recherche du fichier dans le tarball pour recuperer ses informations  
         
         
         struct posix_header *st =malloc(sizeof(struct posix_header));
	 while((stop==0)&&((n=read(fd1,buf,BLOCKSIZE))>0)){
		
                st= (struct posix_header *) buf;
		// on teste si on est pas arrivé à la fin 
                if((st->name)[0] == '\0'){
			stop=1;
		}
		if (strcmp(st->name, nomfic)==0){
			stop=1;
			entete_a_lire=EnteteAlire;
		}
		// on récupère la taille du fichier 
                sscanf(st->size,"%o",&size);
		if(size==0){
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS);// >>BLOCKBITS = /512
		}
		else{
			EnteteAlire=EnteteAlire+ ((size + BLOCKSIZE  ) >> BLOCKBITS)+1; 
		}

		lseek(fd1,EnteteAlire*BLOCKSIZE,SEEK_SET);
	} 
	  // on teste les conditions de sortie 
	if((stop==1)&&((st->name)[0] != '\0')){ // on est au bon fichier
	
	
      //-4 on a toruvé le fichier a copier et tout va bien donc creer la destination , et le fichier destination n'est pas dans un tarball on le creer s'il existe pas ou on l'ecrase 
	 char *n = recupere_nom(source);
         getcwd( origine, sizeof(origine));   // on sauvgarde le chemin actuel
         chdir(destination); // on a vers la destination
         fd2= open(n,O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
            if (fd2<0){
            printf("\nErreur dans l'ouverture du fichier %s . errno : %d \n",   destination,errno);
             exit(EXIT_FAILURE); } 


      chdir(origine);
    
        //5-on se positionne au bonnes positions 
 
        lseek(fd1,0,SEEK_SET); // on se repositionne au debut fchier source
        lseek(fd1,entete_a_lire*BLOCKSIZE,SEEK_SET); //on va directement a lentete concernée 
	struct posix_header *st=malloc(sizeof(struct posix_header*));
	if ((nblus= read(fd1, buff, BLOCKSIZE)>0)){
		st=(struct posix_header*)buff;
		sscanf(st->size,"%o",&size);
	}else{
		exit(EXIT_FAILURE);
	}
        int block= size >> BLOCKBITS; //recuperer le nombre de block de fichier 
		
       
       //7-on copie le contenu 
            for (int k=0;k<block;k++){ 
            //on recupere les block du contenu et on les insère au fichier 
	     if ((nblus= read(fd1, buf, BLOCKSIZE))>0){
	          write(fd2,buf,nblus);
		}
	    }
	     
      }
/*** fin de 1.1 ; premier cas ***/
}






// Une fonction qui renvoie la taille du fichier 
int get_file_size(posix_header *header)
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
void seek_next_entete (int fd ,posix_header *header)
{
seek_n_block(fd,get_file_size(header));
}

// copier un fichier 








int cp(char source[100], char destination[100])
{

int fd1,fd2,errno,nbread,nblus,nbtotal=0;
int delete,reussi,fd,i,j,n,m;
int erreur=0;
char sr[100],ds[100],str[100],str1[100], str2[100],nom[100],nomfic[100],str3[100];
char buff[512];
char buf[512];
int stop=0,stop2=0,p,EnteteAlire=0,entete_a_lire;
int size=0;char origine[100];
strcpy(str1,source);
strcpy(str2,destination);


  if ((strstr(str1, ".tar/")== NULL)&& (strstr(str2, ".tar/")== NULL)){ 
      //  Fonctionnement normal de cp /
       cp_normal(source,destination);
       exit(EXIT_FAILURE);
  
  }else{
   // la source OU la destination implique un tarball // 
       
     
       
  if ((strstr(source, ".tar/")!= NULL)&& (strstr(destination, ".tar/")== NULL)){
        // 1.1- source comporte tar --> destination sans tar 
        cp_source_tar(source,destination);
        exit(EXIT_FAILURE);
      }
       
  if ((strstr(source, ".tar/")== NULL)&& (strstr(destination, ".tar/")!= NULL)){
       // 1.2- source comporte pas tar --> destination  tar
       cp_destination_tar(source,destination);
       exit(EXIT_FAILURE);
      }   
       

  if ((strstr(source, ".tar/")!= NULL)&& (strstr(destination, ".tar/")!= NULL)){
       // 1.3- source comporte tar --> destination comporte  tar 
       cp_source_destination_tar(source,destination);
       exit(EXIT_FAILURE);
      }
  }
}


int main(){
cp("test.tar/dossier/f1.txt", "/home/faycal/Documents/");

return 0;
}
