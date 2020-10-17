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
![](./Images/Diagrame.png)

## Implémentation 
+ La manipulation des fichiers se fera à l'aide des appels systèmes `open`, `close`, `read`, `write` 
+ La manipulation des répertoire se fera avec `opendir`, `readdir`, `closedir`  et `getcwd`, `chdir`, `mkdir` 
+ L'analyse peut être effectuée à l'aide de `strsep ("")`
+ Après l'analyse, on vérifie la liste des commandes intégrées et, le cas échéant, on l'exécute 
+ La détection des pipes peut également être effectuée à l'aide de strsep («|»). Pour les gérer, on sépare d'abord la première partie de la commande de la seconde. Ensuite, après avoir analysé chaque partie, on appel les deux parties dans deux nouveaux enfants séparés, en utilisant `execvp`.