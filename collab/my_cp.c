#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <errno.h>


#define BLOCKSIZE 512
#define BLOCKBITS 9





char pwd[100];///zinou normanement nta rak mnomeihaa pwd_global 
char Buffer[BLOCKSIZE];///le buffer pour les lires ecriture
int std_out_copy , std_error_copy , std_in_copy;/// des variables pour sauvgarder les entres et les sorties


/// structure d un bloc header dans tar 
typedef struct posix_header  posix_header;

struct posix_header
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
  char junk[12];                /* 500 */
};                              /* Total: 512 */

/// recupirer les donnes de l entete 
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
//cette fonction retourn le nombre de bloc d un fichier apartir de header->size
int get_file_size(posix_header *header){
    int size;
    sscanf(header->size,"%o",&size);
    return (size + 512 - 1) / 512;
}

//cette fonction va avancer le curser par nb_block
void seek_n_block(int fd,int nb_block){
    lseek(fd,nb_block*BLOCKSIZE,SEEK_CUR);
}

//cette fonction va avancer le curser a la prochainne entete
void seek_next_entete(int fd, posix_header *header){
    seek_n_block(fd,get_file_size(header));
}

//cette fonction conpare deux chainnes de carractaire en ignorant le "/" a la fin
//  ex: "name" == "name/" 
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

//cette fonction permet d ouvrir le fichier tar avec la permition de lecture
// Ex: ch = "/home/aaaa/bbbb/cccc/dddd.tar/eeeee"
//     return open("/home/aaaa/bbbb/cccc/dddd.tar",READ); 
int open_tar_file_read(char ch[100]){////zinou pas la meme que rm car hadi read
    char s[100];
    char str2[100];
    strcpy(s,ch);
    int i = strlen(strstr(s,".tar/"));
    strncpy(str2,ch,strlen(ch) - i + 4);
    str2[strlen(ch) - i + 4]='\0';
    return open(str2,O_RDONLY);
}

//Cette fonction permet de construire le chemin absolue d'un chemain "ch" donner, a patir de "pwd_globale"
void get_ch_absolu(char *ch){
    
    //si ch comance par "/" alors on a entrer un chemin absolue
    if(ch[0]=='/'){
        return;//pas de changement 
    }
    // sinon "ch" est une chemin relatif, donc 
    char res[100];           /// "res" le chemin final serai ici tq :res = pwd + ch
                             
    strcpy(res,pwd);         /// res = pwd 
    int i = strlen(res);

    if(res[i-1]!='/'){       /// on verifie que "res" fini par "/" sinon je l'ajoute
        res[i]  ='/';
        res[i+1] = '\0';
    }
    
    strcat(res,ch);          /// res = res + pwd
    strcpy(ch,res);          /// puisque on a un passage par variable donc on met le resultat dans "ch"
}

//Cette fonction permet d'enlever les "." d'un chemain donner
void remove_points(char *ch){
    /// "ch" ne contien pas de "/." on change rien 
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
    
    /// on va faire une analyse sur le chemain
    ch1 = strtok(str,"/");
    /// on va simplement suprimer tout les "./" de la chainne, et maitre le resultat dans "ch"
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
    /// si on le chemain fini par "." on ignore le "." 
    if( strcmp(ch2,".") != 0 ) strcat(ch ,ch2);
}

//Cette fonction permet d'enlever les ".." d'un chemain donner
void remove_2points(char *ch){
     /// "ch" ne contien pas de "/." on change rien 
    if(strstr(ch,"..") == NULL){
        return;
    }

    /// on assure que le chemin fini par "/"
    if(ch[strlen(ch)-1]!='/'){
        ch[strlen(ch)+1]='\0';
        ch[strlen(ch)]='/';
    } 

    char str[100] , *ch1, ch2[100];
    strcpy(str,ch);
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
        }
    }
    strcat(ch,"/");
    if(strcmp(ch2,"..")!=0)strcat(ch ,ch2);
    
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


                                ///  CP

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
/////done
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


////cv
void recherche_tar(int fd, char nomfic[100],int *trouv, int *entete,int *size_file){

    int size=0,EnteteAlire=0;
    
    struct posix_header *st = malloc(sizeof(struct posix_header));
    
    while( read(fd,Buffer,BLOCKSIZE)){
		
        st = (struct posix_header *) Buffer;
		// on teste si on est pas arrivé à la fin 
        if((st->name)[0] == '\0'){
                lseek(fd,-BLOCKSIZE,SEEK_CUR);
                *entete = EnteteAlire; //sauvegarde derniere entete destination        
                *trouv=0;//// on a rien trouver
                return;
		}
        sscanf(st->size,"%o",&*size_file);
		if (strcmp(st->name, nomfic)==0){        
			*entete=EnteteAlire;
			*trouv= 1;//// on a trouver unn fichoer ab=vec le meme nom
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

int open_tar_file_rdwr(char ch[100]){////zinou pas la meme que rm car hadi read
    char s[100];
    char str2[100];
    strcpy(s,ch);
    int i = strlen(strstr(s,".tar/"));
    strncpy(str2,ch,strlen(ch) - i + 4);
    str2[strlen(ch) - i + 4]='\0';
    return open(str2,O_RDWR);
}


int checkIfValide(char source[100], char destination[100]){
    //1-on verifie qu'on a recu deux arguments 
    if(source== NULL || destination==NULL ){
        printf("\nSyntaxe erreur: cp source_du_fichier destination_du_fichier\n");
        printf("Erreur dans les paramètres\n");
        return 0;
    } 

    //2- on vérifie si la source= destination le fichier ne sera pas copier comme  on aura juste une duplication. 
    char str[100];
    if(strlen(destination) <= strlen(source)){

        strncpy(str,source,strlen(destination));

        if(strcmp(str,destination) == 0){
            printf( "le fichier %s existe déja à la destination \n",source);
            return 0;
        }
    }

    str[0]='\0';
 
    strcpy(str, destination);
    strcat(str,recupere_nom(source));

    if((strcmp(source,destination)==0) || (open(str,O_RDONLY))!=-1)
    {
        printf( "le fichier %s existe déja à la destination \n",str);
        return 0;
    }
    return 1;

}

void set_checksum(struct posix_header *hd) {
  memset(hd->chksum,' ',8);
  unsigned int sum = 0;
  char *p = (char *)hd;
  for (int i=0; i < BLOCKSIZE; i++) { sum += p[i]; }
  sprintf(hd->chksum,"%06o",sum);
}


int posix_to_buffer(char ch_in_tar[100],int fdS, int fdD, posix_header *st){
    //récupération des caractéristiques du fichier source
    struct stat * statbuf = malloc(sizeof(struct stat * ));
   
    if ( fstat(fdS, statbuf) < 0) {
        printf("error");
        return 0;
    }

     //création de l'entête du fichier à ajouter 
    
    char * uid = getenv("USER");
    (st) = (posix_header *) Buffer;

    strcpy(st -> name, ch_in_tar);
    sprintf(st -> size, "%o",(int) statbuf -> st_size);
    st -> typeflag = '0';
    sprintf(st -> mode, "0000700");
    //time(&t);
    //sprintf(st->mtime, "%ld",t);
    sprintf(st -> uid, "%d", getuid());
    sprintf(st -> gid, "%d", getuid());
    strcpy(st -> uname, uid);
    strcpy(st -> gname, uid);
    strcpy(st -> magic, TMAGIC);
    strcpy(st -> version, TVERSION);
    set_checksum(st);

    //écriture de l'entete
    if (write(fdD, st, BLOCKSIZE) < 0) {
        perror("\n Erreur dans la copie veuillez réesayer\n");
        return 0;
    }

    return 1;
}


///done
void cp_normal(char source[100], char destination[100]){
    
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


    //printf("%s  ->  %s",sourceAbs,destinationAbs);
    if(checkIfValide(sourceAbs,destinationAbs) == 0){
        return;
    }
    

    int fd1,fd2;
    fd1 = open(sourceAbs, O_RDONLY);
    if (fd1 < 0) {
        
            printf("\nVeuillez vérifier le nom du fichier \n");
        return;
    }

    //4- Si le fichier source est bien ouvert, on ouvre le fichier destination avec les modes d'ecriture et de création 

    char dest_file[100];
    
    strcpy(dest_file,destinationAbs);
    
    strcat(dest_file,recupere_nom(sourceAbs));

    fd2 = open(dest_file, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd2 < 0) {
        printf("\nErreur dans l'ouverture du fichier %s \n errno : %d \n", destination, errno);
        return;
    }

    //5- Si le fichier destination est bien ouvert, tout est bien passé, on commence le transfert des données de source vers destination 
    int nbread;
    while ((nbread = read(fd1, Buffer, sizeof(Buffer))) && nbread > 0) {
        if (write(fd2, Buffer, nbread) < 0) {
            printf("\nErreur dans la copie des fichiers, veuillez réesayer\n");
            return;
        }
    }

    //6- On ferme les fichiers 

    close(fd1);
    close(fd2);

}

//////done
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


    int fd1,trouv = 0, entete = 0, size1 = 0, fd2,  nblus = 0, size = 0;
    //1-  on essaie d'ouvrir le tar pour savoir s'il existe 
    fd1 = open_tar_file_read(sourceAbs);
    if (fd1 < 0) {
        // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
        printf("le chemin que vous avez introduit n'est pas valide");
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

    if ((strcmp(sourceAbs, destinationAbs) == 0) || (open(chemin, O_RDONLY)) != -1) {
        printf("le fichier %s existe déja à la destination \n", source);
        return 0;
    }

    //3- maintenant on effectue une recherche du fichier dans le tarball pour recuperer ses informations  

    recherche_tar(fd1, ch_in_tar, & trouv, & entete, & size1);
    
    if (trouv == 0) {
        close(fd1);
        printf("\n le fichier %s n'existe pas \n", source);
        return 0;
    }    
    
    //-4 on a toruvé le fichier a copier et tout va bien donc creer la destination , 
    //    et le fichier destination n'est pas dans un tarball on le creer s'il existe pas ou on l'ecrase 
    
    fd2 = open(chemin, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd2 < 0) {
        close(fd1);
        printf("\nErreur dans l'ouverture du fichier %s . \n", destination);
        return 0;
    }
    /*
    lseek(fd1,-BLOCKSIZE,SEEK_CUR);
    nblus = read(fd1,Buffer,BLOCKSIZE);
write(1, Buffer, nblus);write(1,"\nyes ..\n", 8);*/
    //6-on copie le contenu 

    int block = size1 >> BLOCKBITS; //recuperer le nombre de block de fichier

    for (int k = 0; k < block; k++) {
        //on recupere les block du contenu et on les insère au fichier 
        if ((nblus = read(fd1, Buffer, BLOCKSIZE)) > 0) {
            write(fd2, Buffer, nblus);
        }
    }
    close(fd1);
    close(fd2);
    return 1;
}


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


    // 1.2- source comporte pas tar --> destination  tar

    //2-  on essaie d'ouvrir le tar de destination pour savoir s'il existe 
    int fdS, fdD, errno, nbread, nbtotal = 0;


    fdD = open_tar_file_rdwr(destinationAbs);
    if (fdD < 0) {
        // le tarball n'existe pas ou le chemin vers le tarball n'existe pas 
        printf("le chemin destination que vous avez introduit n'est pas    valide -> %s\n",destinationAbs);
        return 0;
    }
    //  dans ce cas le tarball est bien ouvert <=> il existe et le chemin est correcte  

    //3- on récupere le nom du fichier dans la source ainsi ou il doit etre mis

    int i = strlen(strstr(destinationAbs,".tar/"));

	char ch_in_tar[100]="";
	strncpy(ch_in_tar,&destinationAbs[strlen(destinationAbs)-i+5],i);

    //2- on récupere le nom du fichier dans la source
   
    strcat(ch_in_tar, recupere_nom(sourceAbs));

    // maintenant dans nom fic, on a le chemin apartir du tar vers le fichier a copier 

    //4- maintenant on effectue une recherche du fichier dans le tarball pour savoir si il nexiste pas   
    int trouv = 0, entete = 0, size1 = 0;
    recherche_tar(fdD, ch_in_tar, & trouv, & entete, & size1);

    if (trouv == 1) {
        printf("\n le fichier %s existe déja à la destination \n", source);
        return 0;
    }
        
    fdS = open(source, O_RDONLY);
    if (fdS < 0) {
            printf("\nVotre chemin n'est pas valide, veuillez réessayer ! \n");
            return 0;
    }

    posix_header *st = malloc(sizeof( posix_header ));;
    if(posix_to_buffer(ch_in_tar, fdS, fdD, st ) == 0){
        return 0;
    }

    int nblus;
    while (nblus = read(fdS, Buffer, BLOCKSIZE)) {
        write(fdD, Buffer, nblus);
    }
    close(fdS);
    close(fdD);
    return 1;
}


void cp_source_destination_tar(char source[100], char destination[100]) {

    char fake_distination[100]="/tmp";

    if(cp_source_tar( source, fake_distination)){

        strcat(fake_distination,"/");                   ////  "/tmp/"
        strcat(fake_distination,recupere_nom(source));  ////  "/tmp/nom_fichier"

        cp_destination_tar( fake_distination, destination);

        remove(fake_distination);
    }
}

void cp (char source[100], char destination[100]){

    if ((strstr(source, ".tar") == NULL) && (strstr(destination, ".tar") == NULL)) {
            // 1.1- source comporte pas tar --> destination comporte pas tar
            cp_normal(source, destination);
            return;
    }

    if ((strstr(source, ".tar") == NULL) && (strstr(destination, ".tar") != NULL)) {
            // 1.2- source comporte pas tar --> destination  tar
            cp_destination_tar(source, destination);
            return;
    }

    if ((strstr(source, ".tar") != NULL) && (strstr(destination, ".tar") == NULL)) {
            // 1.3- source  tar --> destination comporte pas tar
            cp_source_tar(source, destination);
            return;
    }

    if ((strstr(source, ".tar") != NULL) && (strstr(destination, ".tar") != NULL)) {
            // 1.4- source comporte tar --> destination comporte  tar 
            cp_source_destination_tar(source, destination);
            return;
    }

    write(STDOUT_FILENO,"unknown erro\n",13);
}

int main(){
    strcpy(pwd,"/home/islam/Desktop");
    cp("./main_cat_ls_red.c","./test");

}