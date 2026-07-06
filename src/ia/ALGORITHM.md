# Zappy AI - Algorithm Design

## Why an algorithm and not machine learning

The game rules are fully deterministic and known in advance. The state space is manageable, the win condition is precise, and there is no training data. A reinforcement learning agent would require millions of simulated games to converge, and there is no guarantee it would discover the coordination protocol needed for incantation in reasonable time. A carefully designed deterministic algorithm, by contrast, can encode domain knowledge directly, reason about the current game state with certainty, and coordinate with teammates via a structured protocol. It is also fully reproducible, debuggable, and predictable under time pressure.

---

## Architecture overview

Each `zappy_ai` process drives exactly one player. The processes are independent and communicate exclusively through the server's `Broadcast` mechanism. There is no shared memory, no external channel, and no central coordinator. The algorithm running in each process is identical; roles emerge dynamically from the game state.

The core of each player is a **priority-driven finite state machine** (FSM). At every decision cycle, the player evaluates its current state against a fixed priority hierarchy and transitions to the highest-priority applicable state. Commands are pipelined toward the server (up to 10 in flight) so that the player is never idle while waiting for responses.

---

## Self-tracking: dead reckoning

The server does not send the player's absolute position to the AI client. The player must therefore maintain its own position internally by counting the effects of movement commands.

The player stores three values: its current `x` coordinate, its current `y` coordinate, and its current orientation (one of four cardinal directions: North, East, South, West). All three are updated each time a movement command receives a successful response:

- `Forward` moves the player one tile in the current orientation direction. The coordinates are updated modulo the map width and height, respectively, so toroidal wrapping is handled automatically.
- `Right` rotates the orientation 90 degrees clockwise.
- `Left` rotates the orientation 90 degrees counter-clockwise.

The starting position is unknown, so the player assigns itself an arbitrary starting coordinate (for example, `(0, 0)`). This is consistent across the team because absolute coordinates are only used to communicate rally points via Broadcast. A teammate receiving a rally-point coordinate does not need to know the sender's true position; it only needs the direction to walk, which is derivable from the broadcast direction indicator `K`.

---

## State machine

The FSM has eight states. At each decision cycle, the player evaluates them from highest to lowest priority and enters the first one whose entry condition is satisfied.

### Priority 1: CRITICAL_SURVIVAL

**Condition:** food in inventory is below a critical threshold (suggested: fewer than 3 units).

The player drops every other concern and focuses exclusively on finding and collecting food. It runs `Look`, scans all visible tiles for food, and moves directly toward the nearest one. If no food is visible, it moves in a spiral pattern until food appears. This state is not exited until food rises above the critical threshold.

The threshold is deliberately conservative: at 3 food units, the player has roughly 378 ticks before death. With `f=100`, that is about 3.78 seconds, which is enough for a few `Forward` and `Take` commands but not much more.

### Priority 2: ELEVATING

**Condition:** the player has just sent `Incantation` and is waiting for the result, or has received `Elevation underway` from the server and is frozen.

During this state the player does nothing. All queued commands are flushed. The player simply waits for the server to send either `Current level: k` (success) or `ko` (failure). On success, the player's internal level is incremented. On failure, the player returns to the collection phase for the same level.

This state is non-interruptible by design: the player is physically frozen during a ritual and cannot act regardless.

### Priority 3: LEADING_ELEVATION

**Condition:** the player has all the stones required for its current level's elevation ritual, and it has received confirmation from enough teammates of the same level that they are present on its tile (verified via `Look`).

The player deposits all required stones onto the tile using `Set`, verifies via `Look` that the tile contains the correct stones and the correct number of same-level players, then sends `Incantation` and transitions to ELEVATING.

If the `Look` check fails (a teammate moved away, stones are missing), the player cancels the ritual attempt, picks the stones back up, and re-enters the COORDINATING state.

### Priority 4: COORDINATING

**Condition:** the player has all the stones required for its current level's elevation ritual, but the ritual conditions are not yet met (not enough same-level players present, or the player is a follower navigating toward a leader).

This state is divided into two sub-roles that are assigned dynamically.

**Leader sub-role:**

A player becomes a leader if it is the first in the team to broadcast a rally call for its level. The leader broadcasts a structured message containing its level and its self-tracked position. It then waits at its current tile, periodically re-broadcasting the rally call, until enough same-level players are visible on the tile via `Look`. While waiting, the leader does not move but continues to collect food opportunistically if food appears on its tile (via `Take`, no movement required).

**Follower sub-role:**

A player becomes a follower when it receives a rally call for its current level. It decodes the direction indicator `K` from the broadcast and begins walking toward the sender. As it moves, it updates its dead-reckoned position. It sends a confirmation broadcast so the leader knows it is coming. If it receives a new rally call from the same leader with a different `K`, it corrects its heading. Once its `Look` output matches the leader's expected tile content (the leader is visible on tile 0, meaning the follower is on the same tile), the follower stops and broadcasts `READY` to the leader.

**Leader election:**

If two players broadcast a rally call for the same level simultaneously, the conflict is resolved by comparing player identifiers embedded in the Broadcast message. The player with the lower identifier yields and becomes a follower.

### Priority 5: FORAGING

**Condition:** food in inventory is below a safe threshold (suggested: fewer than 15 units), and the player is not currently in a coordination or elevation state.

The player uses `Look` to find the nearest food tile and moves toward it. If no food is visible, it moves forward until food appears or until the threshold is no longer a concern. This state differs from CRITICAL_SURVIVAL in that the player does not abandon ongoing tasks entirely; it merely biases its movement toward food when the opportunity arises.

### Priority 6: COLLECTING

**Condition:** the player is missing one or more stones required for its current level's elevation ritual.

The player first computes which stones it still needs by comparing its inventory against the elevation requirement table for its level. It then uses `Look` to find those stones on nearby tiles and navigates toward them. If the needed stone is not visible, the player roams using a systematic spiral or random-walk pattern until the stone appears in the field of view.

The player avoids picking up stones it does not need, so as not to deprive teammates. This is enforced by cross-checking the pick-up target against the player's own needed-stone list before issuing `Take`.

### Priority 7: FORKING

**Condition:** the team has fewer living players than the maximum needed (6 for level 8), the current player has not recently forked, and the player has comfortable food reserves (suggested: more than 25 units).

The player issues `Fork` (42/f ticks). Once the egg is laid, the player resumes its normal cycle. The newly available slot allows a new `zappy_ai` process to connect. The forking condition is rate-limited: a player does not fork twice in rapid succession, to avoid consuming food unnecessarily.

### Priority 8: SCOUTING

**Condition:** none of the above conditions are met.

The player runs `Look` and `Inventory` to update its knowledge of its surroundings and its resource state, then moves forward one tile. This ensures the player is always exploring and not standing still.

---

## Broadcast protocol

All inter-player communication uses a structured plain-text format embedded in the `Broadcast` message. Each message has the form:

```
TYPE:field1:field2:...
```

The following message types are defined:

**RALLY:level:x:y**
Sent by a leader. Announces that the player at self-tracked position `(x, y)` is ready for elevation to `level+1` and needs same-level teammates to gather.

**COMING:level:sender_id**
Sent by a follower acknowledging a RALLY. Informs the leader that one more player of the correct level is en route.

**READY:level:sender_id**
Sent by a follower once it has arrived on the leader's tile (confirmed via `Look`). The leader counts these to determine when to initiate the ritual.

**CANCEL:level:sender_id**
Sent by a leader or follower when the rally is aborted (food critical, ritual failed, etc.). All participants return to COLLECTING.

**HAVE:resource:count:level:sender_id**
Periodic inventory broadcast. Allows teammates to know which stones are already claimed, avoiding redundant collection.

The broadcast direction indicator `K` is used for navigation. When a follower receives a `RALLY`, it reads `K` to determine which direction the leader is relative to itself and begins moving in that direction. As it gets closer, subsequent re-broadcasts from the leader will yield a `K` value closer to `1` (directly in front). When `K` equals `0`, the follower is on the same tile as the leader.

---

## Navigation toward a broadcast source

Given the direction `K` of an incoming broadcast (values 1 through 8), the player determines the required sequence of turns and forward moves to close the distance. The tile numbering around the player is:

```
4  3  2
5  @  1
6  7  8
```

where `@` is the player and `1` is directly in front. The player always faces North internally; it adjusts by computing how many `Right` or `Left` turns are needed to face the direction corresponding to `K`, then issues `Forward` commands.

Because the world is toroidal, the shortest path may wrap around an edge. The navigation sub-routine always chooses the shorter of the two possible paths along each axis independently.

---

## Elevation readiness check

Before issuing `Incantation`, the leader performs a two-step verification:

1. It runs `Look` and parses tile 0 (its own tile). It counts the number of players present and the stones present. Both must match the requirement table for the current level.
2. It checks its own inventory to confirm all required stones have been deposited (i.e., are no longer in inventory after the `Set` commands).

Only if both checks pass does the leader issue `Incantation`. This prevents wasting the 300/f frozen period on a ritual that will certainly fail.

---

## Food management during elevation

A frozen player continues to consume food during the incantation (300/f ticks). At `f=100`, that is 3 seconds, consuming roughly 2.4 food units. Before entering LEADING or accepting a RALLY as a follower, the player checks that it has at least 5 food units in inventory. If not, it first forages and only joins the ritual once the threshold is met.

---

## Command pipelining

The server allows up to 10 commands to be queued without waiting for responses. The algorithm exploits this by grouping commands that can be issued in sequence without needing intermediate feedback. For example, a sequence of `Forward`, `Forward`, `Right`, `Forward` to reach a nearby tile can be sent in a single burst. The responses are then processed in order as they arrive. The pipeline depth is tracked internally; no new command is sent when 10 are already in flight.

---

## Team convergence toward level 8

The overall strategy converges toward the win condition through the following macro-progression:

1. All players start at level 1 and independently collect one `linemate` each. Each player can self-elevate to level 2 alone (only 1 player required).
2. From level 2 onward, players must coordinate in pairs, then groups of four, then groups of six.
3. Forking is used aggressively in the early levels to ensure at least six players exist before the final elevation.
4. In the final stretch (levels 6 and 7), the required group size is six. All six players must reach the same tile simultaneously with substantial stone reserves. This phase requires sustained coordination over many broadcasts and is the primary design challenge of the algorithm.
