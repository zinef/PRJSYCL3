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


#define BLOCKSIZE 512
#define BLOCKBITS 9



static volatile int running = 1;
void stop(int a){
    running = 0;
}

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


/*******************************************************************************************/
/*******************************************************************************************/
/************************************   "ls"   *********************************************/
/*******************************************************************************************/
/*******************************************************************************************/

//// pour le cas de tar
//cette fonction permet de verifier si le repertoir "dir_name" contien le fichier nomme "header_name" 
//retourn 1 si le repertoir "dir_name" contien un fichier nomme "header_name"
// sinon retourn -1
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

//cette fonction permet de deplaser le cursor au block de repertoire "dir_name"
int get_dir(int fd,char dir_name[100], posix_header *header){
    int i , cpt = 0;
    read(fd,Buffer,BLOCKSIZE);cpt++;
    while (Buffer[0]!='\0' && Buffer[0]!=EOF)
    {   
        get_entete_info(header, Buffer);        
        if(cmp_name(header->name,dir_name) == 0){
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

//avoir les permitions d' un fichier/repertoir de la forme "d-w-rw-r--"
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

//cette fonction va ecrire le resultat de "ls" et "ls -l"
//option == {0,1,2,3}
// 0 et 2 pour ls
// 1 et 3 pour ls -l
// on a ajouter 2 et 3 pour le cas ou on doit lister le contenu d un fichier tar. Ex: "ls test.tar"
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

//Cette procedure est responsable de la comande "ls" dans les tar
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

//cette fonction va ecrire le resultat de "ls" et "ls -l"
// "ls"    ==> option == 0 
// "ls -l" ==> option == 1
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
            perror(str1);
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

            sprintf(str,"%2ld",st.st_nlink);
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

//Cette procedure est responsable de la comande "ls" out tar
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

//Cette procedure est responsable de la comande "ls" 
void ls(char ch[100],int option){
    int fd;int i;
    struct stat sb,sp;    
    char str[100],str2[100];

    strcpy(str,ch);         /// pour ne pas changer le chemin en entrer, copie "ch" dans "str"
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

            //ls d'un repertoir dans un tar
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

/*******************************************************************************************/
/*******************************************************************************************/
/************************************   "cat"   ********************************************/
/*******************************************************************************************/
/*******************************************************************************************/

//cette fonction permet de deplaser le cursor au block du fichier "file_name"
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

//Cette procedure est responsable de la comande "cat" out tar
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

//Cette procedure est responsable de la comande "cat" 
void cat(char ch[100]){
    
    ///dans le cas ou "cat" et executer sans argumment 
    if(strlen(ch)==0){
        int cpt;
        int i=1;
        while (running){
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

/*******************************************************************************************/
/*******************************************************************************************/
/********************   ">"  ">>"  "<"  "2>"  "2>>"  "2>&1"  "2>>&1"   *********************/
/*******************************************************************************************/
/*******************************************************************************************/

/// recherche dans un tar 
int search(char ch[100]){
    posix_header *header;
    int fd = open_tar_file_read(ch);
    int i;
    if( fd <= 0 ){
        perror(ch);
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

//int type : nombre de redirection , veux 'type == 1' pour '>'    et '>>'   , 'type == 2' pour '2>' et '2>>' , 
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
    //cp(out_file,ch);

}
//char ch : nom du ficher de sortie
//int std_out_copy : pour sauvgareder dup(1)
//char out_file : dans le cas ou en ecrie dans un tar on va cerer un fichier au /tmp ecrire de dans a la fin on recopier le resultat ou on veut
//int type : nombre de redirection , veux 'type == 1' pour '>'    , 'type == 2' pour '>>'    , 'type == 3' pour '2>' ,
//                                         'type == 4' pour '2>>' , 'type == 5 pour '2>&1   et 'type == 6' pour '2>>&1'
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
            return 0;
        }

        strcpy(str3,"/tmp");
        i = strlen(strstr(str,".tar/"));
        strncpy(str2 , &str[strlen(str) - i + 5] , i);

        strcat(str3,str2);
        strcpy(out_file,str3);
            
        if (res == 1){

            if( type == 2 || type == 4 || type == 6){
                //cp(ch,str3);
            }
            //rm(ch)
        }

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
            return 0;
        }

        strcpy(str3,"/tmp");
        i = strlen(strstr(str,".tar/"));
        strncpy(str2 , &str[strlen(str) - i + 5] , i);

        strcat(str3,str2);

        //cp(ch,str3);

    /// si le chemin n'inclue pas des tar
    }else{
        strcpy(str3,str);
    }

    
    fd = open(str3,O_RDONLY);

    if(fd <= 0 ){ 
        perror(ch); 
        return 0; 
    }

    dup2( fd , STDIN_FILENO );
    return fd;
}

/// cette fonction gere les entres/sorties
///          cette fonction retourn le fd du fichier 
///          ch       : le chemin du ficher a ecrire
///          out_file : dans le cas ou on veut ecrire dans un fichier dans un tar 
///                   on ecrie dans un ficher dans /tmp
///                   et retourn le chemin du fichier pour le deplacer apres vers son emplacement 
///          type     : definie le type de redirection
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
        return redercet_stdin( ch );
    }

}

/// cette procedure gere la fin des redirectiom       
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




int main(int argc, char *argv[]){

    /////////////////////////////
    ////zinou hadi dirha fl main 
    /// signal bach une fois ndir (ctrl + C) le programe mayahbassche mais y habess l action en cous
    signal(SIGINT , stop);
    /// hadou bsh nsauvgarder les donnes 
    ///ila ksh haja gouli 
    std_out_copy   = dup(STDOUT_FILENO);
    std_error_copy = dup(STDERR_FILENO);
    std_in_copy    = dup(STDIN_FILENO );
    ////////////////////////////

    posix_header *header;
    int size;
    int fd1,fd2;

    

    char ch[100],ch1[100],ch2[100],out_file_std_out[100],out_file_std_error[100];

    strcpy(ch,"./my project/tata/../tata/tatadpll (1).ml");
    
    strcpy(pwd,"/home/islam/Desktop/my project");

    ls(".././my project/test.tar",1);
    cat("");
    cat("");

    return 0;
}