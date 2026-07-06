# zappy_gui

Client graphique du projet Zappy : il se connecte au serveur en tant que
client `GRAPHIC`, récupère l'état du monde et affiche la partie en temps réel.

## Usage

```sh
./zappy_gui -p port -h machine
```

Tous les arguments sont **facultatifs** : une valeur par défaut est utilisée
quand un flag est omis. `./zappy_gui` sans aucun argument tente donc une
connexion vers `localhost:4242`.

## Arguments

| Flag         | Obligatoire | Description                         | Contraintes   | Défaut      |
| ------------ | ----------- | ----------------------------------- | ------------- | ----------- |
| `-p port`    | non         | Port TCP du serveur                 | 1 à 65535     | `4242`      |
| `-h machine` | non         | Nom d'hôte ou adresse IP du serveur | chaîne valide | `localhost` |
| `--help`     | non         | Affiche l'usage et quitte           |               |             |

Un argument **invalide** (port hors borne, entier malformé, valeur manquante
ou flag inconnu) affiche l'erreur suivie de l'usage et quitte avec le code
**84**.

Un argument simplement **omis** prend sa valeur par défaut.

## Gestion des erreurs

* **Au démarrage** (parsing CLI, résolution DNS, création de socket,
  connexion au serveur) : toute erreur est fatale, un message est affiché
  puis le programme quitte avec le code **84**.
* **Au runtime** : les erreurs réseau ou protocolaires sont signalées à
  l'utilisateur. Une perte de connexion entraîne la fermeture propre du GUI.

## Exemples

```sh
# Connexion locale (localhost:4242)
./zappy_gui

# Connexion locale sur un port personnalisé
./zappy_gui -p 5000

# Connexion à un serveur distant
./zappy_gui -h 192.168.1.42 -p 4242

# Aide
./zappy_gui --help
```

## Authentification

Conformément au sujet, le GUI s'identifie auprès du serveur comme client
graphique.

Une fois connecté, le serveur envoie :

```text
WELCOME
```

Le GUI répond alors :

```text
GRAPHIC
```

afin d'être enregistré comme observateur de la partie.

Le nom d'équipe `GRAPHIC` est réservé et ne peut pas être utilisé par les
clients IA.

## Fonctionnement

Le GUI maintient une représentation locale complète du monde.

À la connexion :

1. récupération des dimensions de la carte ;
2. récupération du contenu initial des tuiles ;
3. récupération des équipes ;
4. récupération des joueurs, œufs et ressources ;
5. démarrage de la boucle graphique.

Une fois initialisé, le GUI reçoit les événements envoyés par le serveur et
met à jour uniquement les éléments concernés.

Les événements pris en charge incluent notamment :

* déplacement des joueurs ;
* rotation des joueurs ;
* connexion et déconnexion ;
* ponte et éclosion des œufs ;
* collecte et dépôt de ressources ;
* diffusion des broadcasts ;
* élévations (incantations) ;
* mise à jour du contenu des tuiles ;
* fin de partie et annonce de l'équipe gagnante.

## Représentation du monde

Le monde est affiché sous forme de grille 2D représentant les tuiles du jeu.

Chaque tuile peut contenir :

* des joueurs ;
* des œufs ;
* de la nourriture ;
* des ressources d'élévation :

  * linemate ;
  * deraumere ;
  * sibur ;
  * mendiane ;
  * phiras ;
  * thystame.

Les informations sont mises à jour en temps réel à partir des événements
reçus du serveur.

## Interface

L'interface permet notamment :

* le déplacement de la caméra ;
* le zoom avant / arrière ;
* la sélection d'un joueur ;
* l'affichage des informations détaillées d'un joueur ;
* l'affichage du niveau d'un joueur ;
* l'affichage de son inventaire ;
* l'affichage des équipes présentes ;
* l'affichage des ressources d'une tuile ;
* l'affichage des broadcasts ;
* l'affichage du vainqueur en fin de partie.

## Architecture (l'essentiel)

Le GUI est organisé autour de plusieurs composants :

```text
src/gui/
├── main.cpp
├── Gui.cpp
├── GuiArgParser.cpp
├── client/
│   ├── Client.cpp
│   └── ClientRegistry.cpp
└── ipc/
```

### `GuiArgParser`

Transforme les arguments de la ligne de commande en une configuration validée.

### `ServerInfo`

Objet immuable contenant les paramètres de connexion du serveur.

### `Gui`

Orchestrateur principal du client graphique.

Responsabilités :

* connexion au serveur ;
* authentification ;
* réception et décodage des événements ;
* synchronisation du monde ;
* gestion de la boucle graphique ;
* rendu de la carte ;
* gestion des interactions utilisateur.

### `client`

Couche réseau responsable :

* de la communication TCP ;
* du buffering ;
* de la lecture des messages ;
* de la gestion des connexions.

### `ipc`

Structures de données et mécanismes de sérialisation utilisés pour la
communication avec le serveur.

## Performances

Le GUI repose sur une architecture événementielle.

Le serveur ne renvoie que les modifications nécessaires afin de limiter le
trafic réseau :

* les changements de tuiles sont envoyés uniquement lorsqu'ils surviennent ;
* les actions des joueurs sont représentées par des événements dédiés ;
* aucune synchronisation complète du monde n'est nécessaire après
  l'initialisation.

Cette approche permet de visualiser des parties comportant un grand nombre de
joueurs tout en conservant une interface fluide.

## Compilation

Depuis la racine du projet :

```sh
make zappy_gui
```

ou :

```sh
make
```

pour compiler l'ensemble du projet.

Build de debug :

```sh
make debug
```

Nettoyage :

```sh
make clean
make fclean
```

## Tests

```sh
make tests_run
```

Les tests couvrent notamment :

* le parsing CLI ;
* la validation des paramètres ;
* la communication réseau ;
* la gestion des événements ;
* la synchronisation du monde ;
* le décodage du protocole graphique.
