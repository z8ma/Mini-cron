# Architecture du Projet Erraid / Tadmor

## 1. Vue d'ensemble
Le projet implémente un planificateur de tâches composé de deux processus distincts communiquant via des tubes nommés :
- **Erraid (Démon)** : Gère le stockage, la planification et l'exécution des tâches.
- **Tadmor (Client)** : Interface utilisateur permettant d'envoyer des requêtes au démon.

## 2. Choix de Conception

### Communication Inter-Processus
La communication repose sur deux tubes nommés situés par défaut dans `/tmp/$USER/erraid/pipes/` :
- `erraid-request-pipe` : Envoi des commandes (Client -> Démon).
- `erraid-reply-pipe` : Réception des réponses (Démon -> Client).

Le protocole respecte strictement le format binaire défini (Opcode, champs de longueur, données). Toutes les données entières échangées sont converties en **Big-Endian** (`htobeXX` / `beXXtoh`) pour garantir la portabilité et le respect du sujet.

### Persistance et Stockage
L'arborescence des tâches est reproduite sur le système de fichiers (par défaut dans `/tmp/$USER/erraid/tasks/`).
- Chaque tâche possède son propre répertoire nommé par son `TASKID`.
- **Sérialisation :** Les structures (`timing`, `command`) ne sont pas écrites telles quelles (via `fwrite` de la struct) pour éviter les problèmes de *padding* mémoire. Chaque champ est écrit individuellement après conversion endian.
- **Récursion :** Les commandes complexes (Séquences, Pipes, If) sont gérées récursivement grâce à une structure arborescente dans le code (`struct command` contenant un tableau de sous-commandes) et sur le disque (sous-dossiers `cmd/0`, `cmd/1`, etc.).

### Gestion du Temps et Exécution
- **Boucle principale :** Le démon utilise `select()` avec un timeout calculé dynamiquement pour se réveiller à la 59ème seconde de chaque minute. Cela permet une précision à la minute près sans attente active (polling).
- **Double Fork (`fork_detached`) :**
    Pour chaque tâche à exécuter, le démon effectue un double fork.
    1. Le père forke un fils.
    2. Le fils forke un petit-fils et termine immédiatement (`_exit(0)`).
    3. Le petit-fils exécute la tâche.
    *Avantage :* Le petit-fils est reparenté au processus `init` (PID 1), ce qui évite la création de processus zombies que le démon devrait nettoyer manuellement avec `wait`.

### Gestion des Signaux et Tubes
- L'ouverture des tubes côté client (`tadmor`) est bloquante pour assurer la synchronisation avec le démon.
- Le démon maintient les tubes ouverts pour éviter les signaux `SIGPIPE` ou les EOF intempestifs.

## 3. Organisation Modulaire
Le code est découpé selon les entités du protocole pour faciliter la maintenance :
- `communication.c/h` : Orchestration haut niveau des échanges.
- `request.c/h` & `reply.c/h` : Formatage et parsing des messages.
- `task.c/h` : Manipulation de l'entité tâche (création dossier, lecture).
- `command.c/h` : Logique des commandes (simples et complexes) et exécution (`execvp`, `dup2`).
- `timing.c/h` : Gestion des horaires (parsing crontab et vérification `is_it_time`).
- `string_uint.c/h` : Utilitaires bas niveau (gestion chaînes, conversion).