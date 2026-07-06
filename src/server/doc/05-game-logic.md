Files: `src/server/game/` and `include/server/game/`. This is the authoritative simulation. All gameplay rules live here, decoupled from the network through an observer interface.

## World and geometry

- `Position` is an immutable 2D coordinate on a torus. `normalized(w, h)` wraps both axes and handles negatives with `((x % w) + w) % w`. `shortestVectorTo(other, w, h)` gives the shortest wrapped vector, which the broadcast direction relies on.
- `Tile` is one cell. It stores 7 resource quantities as `Inventory = array<int, 7>`, a list of player ids, and a list of egg ids. It offers `addResource`, `removeResource`, `setResource`, `addPlayer`, `removePlayer`, `addEgg`, `removeEgg`.
- `World` is the authoritative state. The grid is row major with `tileIndex = y * width + x`. It holds teams, players, eggs, and observers. Every mutation notifies observers. Key methods: `tileAt`, `tileAtRaw`, `movePlayer`, `rotatePlayer`, `takeResourceFromTile`, `dropResourceOnTile`, `consumePlayerFood`, `setPlayerLevel`, `freezePlayer`, `unfreezePlayer`, `killPlayer`, egg management (`addEgg`, `hatchEgg`, `removeEgg`, `waitingEggCount`, `pickRandomWaitingEgg`), and notifications (`notifyBroadcast`, `notifyForkStarted`, `notifyIncantationStarted`, `notifyIncantationEnded`). It throws `WorldError` on an unknown id.

## Actors

- `Team` holds a name and `slotsTotal`, with `addSlot()` used by the Fork command.
- `Player` holds id, team, position, orientation, level (starts at 1), inventory (starts with `FOOD = 10`), and state in `{ ALIVE, INCANTING, DEAD }`.
- `Egg` holds id, team, position, state in `{ WAITING, HATCHED }`, and `layingPlayerId` which is -1 for a system spawned egg. `EggSpawner::spawnInitial` lays one egg per team slot on random tiles.

## Resources

- `Constants.hpp` defines `ResourceType { FOOD = 0 ... THYSTAME = 6 }`, `RESOURCE_NAMES`, `INITIAL_FOOD = 10`, and `FOOD_CONSUMPTION_INTERVAL_TIME_UNITS = 126`. It also defines `Orientation { NORTH = 1, EAST, SOUTH, WEST }`, aligned with the GUI protocol.
- `ResourceDensity` gives per tile densities: food 0.5, linemate 0.3, deraumere 0.15, sibur 0.1, mendiane 0.1, phiras 0.08, thystame 0.05.
- `ResourceSpawner` provides `spawnInitial` (density times area, at least 1), `refillMissing` (only adds what is missing, never removes surplus), and `targetCountFor`.
- `ResourceNameResolver::resolve(name)` maps a string like `food` to `ResourceType::FOOD`.

## Elevation rules

`ElevationRules` holds the table `RULES[7]` for level L to L+1, each entry being `{ playersRequired, stonesRequired[7] }`. Levels run from 1 to 8, with `maxLevel() = 8`. For example, 1 to 2 needs 1 player and 1 linemate, while 7 to 8 needs 6 players plus 2 linemate, 2 deraumere, 2 sibur, 2 mendiane, 2 phiras, and 1 thystame.

## Vision and orientation

- `VisionCone::tilesFor(pos, orient, level, w, h)` returns `(level + 1)^2` tiles in `Look` protocol order, nearest row first and left to right, using the forward and right deltas.
- `OrientationHelper` is constexpr geometry: `forwardDelta`, `rightDelta`, `rotateRight` (N to E to S to W), `rotateLeft` (N to W to S to E). The convention is `NORTH = -y`, `EAST = +x`, `SOUTH = +y`, `WEST = -x`.

## Sound direction

`BroadcastDirection::compute(...)` returns K, which is 0 when sender and receiver share a tile, otherwise 1 to 8 clockwise from the receiver front. It computes the shortest toroidal vector from receiver to sender, the `atan2` angle, brings it into the receiver local frame, and quantizes into 8 sectors of 45 degrees.

## Game timers

- `FoodScheduler` consumes one food per player every `126 * 1000 / frequency` milliseconds, and calls `onDeath(playerId)` when food reaches 0. It exposes `startConsumption` (idempotent), `stopConsumption`, and `setFrequency`.
- `RefillScheduler` calls `refillMissing` every `20 * 1000 / frequency` milliseconds, with `start`, `stop`, and `setFrequency`, and keeps a single event in flight.

## Observer interface

`IWorldObserver` is the roughly 20 callback interface that decouples the simulation from the network: `onTileChanged`, `onPlayerAdded`, `onPlayerMoved`, `onPlayerRotated`, `onPlayerRemoved`, `onPlayerStateChanged`, `onPlayerLevelChanged`, `onPlayerInventoryChanged`, `onPlayerDroppedResource`, `onPlayerPickedUpResource`, `onEggAdded`, `onEggHatched`, `onEggRemoved`, `onTeamSlotsChanged`, `onPlayerBroadcast`, `onPlayerForkStarted`, `onPlayerEjected`, `onIncantationStarted`, `onIncantationEnded`. `WorldObserverAdapter` provides empty default implementations. The GUI notifier is the main implementer.

`WorldError` is a `std::runtime_error` thrown on an unknown id or a full team.
