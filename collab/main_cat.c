#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

int check_dir(char header_name[100]){
    char name[100];
    if(in_tar == 1){
        strncpy(name,header_name,strlen(pwd));
        name[strlen(pwd)]='\0';
        return strcmp(pwd,name);
    }return 0;
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

int get_file(int fd,char file_name[100], posix_header *header){
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
    lseek(fd,-(cpt*BLOCKSIZE),SEEK_CUR);
    write(1,"cat: ",5);
    write(1,file_name,strlen(file_name));
    write(1,": No such file or directory\n",29);
    return 0;
}

void cat_out_tar(char file_name[100],char ch[100]){
    int i;
    struct stat sb;    
    int fd = open(file_name,O_RDONLY);//printf("file_name : %s ",file_name);

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

void cat_in_tar(int fd,char file_name[100]){
    posix_header *header = malloc(sizeof(posix_header));
    int cpt = get_file(fd,file_name,header);
    if(cpt != 0){
        int i = get_file_size(header);
        cpt += i;
        for(i;i>0;i--){
            read(fd,Buffer,BLOCKSIZE);
            write(1,Buffer,BLOCKSIZE);
        }
        lseek(fd,-(cpt*BLOCKSIZE),SEEK_CUR);
        free(header);
    }
   
}

int check_if_in_tar(char ch[100]){
    char str[100];
    strcpy(str,ch);
    if(strstr(str,".tar/")==NULL){
        return 0;
    }
    return 1;
}

int open_tar_file(char ch[100]){
    char s[100];
    char str2[100];
    strcpy(s,ch);
    int i = strlen(strstr(s,".tar/"));
    strncpy(str2,ch,strlen(ch) - i + 4);
    str2[strlen(ch) - i + 4]='\0';
    
    return open(str2,O_RDONLY);

}

void get_ch_absolu(char *ch){
    char res[100];
    if(ch[0]=='/'){
        return;
    }
    if(strlen(ch)>strlen(pwd)){
        strncpy(res,ch,strlen(pwd));
        res[strlen(pwd)]='\0';
        if( strcmp(pwd,res) == 0 ){
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

void cat(char ch[100]){
    int fd;int i;
    struct stat sb,sp;    
    char str[100],str2[100];
    strcpy(str,ch);
    get_ch_absolu(str);
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
    cat_in_tar(fd,str2);
    close(fd);
    
}
/// test a : le fichier existe  /////////////////////////
/// test b : le fichier n'existe pas  ///////////////////
///test 1 : 
//  a / cat sur un fichier qui n est pas dans tar avec un chemain absolu
//  b / cat sur un fichier qui n est pas dans tar avec un chemain absolu sur un fichier n existe pas
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 0;
    char ch[100];
    //strcpy(ch,"/home/islam/Desktop/my project/main_cat.c");/// a
    strcpy(ch,"/home/islam/Desktop/my project/do_not_existe_file.c");/// b

    strcpy(pwd,"/home/islam/Desktop/tp");
    cat(ch);
    
    return 0;
}
*/
///test 2 : 
//  a / cat sur un fichier qui n est pas dans tar avec un chemain relative
//  b / cat sur un fichier qui n est pas dans tar avec un chemain relative faux
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 0;
    char ch[100];
    strcpy(ch,"my project/main_cat.c");/// a
    //strcpy(ch,"/Desktop/my project/main_cat.c"); /// b

    strcpy(pwd,"/home/islam/Desktop");
    cat(ch);
    
    return 0;
}
*/
///test 3 : 
//  a / cat sur un fichier qui est dans tar avec un chemain abslu
//  b / cat sur un fichier qui est dans tar avec un chemain abslu sur un fichier qui existe pas
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 0;
    char ch[100];
    strcpy(ch,"/home/islam/Desktop/my project/test.tar/tata/tatadpll (1) (copy).ml");/// a
    //strcpy(ch,"/home/islam/Desktop/my project/test.tar/titi/titidpll(1) (another copy).ml");/// a
    //strcpy(ch,"/home/islam/Desktop/my project/test.tar/tata/do_not_existe_file.c"); /// b
    //strcpy(ch,"/home/islam/Desktop/my project/test.tar/titi/do_not_existe_file.c"); ///b
    
    strcpy(pwd,"/home/islam/Desktop");
    cat(ch);
    
    return 0;
}
*/
///test 4 : 
//  a / cat sur un fichier qui est dans tar avec un chemain relative dans un tar
//  b / cat sur un fichier qui est dans tar avec un faux chemain relative dans un tar
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 1;
    char ch[100];
    strcpy(ch,"tatadpll (1) (copy).ml");/// a1
    //strcpy(ch,"titidpll(1) (another copy).ml");/// a2
    
    //strcpy(ch,"titidpll(1) (another copy).ml");/// b1
    //strcpy(ch,"tatadpll (1) (copy).ml");/// b2
    

    strcpy(pwd,"/home/islam/Desktop/my project/test.tar/tata");/// a1 et b1
    //strcpy(pwd,"/home/islam/Desktop/my project/test.tar/titi");/// a2 et b2
    cat(ch);
    
    return 0;
}
*/
///test 5 : 
///  a / cat sur un fichier qui n' est pas dans tar avec un chemain absolu et le docier acctuel et dans tar
///  b / cat sur un fichier qui n' est pas dans tar avec un faux chemain absolu et le docier acctuel et dans tar
/*
int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 1;
    char ch[100];
    strcpy(ch,"/home/islam/Desktop/my project/main_cat.c");/// a
    //strcpy(ch,"/home/islam/Desktop/my project/do_not_existe_file.c");/// b

    strcpy(pwd,"/home/islam/Desktop/my project/test.tar/tata");
    cat(ch);
    
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
    strcpy(ch,"/home/islam/Desktop/my project/test2.tar/tata/tatadpll (1) (another copy).ml");
    
    strcpy(pwd,"/home/islam/Desktop/my project/test.tar/tata");
    cat(ch);
    
    return 0;
}
*/

/******************************************************************************************************/
///pour le cas d utulisation des .. / pas encore traiter ...