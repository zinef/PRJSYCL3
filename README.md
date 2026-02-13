## Un `Shell` pour les `tarballs` (*.tar)

Le but de ce projet est de développer un <ins>`Shell`</ins> permettant à l’utilisateur de manipuler des <ins>`tarballs`</ins> comme s’il s’agissait de répertoires, **sans que les archives ne soient désarchivées**.

Mais qu’est-ce qu’un Shell ? Et qu’est-ce qu’une tarball ?

<ins>**Définitions :**</ins>

<ins>**Shell :**</ins>
Un Shell (ou interface système) est un programme qui interprète les commandes saisies par l’utilisateur au clavier et les transmet au système d’exploitation, qui se charge ensuite de les exécuter.

<ins>**Tarballs :**</ins>
Le programme `tar` (de l’anglais *tape archiver*, littéralement « archiveur pour bande ») est un outil standard d’archivage de fichiers sur les systèmes de type UNIX. Il a été introduit dans les premières versions d’UNIX et standardisé par POSIX. Il existe plusieurs implémentations de `tar`, la plus courante étant GNU tar.

Nota : pour plus de détails, voir `ARCHITECTURE.md`.

---

## Installation et exécution

1. Cloner le dépôt distant sur votre machine à l’aide de la commande :
   `git clone https://gaufre.informatique.univ-paris-diderot.fr/fodil/prjsycl3.git`
   (ou via SSH).

2. Se placer dans le répertoire `prjsycl3` créé après le clonage.

3. Pour exécuter le projet directement sur votre machine, taper les deux commandes suivantes :
   `make compiler`
   puis, après la compilation :
   `make exec`

4. Il est également possible d’exécuter l’application dans un conteneur Docker (à condition que Docker soit installé sur votre machine).
   Pour cela :

   * Créer l’image Docker avec : `make image`
   * Lancer le conteneur avec : `make run`

   Une fois le conteneur lancé, se placer dans le répertoire `/home/projetSycL3` puis exécuter le Shell `zfi` comme indiqué précédemment.

---

## Makefile

Le `Makefile` contient les commandes nécessaires à la compilation et à l’exécution du Shell, ainsi que celles permettant de créer et lancer l’image Docker à partir du `Dockerfile` situé dans le même répertoire.

### Commandes disponibles :

* Pour compiler :
  `make compiler`
  Cette opération nécessite l’installation de la bibliothèque C `readline`.
  Si vous utilisez le conteneur Docker, cette bibliothèque est normalement déjà installée.

* Pour exécuter :
  `make exec`
  Il est également possible d’installer le programme dans `/usr/bin` avec :
  `make command`
  puis de le lancer simplement avec la commande :
  `zfi`

* Pour créer l’image Docker :
  `make image`
  Cette opération nécessite l’installation de Docker Engine.

* Pour lancer le conteneur :
  `make run`

* Pour déboguer le programme :
  `make debug`

---

Si tu veux, je peux aussi te proposer une version encore plus propre et orientée "projet GitHub public" avec un ton plus professionnel et structuré.


