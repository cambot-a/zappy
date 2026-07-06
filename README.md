<img width="1226" height="638" alt="image" src="https://github.com/user-attachments/assets/cae50f18-bd27-4506-9199-f267e19120ae" />


Jeu en réseau multi-joueurs développé dans le cadre du cursus Epitech.
Des équipes d'IA s'affrontent sur un monde en tuiles pour collecter des
ressources et faire évoluer leurs joueurs, le tout orchestré par un serveur
central et observable via une interface graphique.

___

<img width="253" height="209" alt="image" src="https://github.com/user-attachments/assets/97f55318-7e0c-4a93-a58b-b2b96c50e089" />

___

## Composants

| Binaire        | Langage | Rôle                                    |
|----------------|---------|-----------------------------------------|
| `zappy_server` | C++     | Arbitre du jeu, gère le monde et les clients |
| `zappy_gui`    | C++     | Visualisation de la partie              |
| `zappy_ai`     | Python  | Client autonome qui joue pour une équipe |

## Compilation

```sh
make            # compile les trois binaires
make debug      # build de debug (ASan)
make tests_run  # lance les tests unitaires (Criterion)
make tests_cov  # tests + couverture (gcovr)
make fclean     # nettoyage complet
make re         # rebuild complet
```

## Lancement rapide

### Server -> [src/server/README.md](Plus d'info)
```sh
./zappy_server -p 4242 -x 10 -y 10 -n team1 team2 -c 5 -f 100
```

## Libraries
- **libzappy_posix** : RAII wrappers for POSIX resources (FileDescriptor, Address).
- **libzappy_net** : network layer, low level (Listener, ClientBuffer, PollLoop, SignalHandler).
- **libzappy_protocol** : parsing/serialization Zappy protocol (AI & GUI), pure logic without any network dependancy.
