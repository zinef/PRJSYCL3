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
        if(check_dir(header->name) == 0 ){
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
        }else{
            free(header);
            lseek(fd,-(cpt*BLOCKSIZE),SEEK_CUR);
            write(1,"cat: ",5);
            write(1,file_name,strlen(file_name));
            write(1,": No such file or directory\n",29);
            return 0;
        }
    }
    free(header);
    lseek(fd,-(cpt*BLOCKSIZE),SEEK_CUR);
    write(1,"cat: ",5);
    write(1,file_name,strlen(file_name));
    write(1,": No such file or directory\n",29);
    return 0;
}

void cat_out_tar(char file_name[100]){
    int i;
    struct stat sb;    
    
    if(stat(file_name, &sb)==-1){
        perror("stat");
        return;
    }
    if(S_ISDIR(sb.st_mode)){
        write(1,"cat: ",5);
        write(1,file_name,strlen(file_name));
        write(1,": Is a directory\n",18);
    }else{
        int fd = open(file_name,O_RDONLY);
        while ((i=read(fd, Buffer,sizeof(Buffer)))>0)
        {
            write(1,Buffer,i);
        }
        close(fd);
        return ;
    }
 
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
    }
    free(header);
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
    return open(str2,O_RDONLY);

}
///
void cat(int fd_tar,char ch[100]){//je surprime fd_tar
    int fd;int i;
    char str[100];
    strcpy(str,ch);
    if(in_tar == 0){
        if(strstr(str,".tar/")==NULL){
            cat_out_tar(str);
        }else{
            fd = open_tar_file(str);//if != 0 else faux chaine []
            i = strlen(strstr(ch,".tar/"));
            strncpy(str , &ch[strlen(ch) - i + 5] , i);
            cat_in_tar(fd,str);
            close(fd);
        }
    }else{//je vais pwd et open_tar_file ....
        //fd = open_tar_file(str);//if != 0 else faux chaine []
        i = strlen(strstr(ch,".tar/"));
        if( i != 0){strncpy(str , &ch[strlen(ch) - i + 5] , i);}
        else{strcpy(str,ch);}
        cat_in_tar(fd_tar,str);
        //close(fd);
    }
}

int main(int argc, char *argv[]){
    posix_header *header;
    int size;
    in_tar = 0;
    char ch[100];
    strcpy(ch,"../my project/../my project/test.tar/tata");
    int fd = open("./test.tar",O_RDONLY);
    strcpy(pwd,"tete/");
    cat(fd,ch);
    close(fd);
    
    return 0;
}



