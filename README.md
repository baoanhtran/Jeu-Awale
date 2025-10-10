# 🎮 Jeu Awalé - Implémentation en ligne de commande (CLI)

## 🧭 Aperçu

Ce projet est une implémentation simple du jeu **Awalé** en **interface en ligne de commande (CLI)**, développée dans le cadre du cours *Programmation Réseau (Internet Programming)* par **TRAN Bao Anh**, en langage **C**.

Le jeu permet à plusieurs joueurs de se connecter et d’interagir via un modèle client-serveur.  
Il inclut des fonctionnalités telles que les **défis**, la **visibilité des parties**, un **système de discussion privée** et la **gestion des amis**.

---

## ⚙️ Fonctionnalités implémentées

- Tous les clients sont gérés dans un seul tableau `clients`, qu’ils soient connectés, déconnectés, en jeu ou en observation.  
- Les défis sont stockés dans la variable `challenges`. Cette variable est mise à jour (ajout/suppression) lorsqu’un défi est envoyé, accepté ou refusé — même principe pour les parties (`games`).  
- Chaque client est associé à une seule **adresse IP**.  
  - Si un client A joue sous le nom *Frank*, se déconnecte, et qu’un client B tente de se reconnecter sous le même nom mais depuis une autre IP, la connexion est refusée.  
  - De même, un même client ne peut pas ouvrir deux sessions avec le même nom dans deux terminaux différents.  
  - Ce mécanisme remplace l’usage de mots de passe pour plus de simplicité.  
- Protection contre les comportements malveillants : lorsqu’un client envoie une demande de défi, un **délai de 30 secondes** est imposé avant qu’il puisse en renvoyer une autre.  
- En jeu :  
  - Chaque joueur peut se **déconnecter au maximum deux fois**.  
  - Passé ce nombre, l’adversaire est déclaré vainqueur.  
  - Un joueur déconnecté dispose de **30 secondes** pour se reconnecter.  
- Les clients peuvent **observer** les parties publiques. Pour observer une partie privée, il faut être **ami avec au moins un des joueurs**.  
- Système de **classement Elo**.  
- **Chat privé** (chuchotement).  
- **Système d’amis**.  
- **Système de biographie** (bio).  

> 💡 Le code gère de nombreux cas imprévus : usurpation d’identité, déconnexions dans différents états, défis envoyés pendant une partie, interruption par `<Ctrl-D>` / `<Ctrl-C>`, etc.  
> Avec le recul, l’usage de **pointeurs** au lieu de **copies** aurait permis de réduire la redondance et d’améliorer la flexibilité.

---

## 🚀 Installation et exécution

### 1️⃣ Télécharger le code
Clonez ou téléchargez le dépôt sur votre machine locale.

### 2️⃣ Compiler le code
Dans le répertoire du projet, exécutez :
```bash
$ make
```

### 3️⃣ Lancer le serveur
Sur la machine qui fera office de serveur, récupérez son adresse IP :
```bash
$ hostname -I
```
Puis lancez le serveur :
```bash
$ ./server
```

### 4️⃣ Lancer le client
Sur une autre machine ou un autre terminal, exécutez :
```bash
$ ./client ip-serveur pseudo
```

Remplacez `ip-serveur` par l’adresse IP obtenue à l’étape précédente, et `pseudo` par votre nom de joueur.

---

## 🕹️ Commandes du jeu

### Menu principal
Une fois connecté, vous aurez accès aux commandes suivantes :

#### 🔧 Commandes générales
- **`names`** : Affiche tous les clients connectés et leurs statuts.  
- **`menu`** : Affiche la liste de toutes les commandes disponibles.  
- **`clear`** : Nettoie le terminal.  

#### 🎯 Interaction avec le jeu
- **`challenge [pseudo]`** : Défier un autre joueur.  
  Pendant la partie :
  - **`play [numéro_case]`** : Joue les graines à partir de la case spécifiée.  
- **`games`** : Liste toutes les parties en cours.  
- **`set visibility [private/public]`** : Change la visibilité de votre partie (par défaut : **privée**).  
- **`observe [pseudo]`** : Observer la partie d’un autre joueur.  
- **`leave`** : Quitter le mode observation.  

#### 💬 Fonctions sociales
- **`chat [pseudo] [message]`** : Envoyer un message privé à un autre joueur.  
- **`set bio [description]`** : Modifier votre biographie.  
- **`bio [pseudo]`** : Consulter la biographie d’un autre joueur.  

#### 🤝 Gestion des amis
- **`make friend [pseudo]`** : Envoyer une demande d’ami.  
- **`accept fr [pseudo]`** : Accepter une demande d’ami.  
- **`deny fr [pseudo]`** : Refuser une demande d’ami.  
- **`cancel fr [pseudo]`** : Annuler une demande d’ami envoyée.  
- **`unfriend [pseudo]`** : Retirer un utilisateur de votre liste d’amis.  
- **`friends`** : Afficher votre liste d’amis.  
- **`friend requests`** : Voir vos demandes d’amis en attente.  

---

## 📝 Remarques

- Assurez-vous que le **serveur est démarré** avant la connexion des clients.  
- Utilisez des **pseudonymes uniques** pour éviter les conflits pendant les parties.  

---

## 🎉 Conclusion

Explorez toutes les fonctionnalités et amusez-vous bien à jouer à **Awalé** avec vos amis ! 🌱  