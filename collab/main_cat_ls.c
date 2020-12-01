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

#define BLOCKSIZE 512
#define BLOCKBITS 9

int in_tar;
char pwd[100];
char Buffer[BLOCKSIZE];


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

void get_entete_info(posix_header *header,char buffer[BLOCKSIZE]){
    strncpy(header->name     , buffer      , 100);
    strncpy(header->mode     , &buffer[100], 8  );
    strncpy(header->uid      , &buffer[108], 8  );
    strncpy(header->gid      , &buffer[116], 8  );
    strncpy(header->size     , &buffer[124], 12 );
    strncpy(header->mtime    , &buffer[136], 12 );
    strncpy(header->chksum   , &buffer[148], 8  );
    header->typeflag         =  buffer[156] ;
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

//cette fonction conpare deux chainnes de carractaire en ignorant le /
//  ex: "name" == "name/"" 
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
int open_tar_file(char ch[100]){
    char s[100];
    char str2[100];
    strcpy(s,ch);
    int i = strlen(strstr(s,".tar/"));
    strncpy(str2,ch,strlen(ch) - i + 4);
    str2[strlen(ch) - i + 4]='\0';
    return open(str2,O_RDONLY);
}

//Cette fonction permet de construire le chemain absolue d'un chemain "ch" donner apatir de "pwd_globale"
void get_ch_absolu(char *ch){
    char res[100];
    if(ch[0]=='/'){//si ch comance par "/" alors on a entrer un chemain absolue
        return;//pas de changement 
    }
    if(strlen(ch)>strlen(pwd)){
        strncpy(res,ch,strlen(pwd));
        res[strlen(pwd)]='\0';
        if( cmp_name(pwd,res) == 0 ){
            return ;
        }
    }
    strcpy(res,pwd);
    int i=strlen(res);
    if(res[i-1]!='/'){
        res[i]  ='/';
        res[i+1] = '\0';
    }
    strcat(res,ch);
    strcpy(ch,res);
    return ; 
}

//Cette fonction permet d'enlever les "." d'un chemain donner
void remove_points(char *ch){
    if(strstr(ch,"/.") == NULL){
        return;
    }
    if(ch[strlen(ch)-1]!='/'){
        ch[strlen(ch)+1]='\0';
        ch[strlen(ch)]='/';
    }

    char str[100] , *ch1, ch2[100];
    strcpy(str,ch);
    ch[0]='\0';
    ch1 = strtok(str,"/");
    
    while (1){
        strcpy(ch2,ch1);
        ch1 = strtok (NULL, "/");
        if(ch1 == NULL){break;}
        if(  strcmp(ch2,".") !=0 ){
            strcat(ch,"/");
            strcat(ch ,ch2);
        }
    }
    strcat(ch,"/");
    if(strcmp(ch2,".")!=0)strcat(ch ,ch2);
}

//Cette fonction permet d'enlever les ".." d'un chemain donner
void remove_2points(char *ch){
    if(strstr(ch,"..") == NULL){
        return;
    }

    if(ch[strlen(ch)-1]!='/'){
        ch[strlen(ch)+1]='\0';
        ch[strlen(ch)]='/';
    } 

    char str[100] , *ch1, ch2[100];
    strcpy(str,ch);
    ch[0]='\0';
    ch1 = strtok(str,"/");
    
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


//cette fonction permet de verifier si le repertoir "dir_name" contien le fichier nomme "header_name" 
//retourn 1 si le repertoir "dir_name" contien un fichier nomme "header_name"
// sinon retourn -1
int check_dir(char header_name[100],char dir_name[100]){
    char name[100],str[100];
    int i;
    if( strlen(dir_name) > 0 ) {
        if ( dir_name[strlen(dir_name)-1]!='/' ){strcat( dir_name,"/\0");}

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
    strcat(str, (S_ISDIR(st.st_mode)) ? "d" : "-");
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
    
    if(option == 1 || option == 3 ){
        struct stat st;
        struct tm *dt;
    
        char str[100];int tail;
            str[0]='\0';
            sscanf(header->mode,"%o",&st.st_mode);
            permision(st,str);
            if(name[strlen(name)-1]=='/'){str[0]='d';}
        write(1,str,10);
        write(1,"  0  ",5);

            sscanf(header->uid,"%o",&st.st_uid);
            sscanf(header->gid,"%o",&st.st_gid);
            struct passwd *pw = getpwuid(st.st_uid);
            struct group  *gr = getgrgid(st.st_gid);
            sprintf(str,"%s %s", pw->pw_name, gr->gr_name);
        write(1,str,strlen(str));

            sscanf(header->size,"%o",&tail);
            sprintf(str,"%10d", tail);
        write(1,str,strlen(str));
        write(1,"  ",2);

            sscanf(header->mtime,"%lo",&st.st_mtime);
            dt=localtime(&st.st_mtime);
            strcpy(str,asctime(dt));
            str[strlen(str)-1]='\0';
        write(1,str,strlen(str));
        write(1,"  ",2);
    }
    write(1,name,strlen(name));
    write(1,"\n",2);
    return;
}

//Cette procedure est responsable de la comande "ls" dans les tar
void ls_in_tar(int fd,char dir_name[100],int option,char ch[100]){
    posix_header *header = malloc(sizeof(posix_header));
    if( strlen(dir_name)== 0 ){option+=2;}
    if( (strlen(dir_name) > 0) && (get_dir(fd,dir_name,header) == 0)){
        write(1,"ls: ",5);
        write(1,ch,strlen(ch));
        write(1,": No such file or directory\n",29);
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
void write_out_tar(struct dirent *entry,char dir_name[100],int option){
    if(option == 1){
        char str1[100];
        struct stat st;
        struct tm *dt;
        if(dir_name[strlen(dir_name)-1]!='/'){strcat(dir_name,"/");}

        strcpy(str1,dir_name);
        strcat(str1,entry->d_name);
        if(stat(str1,&st)==-1){
            if(lstat(str1,&st)==-1){
                int fd=open(str1,O_RDONLY);
                if(fstat(fd,&st)!=1){
                    close(fd);
                }else{
                    perror(str1);
                    return;
                }
            }
        }
        char str[100];
            str[0]='\0';
            permision(st,str);
        write(1,str,10);
        write(1," ",2);

            sprintf(str,"%2ld",st.st_nlink);
        write(1,str,2);
        write(1,"  ",2);

            struct passwd *pw = getpwuid(st.st_uid);
            struct group  *gr = getgrgid(st.st_gid);
            sprintf(str,"%s %s", pw->pw_name, gr->gr_name);
        write(1,str,strlen(str));

            sprintf(str,"%10ld",st.st_size);
        write(1,str,strlen(str));
        write(1,"  ",2);

            dt=localtime(&st.st_mtime);
            strcpy(str,asctime(dt));
            str[strlen(str)-1]='\0';
        write(1,str,strlen(str));
        write(1,"  ",2);

    }        
    write(1,entry->d_name,strlen(entry->d_name));
    write(1,"\n",2);
    
}

//Cette procedure est responsable de la comande "ls" out tar
void ls_out_tar(char dir_name[100],char ch[100],int option){
    struct stat st;
    char str[100];
    if(stat(dir_name, &st)==-1){//tester si le fichier/repertoir existe 
        perror(ch);
        return;
    }
    if(S_ISDIR(st.st_mode)){//test si "dir_name" est un repertoir
        DIR *dirp = opendir(dir_name);//ouvrir le repertoir 
        struct dirent *entry = malloc(sizeof(entry));
        while((entry=readdir(dirp))){//ecrire tous les fichiers et repertoirs de "dir_name"
            if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){ //ignorer '.' et '..' 
                write_out_tar(entry,dir_name,option);
            }
        }
        closedir(dirp);
    }else {// si dir_name n'est pas un repertoir
        write(1,dir_name,strlen(dir_name));
        write(1,"\n",2);
    }
    
}

//Cette procedure est responsable de la comande "ls" 
void ls(char ch[100],int option){
    int fd;int i;
    struct stat sb,sp;    
    char str[100],str2[100];
    strcpy(str,ch);
    get_ch_absolu(str);
    remove_points (str);
    remove_2points(str);
    if(strstr(str,".tar")!=NULL  ){
        if(strlen(strstr(str,".tar")) == 4 || strlen(strstr(str,".tar/")) == 5){
            if(strlen(strstr(str,".tar")) != 4){
                str[strlen(str)-1] = '\0';
            }
            fd = open(str,O_RDONLY);
            if(fstat(fd, &sb)==-1){
                perror(ch);
                return ;
            }
            ls_in_tar(fd,"",option,ch);
            return;
        }
    }
    if(in_tar == 0 || strstr(str,".tar/")==NULL){
        if(strstr(str,".tar/")==NULL){
            ls_out_tar(str,ch,option);
            return;
        }
    }
    fd = open_tar_file(str);
    if(fstat(fd, &sb)==-1){
        perror(ch);
        return ;
    }
    i = strlen(strstr(str,".tar/"));
    strncpy(str2 , &str[strlen(str) - i + 5] , i);
    str2[i]='\0';
    ls_in_tar(fd,str2,option,ch);
    close(fd);
    
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
                write(1,"cat: ",5);
                write(1,file_name,strlen(file_name));
                write(1,": Is a directory\n",18);
                lseek(fd,-(cpt*BLOCKSIZE),SEEK_CUR);
                return 0;
            }
            return cpt;
        }else{
            free(header);
            header = malloc(sizeof(posix_header));                
            i = get_file_size(header);
            lseek(fd,i*BLOCKSIZE,SEEK_CUR);cpt+=i;
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
            write(1,Buffer,BLOCKSIZE);
        }
        lseek(fd,-(cpt*BLOCKSIZE),SEEK_CUR);
        free(header);
    }else{ 
        write(1,"cat: ",5);
        write(1,ch,strlen(ch));
        write(1,": No such file or directory\n",29);
    }
}

//Cette procedure est responsable de la comande "cat" out tar
void cat_out_tar(char file_name[100],char ch[100]){
    int i;
    struct stat sb;    
    int fd = open(file_name,O_RDONLY);

    if(fstat(fd, &sb)==-1){
            perror(ch);
            return ;
    }     
    while ((i=read(fd, Buffer,sizeof(Buffer)))>0)
    {
        write(1,Buffer,i);
    }
    close(fd);
    return ;
}

//Cette procedure est responsable de la comande "cat" 
void cat(char ch[100]){
    int fd;int i;
    struct stat sb,sp;    
    char str[100],str2[100];
    strcpy(str,ch);
    get_ch_absolu(str);
    remove_points(str);
    remove_2points(str);
    if(in_tar == 0 || strstr(str,".tar/")==NULL){
        if(strstr(str,".tar/")==NULL){
            cat_out_tar(str,ch);
            return;
        }
    }
    fd = open_tar_file(str);
    if(fstat(fd, &sb)==-1){
        perror(ch);
        return ;
    }
    i = strlen(strstr(str,".tar/"));
    strncpy(str2 , &str[strlen(str) - i + 5] , i);
    str2[i]='\0';
    cat_in_tar(fd,str2,ch);
    close(fd);
    
}


/**/
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 0;
    char ch[100];
    strcpy(ch,"my project/tata/../tata/");/// a
    //strcpy(ch,"/home/islam/Desktop/my project/do_not_existe_dir");/// b

    strcpy(pwd,"/home/islam/Desktop/");
    ls(ch,0);
    
    //write(1,strstr("hi/../hello/..",".."),11);
    //remove_point(ch);
    //write(1,ch,strlen(ch));
    return 0;
}
