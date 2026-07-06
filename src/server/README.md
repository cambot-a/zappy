# zappy_server

Serveur du projet Zappy : il écoute en TCP, accepte les clients IA et
graphiques, et arbitre la partie.

## Usage

```sh
./zappy_server -p port -x width -y height -n name1 name2 ... -c clientsNb -f freq
```

Tous les arguments sont **facultatifs** : une valeur par défaut est utilisée
quand un flag est omis. `./zappy_server` sans aucun argument démarre donc une
partie jouable.

## Arguments

| Flag | Obligatoire | Description | Contraintes | Défaut |
|------|-------------|-------------|-------------|--------|
| `-p port`      | non | Port d'écoute TCP | 1 à 65535 | `4242` |
| `-x width`     | non | Largeur du monde en tuiles | > 0 | `10` |
| `-y height`    | non | Hauteur du monde en tuiles | > 0 | `10` |
| `-n name1 ...` | non | Noms des équipes (un ou plusieurs) | uniques, `GRAPHIC` est réservé | `team1 team2` |
| `-c clientsNb` | non | Nombre de clients autorisés par équipe | > 0 | `5` |
| `-f freq`      | non | Inverse de l'unité de temps des actions | > 0 | `100` |
| `--help`       | non | Affiche l'usage et quitte | | |

Options bonus (parsées, fonctionnalités en cours) :

| Flag | Description |
|------|-------------|
| `--enable-events`       | Active les événements dynamiques |
| `--enable-biomes`       | Active la génération de biomes |
| `--enable-admin`        | Active la console d'administration |
| `--admin-password <pw>` | Mot de passe de la console admin |

Un argument **invalide** (valeur hors borne, entier malformé, flag inconnu,
nom d'équipe dupliqué ou réservé) affiche l'erreur suivie de l'usage et quitte
avec le code **84**. Un argument simplement **omis** prend sa valeur par défaut.

## Gestion des erreurs

- **Au démarrage** (parsing CLI, bind du port, écoute) : une erreur est fatale,
  le serveur affiche le message et quitte avec le code **84**.
- **Au runtime** (une fois le serveur en écoute) : toute erreur levée pendant le
  traitement d'un client ou d'un timer est **loguée sur `stderr` sans arrêter le
  serveur** ; la boucle d'événements continue.

## Exemples

```sh
# Sans argument : tout par défaut (port 4242, 10x10, team1 team2, 5 clients, freq 100)
./zappy_server

# Partie minimale équivalente, explicite
./zappy_server -p 4242 -x 10 -y 10 -n team1 team2 -c 5

# Fréquence explicite
./zappy_server -p 4242 -x 20 -y 20 -n alpha beta gamma -c 3 -f 50

# Avec bonus
./zappy_server -p 4242 -x 10 -y 10 -n team1 -c 5 --enable-admin --admin-password secret

# Aide
./zappy_server --help
```

Test rapide à la main une fois le serveur lancé :

```sh
nc 127.0.0.1 4242
# le serveur envoie : WELCOME
# répondre "team1"  -> slots restants puis dimensions du monde (client IA)
# répondre "GRAPHIC" -> connexion en tant que client graphique
```

## Architecture (l'essentiel)

Le serveur est **mono-thread** et repose sur du multiplexage d'I/O non bloquant via `poll()`.
Il est découpé en trois couches :

```
src/server/   logique applicative (CLI, clients, équipes, handshake)
libs/net/     moteur réseau (libzappy_net.a)
libs/posix/   wrappers RAII des ressources POSIX (libzappy_posix.a)
```

- **`libs/posix`** : `FileDescriptor` (propriétaire RAII move-only d'un fd, fermeture automatique) et `Address` (wrapper IPv4 de `sockaddr_in`).
- **`libs/net`** :
  - `Listener` : socket TCP d'écoute non bloquante (`SO_REUSEADDR`, `accept4`) ;
  - `PollLoop` : boucle d'événements `poll()` avec callbacks par fd ;
    SIGINT arrive via `signalfd`, donc Ctrl+C est un simple événement
    et l'arrêt est propre, sans état global ;
  - `ClientBuffer` : bufferisation par client des lectures et écritures
    partielles, découpage des messages sur `\n` (cap à 1 Mo par client).
- **`src/server`** :
  - `CliParser` produit une `ServerConfig` immuable et validée ;
  - `Server` orchestre : accepte les connexions, envoie `WELCOME`,
    route les lignes reçues selon l'état du client ;
  - `Client` est une machine à états (`HANDSHAKE -> AI | GUI | GUI_ADMIN`)
    stockée dans `ClientRegistry` (indexé par fd) ;
  - `HandshakeHandler` traite la première ligne du client : `GRAPHIC`
    promeut en GUI ; un nom d'équipe valide avec un slot libre
    (suivi par `TeamRegistry`) promeut en IA et renvoie le nombre de
    slots restants puis les dimensions du monde ; tout le reste reçoit
    `ko` et la connexion est fermée.


## Tests

```sh
make tests_run   # depuis la racine du repo
```

Tests unitaires Criterion couvrant le parser CLI, la machine à états
client, le registre d'équipes, le handshake et la couche réseau.
