#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BLOCKBITS 9
#define BLOCKSIZE 512
char pwd[100];
char Buffer[512];
typedef struct posix_header posix_header;
int in_tar;
char *pwd_global;


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



void rm_in_tar(int fd,char file_name[100])
{
	char buf[513];
	int EnteteAlire=0;
	int stop=0;
	int size=0;
	int n=0;
	int ret=0;
	int entete_a_lire;
	struct posix_header *st =malloc(sizeof(struct posix_header));
	while((stop==0)&&((n=read(fd,buf,BLOCKSIZE))>0)){
		
                st= (struct posix_header *) buf;
		// on teste si on est pas arrivé à la fin 
                if((st->name)[0] == '\0'){
			stop=1;
		}
		if (strcmp(st->name,file_name)==0){
			stop=1;
			entete_a_lire=EnteteAlire;
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
	}
     // on teste les conditions de sortie 
	if((stop==1)&&((st->name)[0] != '\0')){ // on est au bon fichier 
		lseek(fd,0,SEEK_SET); // on se repositionne au debut
		lseek(fd,entete_a_lire*BLOCKSIZE,SEEK_SET); //on va directement a lentete concernée 
		write(fd,"#",1);// # i.e le fichier est supprimé logiquement 
		//Une suppresion logique 
		//une boucle pour supprimer logiquement (i.e mettre le début des blocs concernés par le fichier à \0 )
		int block= size >> BLOCKBITS; //recuperer le nombre de block de fichier 
		while((block>0)&&((n=read(fd,buf,BLOCKSIZE))>0)){
			write(fd,"#",1);
			block--;  /* supprimer les blocks logiquement */
		}
	}
	if (n<0){
		perror("erreur dans la lecture");
		exit(errno);
	}
}



int rm(char chaine[100])
{
int delete,reussi,fd,i;
int erreur=0;
char str[100], nom[100];
strcpy(str,chaine);
if (in_tar==0){
   if (strstr(str, ".tar/")== NULL){ // Fonctionnement normal de rm
    
           delete= remove(chaine);
           if (delete !=0){
               erreur=errno;
               if (erreur==ENOENT )
               printf("Le fichier n'existe pas\n");
           }
           else 
             printf("le Fichier a été bien supprimé\n");

    } // Fin du fonctionnement normal de rm
   else {
           fd= open_tar_file(str);
           if (fd<0) // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
           {
              
              printf("le chemin que vous avez introduit n'est pas valide\n");
              return 0; // on sort 
           }
           // si le chemin vers le tarball existe et que ce dernier est bien ouvert
           // on recupere le chemin jusqu'au tar et on recupere la suite du chemain dans une autre chaine
           i = strlen(strstr(chaine,".tar/")); 
           strncpy(str,&chaine[strlen(chaine)- i + 5 ] , i );
           rm_in_tar(fd,str); 
           close(fd);
   }
}else{ // on est dans un tarball 
   		fd= open_tar_file(str);
           if (fd<0) // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
           {
              
              printf("le chemin que vous avez introduit n'est pas valide\n");
              return 0; // on sort 
           }
           // si le chemin vers le tarball existe et que ce dernier est bien ouvert
           // on recupere le chemin jusqu'au tar et on recupere la suite du chemain dans une autre chaine
           i = strlen(strstr(chaine,".tar/")); 
           strncpy(str,&chaine[strlen(chaine)- i + 5 ] , i );
           rm_in_tar(fd,str); 
           close(fd);
  	 
  	
	}
}







int main()
{
	in_tar=0;
	rm("../ProjetSyst/zizou");
	
	return 0;
}
