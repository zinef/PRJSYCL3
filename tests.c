#include "our_shell_zfi.h"
#include "our_shell_zfi.c"

int main(int argc , char **argv){
	/*	
		//test de la fonction trouverPipe	
		char *commandesSiPipe[10]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	

		char entree[100];
		strcpy(entree,"ls -l |grep aaa |echo bbb");
		int ret= trouverPipe(entree,commandesSiPipe);
		printf("%d\n",ret);
		for(int i=0 ;i<3;i++){
			if(commandesSiPipe[i] != "")
				printf("%s\n",commandesSiPipe[i]);
		}
	*/
	/*	
		//test de la fonction recupArgs
		char entree[100];
		strcpy(entree,"ls -l -a");
		char *listeArgs[4]={NULL,NULL,NULL,NULL};
		recupArgs(entree,listeArgs);
		for(int i=0 ;i<4;i++){
			if(listeArgs[i] != NULL)
				printf("%s \n",listeArgs[i]);
		}
	*/

	/*
		//test de la fonction commande valide
		char entree[100];
		strcpy(entree,"ls -la");//commande existante
		char *listeArgs[4]={NULL,NULL,NULL,NULL};
		recupArgs(entree,listeArgs);
		int ret=commandeValide(listeArgs);
		strcpy(entree,"echo hello");//commande non existante
		recupArgs(entree,listeArgs);
		ret=commandeValide(listeArgs);
	*/
	/*
		//test de la fonction decortiquer 
		char *listeArgsPipe[4]={NULL,NULL,NULL,NULL};
		char *listeArgs[4]={NULL,NULL,NULL,NULL};
		char entree[100];
		strcpy(entree,"mkdir rep1 rep2");
		int cas =decortiquerEntree(entree,listeArgs,listeArgsPipe);
		printf("le cas est le %d \n",cas);
		if (cas != 0){
			for(int i=0 ;i<4;i++){
				if(listeArgs[i] != NULL)
					printf("%s \n",listeArgs[i]);
			}
			for(int i=0 ;i<4;i++){
				if(listeArgsPipe[i] != NULL)
					printf("%s \n",listeArgsPipe[i]);
			}
		}
	*/
	/*
		//test de la fonction recup_extension
		printf("%s\n", recup_ext("test.tar"));
	*/

	/*	//test de la fonction verif_exist_rep_in_tar 
		int *entete_lu;
		int res=verif_exist_rep_in_tar(argv[1],argv[2],entete_lu);
		printf("%d ,%d\n",res,*entete_lu);
	*/
	/*
		//test de cd
		//test de verifier_exist_rep
		//in_tar=1;
		int *entete_lu=malloc(sizeof(int));
		char pwd[1024]; 
			getcwd(pwd, sizeof(pwd)); 
		printf("pwd = %s\n",pwd);
		int ret = verifier_exist_rep(argv[1],entete_lu);
		printf("val ret = %d\n",ret);
			getcwd(pwd, sizeof(pwd)); 
		printf("pwd = %s\n",pwd);
		printf("entete lu =%d\n",*entete_lu);
		printf("Cd test \n");
		my_cd_global(argv[1]);
		printf("pwd : %s \n",pwd_global);
		printf("tar actuel : %s \n",tar_actuel);
		printf("in tar = %d\n",in_tar);
		my_cd_global("Images/");
		printf("pwd : %s \n",pwd_global);
		printf("tar actuel : %s \n",tar_actuel);
		printf("in tar = %d\n",in_tar);
		my_cd_global("test.tar/Drafts/\0");
		printf("pwd : %s \n",pwd_global);
		printf("tar actuel : %s \n",tar_actuel);
		printf("in tar = %d\n",in_tar);
	*/

	//	rm("toto.tar/fic1");
	/*
		in_tar=1;
		strcpy(pwd_global,"/home/zizou/Bureau/SYCL3/prjsycl3/toto.tar/");
		int ret = my_mkdir("rep2");
	*/

	/*	in_tar=1;
		strcpy(pwd_global,"/home/zizou/Bureau/SYCL3/prjsycl3/toto.tar/");
		int ret= my_rmdir("rep2");
	*/
	/*
		//test typique pour les redirection avec un cat en stdin 
		signal(SIGINT , stop);
		std_out_copy   = dup(STDOUT_FILENO);
			std_error_copy = dup(STDERR_FILENO);
			std_in_copy    = dup(STDIN_FILENO );
		strcpy(pwd_global,getcwd(NULL,0));
		char *out_file;
		int fd=redirect_res("zizou.txt",out_file, ">");
		cat ("");
		end_redirect("zizou.txt",fd,out_file,">");*/
	return 0;
}
