# Architecture générale du Shell
## Analyse
#### Une fois la commande entrée, les opérations suivantes sont effectuées:

1. La commende est entrée , si cette commande n'est pas la chaine vide on la garde dans l'historique des commandes
2. Analyse : consiste en la division des commandes en sous chaines pour l'interpréter 
3. Analyse pour les caractères spéciaux comme : **|** **<** **>** **2>**
4. Vérification de l'existence de la commande dans le Shell i.e. une commende interprétable 
5. S'il existe des pipe , on les manipulera 
6. Exécution des  commandes en utilisant les processus avec la famille des **exec** **( execl , execv ...)** et **fork** 
7. Attendre la prochaine entrée 
## Diagramme 
![image](https://gaufre.informatique.univ-paris-diderot.fr/fodil/prjsycl3/blob/master/Images/Diagrame.png)