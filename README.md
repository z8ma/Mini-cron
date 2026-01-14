# Projet Mini Cron
# Erraid & Tadmor - Planificateur de tâches (Projet Système)

Ce projet implémente un système complet de planification de tâches composé de deux entités :
- **`erraid`** : Le démon qui tourne en tâche de fond, gère la persistance des tâches et leur exécution.
- **`tadmor`** : Le client en ligne de commande permettant à l'utilisateur d'interagir avec le démon.

## Auteurs

CHOUX Julien
MZIOUID Souhaile
SOKOU Leny

## Compilation

Un `Makefile` est fourni pour automatiser la compilation.

- **Compiler le projet :**
 
  - Compilation  d'erraid, tadmor et de tous les fichiers objet : `make`

  - Compilation d'erraid et de tous les fichiers objet : `make erraid`

  - Compilation tadmor et de tous les fichiers objet : `make tadmor`

  -  Compilation des fichiers objet : `make build`

  -  Compilation d'un fichier objet précis fic : `make build/fic.o`

  - Nettoyer les fichiers de build/ de compilation : `make clean`

  - Nettoyage complet (incluant les exécutables) : `make distclean`

- **Utilisation :**

    1. **Lancer le démon (Erraid)** :
    ./erraid

    - Options :

      - `-F` : exécution en avant-plan sans démonisation
 
      - `-R RUN_DIR` : définition du répertoire de stockage (par défaut `/tmp/$USER/erraid`)

      - `-P PIPES_DIR` : définition du répertoire contenant les tubes de
        communication avec le client (par défaut le sous-répertoire `pipes` de
        `RUN_DIR`)


    2. **Utiliser le client (Tadmor)** :

    - Gestion et Consultation :

      - Lister les tâches :
      ./tadmor `-l`

      - Supprimer une tâche :
      ./tadmor `-r <TASKID>`

      - Affiche les dates et codes de retour :
      ./tadmor `-x <TASKID>`

      - Sorties standard/erreur (dernière exécution) :
      ./tadmor `-o <TASKID>`
      ./tadmor `-e <TASKID>`

    - Création de tâches (-c) :
    Syntaxe : ./tadmor `-c [-m Minutes] [-H Heures] [-d Jours] Commande [args]`

        - Séquence (-s) : Exécuter des tâches séquentiellement.
        ./tadmor `-s <TASKID1> <TASKID2> <TASKID3> ... <TASKIDN>`

        - Pipeline (-p) : Connecter la sortie d'une tâche à l'entrée de la suivante.         
          ./tadmor `-p [Option de timing] <TASKID1> <TASKID2> <TASKID3> ... <TASKIDN>`

        - Conditionnelle (-i) : if ID1 then ID2 [else ID3].
        ./tadmor `-i <TASKID1> <TASKID2> <TASKID3>`

    3. **Arret du démon :** 
    ./tadmor `-q`
