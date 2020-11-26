#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

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
//pas de modification par raport a cat
int get_file_size(posix_header *header){
    int size;
    sscanf(header->size,"%o",&size);
    return (size + 512 - 1) / 512;
}

void seek_n_block(int fd,int nb_block){
    lseek(fd,nb_block*BLOCKSIZE,SEEK_CUR);
}

void seek_next_entete(int fd, posix_header *header){
    seek_n_block(fd,get_file_size(header));
}

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
//jusqu'a ici


// check_dir est useless parrapor a cat
// 1 print header_name
// 0 j ai lu tt le contenu du fichier
// -1 sauter ce fichier
int check_dir(char header_name[100],char dir_name[100]){
    char name[100],str[100];
    int i;
    if(dir_name[strlen(dir_name)-1]!='/'){
        dir_name[strlen(dir_name)+1] ='\0';
        dir_name[strlen(dir_name)]   ='/';
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
        }else{ 
            return -1;
        }
    }
    return 0;
    
}

int get_dir(int fd,char dir_name[100], posix_header *header){
    /////////////////////////////////////ca marche 
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
            lseek(fd,i*BLOCKSIZE,SEEK_CUR);cpt+=i;//seek_n_block(fd,i);//meme que
            read(fd,Buffer,BLOCKSIZE);cpt++;
        }
    }
    free(header);
    lseek(fd,-(cpt*BLOCKSIZE),SEEK_CUR);
    write(1,"cat: ",5);
    write(1,dir_name,strlen(dir_name));
    write(1,": No such file or directory\n",29);
    return 0;
}

void write_in_tar(posix_header *header,int option){
    char name[100];
    int i = strlen(strstr(header->name,"/"));
    strncpy(name,&header->name[strlen(header->name) - i + 1],i-1);
    name[i] = '\0';
    if(option==0){
        write(1,name,strlen(name));// write file or dir name
        write(1,"\n",2);
    }else{
        write(1,name,strlen(name));// write file or dir name
        write(1,"\n",2);
    }
}

void ls_in_tar(int fd,char dir_name[100],int option){
    //////////////////////////////////////////////////////////////////
    posix_header *header = malloc(sizeof(posix_header));
    if( get_dir(fd,dir_name,header) == 0){
        return;
    }
    if( header->typeflag == '0'){
        write_in_tar(header,option);
        return;
    }
    int i;
    while ( 1 ){

        free( header );
        header = malloc(sizeof(posix_header));
        read(fd,Buffer,BLOCKSIZE);
        get_entete_info(header,Buffer);

        i = check_dir(header->name,dir_name);
        if( i == 1 ){
            write_in_tar(header,option);
            seek_n_block(fd,get_file_size(header)); // seek to the next header
        }
        if( i == -1){
            seek_n_block(fd,get_file_size(header)); // seek to the next header    
        }
        if( i == 0 ){
            break;
        }
    }
   
}
//if option == 1 ==> ecrire avec option -l
void write_out_tar(struct dirent *entry, int option){
    if(option == 0){
        write(1,entry->d_name,strlen(entry->d_name));
        write(1,"\n",2); 
    }else{
        write(1,entry->d_name,strlen(entry->d_name));
        write(1,"\n",2);
    }

}

void ls_out_tar(char dir_name[100],char ch[100],int option){
///////////////////////////////////////////////////////////////////////////////////////
    struct stat st;
    if(stat(dir_name, &st)==-1){
        perror(ch);
        return;
    }
    if(S_ISDIR(st.st_mode)){
        DIR *dirp = opendir(dir_name);
        struct dirent *entry = malloc(sizeof(entry));
        while((entry=readdir(dirp))){
            if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
                write_out_tar(entry,option);
            }
        }
        closedir(dirp);
    }else {
        write(1,dir_name,strlen(dir_name));
        write(1,"\n",2);
    }
    
}
///pas de modification
int open_tar_file(char ch[100]){
    char s[100];
    char str2[100];
    strcpy(s,ch);
    int i = strlen(strstr(s,".tar/"));
    strncpy(str2,ch,strlen(ch) - i + 4);
    str2[strlen(ch) - i + 4]='\0';
    
    return open(str2,O_RDONLY);

}
/// pas de modification
void get_ch_absolu(char *ch){
    char res[100];
    if(ch[0]=='/'){
        return;
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

void ls(char ch[100],int option){
    int fd;int i;
    struct stat sb,sp;    
    char str[100],str2[100];
    strcpy(str,ch);
    get_ch_absolu(str);
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
    ls_in_tar(fd,str2,option);
    close(fd);
    
}

/// test a : le fichier existe  /////////////////////////
/// test b : le fichier n'existe pas  ///////////////////
///test 1 : 
//  a / cat sur un dir qui n est pas dans tar avec un chemain absolu
//  b / cat sur un dir qui n est pas dans tar avec un chemain absolu sur un dir n existe pas
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 0;
    char ch[100];
    strcpy(ch,"/home/islam/Desktop/my project/tata");/// a
    //strcpy(ch,"/home/islam/Desktop/my project/do_not_existe_dir");/// b

    strcpy(pwd,"/home/islam/Desktop/tp");
    ls(ch,0);
    
    return 0;
}
*/
///test 2 : 
//  a / cat sur un dir qui n est pas dans tar avec un chemain relative
//  b / cat sur un dir qui n est pas dans tar avec un chemain relative faux
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 0;
    char ch[100];
    strcpy(ch,"my project/main_cat.c");/// a
    //strcpy(ch,"/Desktop/my project/main_cat.c"); /// b

    strcpy(pwd,"/home/islam/Desktop");
    ls(ch);
    
    return 0;
}
*/
///test 3 : 
//  a / cat sur un dir qui est dans tar avec un chemain abslu
//  b / cat sur un dir qui est dans tar avec un chemain abslu sur un dir qui existe pas
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 0;
    char ch[100];
    strcpy(ch,"/home/islam/Desktop/my project/test.tar/tata/");/// a
    //strcpy(ch,"/home/islam/Desktop/my project/test.tar/titi/");/// a
    //strcpy(ch,"/home/islam/Desktop/my project/test.tar/tata/do_not_existe_dir"); /// b
    //strcpy(ch,"/home/islam/Desktop/my project/test.tar/titi/do_not_existe_file.c"); ///b
    
    strcpy(pwd,"/home/islam/Desktop");
    ls(ch);
    
    return 0;
}
*/
///test 4 : 
//  a / cat sur un dir qui est dans tar avec un chemain relative dans un tar
//  b / cat sur un dir qui est dans tar avec un faux chemain relative dans un tar
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 1;
    char ch[100];
    //strcpy(ch,"tatadpll (1) (copy).ml");/// a1
    strcpy(ch,"toto/");/// a2
    
    //strcpy(ch,"titidpll(1) (another copy).ml");/// b1
    //strcpy(ch,"tatadpll (1) (copy).ml");/// b2
    

    //strcpy(pwd,"/home/islam/Desktop/my project/test.tar/tata");/// a1 et b1
    strcpy(pwd,"/home/islam/Desktop/my project/test.tar");/// a2 et b2
    ls(ch);
    
    return 0;
}
*/
///test 5 : 
///  a / cat sur un dir qui n' est pas dans tar avec un chemain absolu et le docier acctuel et dans tar
///  b / cat sur un dir qui n' est pas dans tar avec un faux chemain absolu et le docier acctuel et dans tar
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 1;
    char ch[100];
    strcpy(ch,"/home/islam/Desktop/my project/tata");/// a
    //strcpy(ch,"/home/islam/Desktop/my project/do_not_existe_file.c");/// b

    strcpy(pwd,"/home/islam/Desktop/my project/test.tar/tata");
    ls(ch);
    
    return 0;
}
*/

///test 6 : 
// a / cat sur un fichier qui est dans tar avec un chemain absolu out un tar
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 1;
    char ch[100];
    strcpy(ch,"/home/islam/Desktop/my project/test2.tar/tata/");
    
    strcpy(pwd,"/home/islam/Desktop/my project/test.tar/tata");
    ls(ch,0);
    
    return 0;
}
*/

/******************************************************************************************************/
///pour le cas d utulisation des .. / pas encore traiter ...