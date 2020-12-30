## un `Shell` pour les `tarballs` (*.tar) 
Le but du projet est de faire tourner un <ins>`Shell`</ins> qui permet à l'utilisateur de traiter les <ins>`tarballs`</ins> comme s'il s'agissait de répertoires, **sans que les tarballs ne soient désarchivés**.
Mais ca consiste en quoi un Shell ? et les tarballs ?

<ins>**Définitions :**</ins>  
<ins>**Shell :**</ins> Le Shell (ou interface système en français) est un programme qui reçoit des commandes informatiques données par un utilisateur à partir de son clavier pour les envoyer au système d’exploitation qui se chargera de les exécuter.  

<ins>**Tarballs :**</ins> Le programme tar (de l'anglais tape archiver, littéralement « archiveur pour bande ») est un logiciel d'archivage de fichiers standard des systèmes de type UNIX. Il a été créé dans les premières versions d'UNIX et standardisé par les normes POSIX.1-1988 puis POSIX.1-2001. Il existe plusieurs implémentations1 tar, la plus couramment utilisée étant GNU tar. (Wikipédia)
`Nota : pour plus de détails voir ` ARCHITECTURE.md 
## Installation et exécution 
1. Récupérer ce dépôt distant dans un dépôt local (sa machine) à l'aide de la commande `git clone https://gaufre.informatique.univ-paris-diderot.fr/fodil/prjsycl3.git ` (ou même avec SSH)  
2. Ce placer dans le répertoire `prjsycl3` crée par le clonage .
3. Si on veut faire une exécution directement sur sa machine , l'utilisateur doit taper les deux commandes suivante `make compiler` suivi après compilation par un `make exec` .
4. Sinon il existe une possibilité d'exécuter l'application dans un conteneur docker (si l'utilisateur à bien installé docker dans sa machine) , en créant une image docker en tapant juste la commande `make image` et puis après création de l'image on tape `make run ` . après que le conteneur est lancé automatiquement l'utilisateur peut utiliser l'instruction 3. pour exécuter le shell `zfi` de la même manière en se plaçant dans le répertoire `/home/projetSycL3` .  
Pour le fichier `makefile` , c'est un fichier qui comporte essentiellement les commandes de compilation et d'exécution du Shell ainsi la commande de création et lancement de l'image docker à partir du `Dockerfile` qui se trouve dans le même répertoire  
+ Pour compiler : `make compiler` (cette opération nécessite l'installation de la la bibliothèque C _readline_ , mais si on l’exécute dans le conteneur  docker cette bibliothèque sera installée à priori).
+ Pour exécuter : `make exec` , ou bien on peut ajouter ce programme dans /usr/bin avec `make command` et puis on le lance avec la commande `zfi` seulement
+ Pour lancer la création de l'image docker : `make image` (cette opération nécessite l'installation du docker _engine_ sur la machine)
+ Pour lancer le conteneur il suffit de taper : `make run` 
+ Il comporte aussi une option pour débuger le programme : `make debug` 

