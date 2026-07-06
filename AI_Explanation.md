# AI Explanation

This document describes precisely how the Zappy AI works at the code level.
It covers architecture, the event loop, the FSM, every state, the coordination
protocol, and the constants that govern behaviour.

---

## 1. Architecture overview

Each AI player is a single Python process. There is no shared memory between
players. Communication with teammates happens exclusively through the server's
Broadcast command, which all other players hear.

```
Server (TCP)
    |
    | raw lines
    v
LineBuffer          -- reassembles fragmented TCP data into complete lines
    |
    v
Dispatcher          -- routes each line to the correct handler by prefix
    |
    +-- "dead"                 -> exit(0)
    +-- "Elevation underway"   -> ElevationState.IN_PROGRESS
    +-- "Current level: N"     -> ElevationState.SUCCESS, level updated
    +-- "message K, <text>"    -> parse_broadcast -> FSM.on_broadcast
    +-- anything else          -> CommandSender.on_response (callback queue)

CommandSender       -- pipelined command queue (max 10 in-flight)
    |
    v
FSM.tick()          -- called once per received server line
```

One FSM tick fires after every line received from the server. The tick
evaluates the current priority state and sends at most one command per tick
(limited by the in-flight queue).

---

## 2. Player state

`PlayerState` is a single dataclass that holds everything the AI knows about
itself:

- `level` (int, 1-8)
- `inventory` (dict: food, linemate, deraumere, sibur, mendiane, phiras, thystame)
- `x`, `y` (dead-reckoned position on the toroidal map, starts at 0,0)
- `orientation` (NORTH/EAST/SOUTH/WEST)
- `fsm_state` (current FSM state enum)
- `last_look` (list of tile contents from the most recent Look response)
- `pending_inventory`, `pending_look` (in-flight flags)

Positions are tracked optimistically: when a Forward is sent, `x` or `y` is
updated immediately without waiting for the response. This is sufficient
because navigation always re-reads Look after moving.

---

## 3. Command pipeline

The server accepts at most 10 unanswered commands at a time. `CommandSender`
enforces this with an `_in_flight` counter and a FIFO callback queue.

```
send("Forward", callback=cb)
    -> writes "Forward\n" to socket
    -> appends cb to _callbacks deque
    -> increments _in_flight

on_response(line)               <- called by Dispatcher for unknown prefixes
    -> pops oldest cb from _callbacks
    -> calls cb(line)
    -> decrements _in_flight
```

`can_send` is True when `_in_flight < 10`. All state handlers check `can_send`
before sending any command.

---

## 4. Periodic background actions

Every tick, before the state machine runs, two actions happen unconditionally:

- `poll_inventory`: sends `Inventory` every 20 ticks (if not already in flight).
- `broadcast_have`: sends a `HAVE` broadcast every 40 ticks, but only when
  `food >= SAFE_FOOD` (20). This prevents starving AIs from wasting pipeline
  slots on broadcasts.

`HAVE` messages carry the sender's level and all six stone counts so teammates
can observe the population's state without direct coordination.

---

## 5. FSM states and priority

`evaluate_fsm_state` returns the highest-priority state that applies each tick.
Evaluation is strictly sequential:

```
food < CRITICAL_FOOD (3)          -> CRITICAL_SURVIVAL
current state == ELEVATING        -> ELEVATING  (held until incantation ends)
lead_cooldown == 0
  AND can_lead_elevation()        -> LEADING_ELEVATION
food < SAFE_FOOD (20)             -> FORAGING
needed_stones() not empty         -> COLLECTING
can_fork()                        -> FORKING
(default)                         -> SCOUTING
```

Two overrides are applied after this evaluation:

- If `rally` is set (follower role active):
  - Override to COORDINATING unless the state is CRITICAL_SURVIVAL,
    ELEVATING, or food < FOLLOW_FOOD (5) while in FORAGING.
  - If overridden away from COORDINATING, increment `rally.ticks_stale`.
    When `ticks_stale > 200`, the rally is discarded automatically.

- If `leader` is set (leader role active):
  - Override to LEADING_ELEVATION unless the state is CRITICAL_SURVIVAL,
    ELEVATING, or COORDINATING.
  - If food < LEAD_FOOD (15), broadcast CANCEL and clear the leader role.

If the state is CRITICAL_SURVIVAL and the leader role is active, broadcast
CANCEL and clear the leader role before proceeding.

---

## 6. State descriptions

### CRITICAL_SURVIVAL

Triggered when `food < 3`. All other goals are abandoned.

Priority actions per tick:
1. Food on current tile (tile 0 of last_look): take it immediately.
2. Food visible on another tile: navigate there (axis-aligned path).
3. No look result: request Look.
4. Nothing visible: move Forward one step, then request Look.

### ELEVATING

Set externally by the elevation dispatcher when "Elevation underway" is
received. The FSM is frozen (returns immediately) until either:
- "Current level: N" arrives -> SUCCESS -> level updated, CANCEL broadcast
  sent to followers, both `leader` and `rally` cleared, state set to COLLECTING.
- "ko" arrives from the Incantation command -> FAILED -> CANCEL broadcast,
  both cleared, state set to COLLECTING.

### LEADING_ELEVATION

The player has all required stones and sufficient food (>= LEAD_FOOD = 15).
It takes the leader role and runs a rally sequence.

```
Startup phase (0 to delay-1 ticks):
    delay = 5 + (x * 3 + y) % 6   (position-based, range 5-10)
    Listen for incoming RALLYs from other AIs.
    If a RALLY is received and has_rallied is False, yield (clear leader).

Broadcast phase (tick >= delay):
    Every 3 ticks: send "Broadcast RALLY:<level>:<x>:<y>"
    has_rallied = True after the first RALLY is sent.

Wait phase:
    Opportunistically eat food from current tile while waiting.
    Issue Look keepalives.

Timeout (ticks_waiting > delay + 150):
    Send "Broadcast CANCEL"
    Clear leader role.
    lead_cooldown is set to 50 ticks (prevents immediate retry).

On receiving READY from a follower:
    Increment ready_count.
    If ready_count >= required followers:
        Deposit all required stones with Set commands.
        Send "Incantation".
```

Level 1 is a special case: no followers are needed. Stones are deposited and
Incantation is sent immediately on tick 0.

Required player counts per level (including the leader):

| Current level | Players needed |
|---------------|---------------|
| 1             | 1             |
| 2             | 2             |
| 3             | 2             |
| 4             | 4             |
| 5             | 4             |
| 6             | 6             |
| 7             | 6             |

### COORDINATING

The player is acting as a follower, navigating toward a leader.

`RallyInfo` holds:
- `level`: the target incantation level
- `direction`: K value from the most recent RALLY (1-8, or 0 = same tile)
- `ready_sent`: True after READY has been broadcast
- `step_taken`: True after a Forward has been queued for the current direction
- `forward_pending`: True while a Forward is in flight (gates READY sending)
- `ticks_stale`: ticks since the last RALLY refresh

Priority actions per tick:
1. `ready_sent == True`:
   - Increment `ticks_stale`. If > 200, discard rally.
   - Opportunistically eat food from current tile if food < SAFE_FOOD.
   - Otherwise issue Look keepalive.
2. `direction == 0` (on the leader's tile):
   - Opportunistically eat food from current tile if food < SAFE_FOOD.
   - If not forward_pending, send "Broadcast READY".
   - Issue Look keepalive.
3. Otherwise (still navigating):
   - If not `step_taken` and not `forward_pending`:
     Compute turn commands to face toward K, send them, send Forward.
     Set `step_taken = True`. `forward_pending` cleared when Forward response arrives.
   - Else: issue Look keepalive.

### FORAGING

Triggered when `food < SAFE_FOOD (20)` and food is not critical.

Priority actions per tick:
1. Food on current tile: take it.
2. Food visible on another tile: navigate there (clears last_look after).
3. No look result: request Look.
4. Nothing visible: move Forward, request Look.

### COLLECTING

Triggered when all food needs are met but stones are missing.

Priority actions per tick:
1. Needed stone on current tile: take it.
2. Opportunistic food on current tile if food < SAFE_FOOD.
3. Needed stone visible on another tile: navigate there.
4. No look result: request Look.
5. Nothing visible: roam (Forward every tick, Right turn every 4 steps).

### FORKING

Triggered when food > 30 and level >= 2 and fork cooldown is 0.

Sends one `Fork` command, then sets a cooldown of 300 ticks before forking
again. This expands available team connection slots.

### SCOUTING

Default state when all other conditions are idle.

Explores with a loose spiral:
- Move Forward each tick.
- Turn Right every 6 steps.
- Invalidate last_look after each move and request a fresh Look.

---

## 7. Coordination protocol

All coordination messages use the format `TYPE:field1:field2:...` and are
sent via the server Broadcast command. Five message types exist:

```
HAVE:<level>:<linemate>:<deraumere>:<sibur>:<mendiane>:<phiras>:<thystame>
    Periodic team status announcement (every 40 ticks, food >= 20).

RALLY:<level>:<x>:<y>
    Leader invitation. x,y are the leader's dead-reckoned coordinates.
    Followers navigate using K direction, not these coordinates.

COMING
    Follower acknowledgement: "I heard your RALLY and am heading over."

READY
    Follower arrival: "I am on your tile, ready for incantation."

CANCEL
    Leader abort: "Ignore my RALLY, coordination is over."
```

### Full coordination sequence

```
Leader                              Follower(s)
------                              ----------
enters LEADING_ELEVATION
waits startup_delay ticks
                                    receives RALLY (direction K)
                                    transitions to COORDINATING
                                    sends COMING
broadcasts RALLY every 3 ticks
                                    navigates toward K each tick
                                    direction refreshed on each RALLY
                                    when direction == 0:
                                      sends READY
receives READY
increments ready_count
when ready_count >= needed:
  sends Set <stone> for each req.
  sends Incantation
server sends "Elevation underway"   server sends "Elevation underway"
server sends "Current level: N"     server sends "Current level: N"
  -> updates level                    -> updates level
  -> broadcasts CANCEL                -> clears rally
  -> clears leader
  -> lead_cooldown = 50
```

### Two-leader prevention

When a follower receives a RALLY and it is itself in LEADING_ELEVATION
with `has_rallied == False` (has not yet committed), it yields: it clears
its own leader role and follows the incoming RALLY instead. If `has_rallied
== True`, the incoming RALLY is ignored so a committed leader does not
abandon followers mid-way.

---

## 8. Navigation

Navigation uses dead-reckoned (x, y) coordinates on a toroidal map.

`navigate_to(state, sender, tx, ty)` breaks the path into at most two
axis-aligned segments (horizontal then vertical), computing the shortest
distance along each axis accounting for map wrap-around. For each segment
it sends the required turn commands followed by the required Forward commands.

For follower navigation in COORDINATING state, absolute coordinates are not
used. The K direction from the RALLY broadcast is converted to a cardinal
orientation with `direction_to_orientation(k, current_orientation)`, and the
follower turns to face that direction then sends one Forward per tick until
the direction becomes 0.

---

## 9. Food thresholds and their roles

| Constant       | Value | Role                                                  |
|----------------|-------|-------------------------------------------------------|
| CRITICAL_FOOD  | 3     | Below this: enter CRITICAL_SURVIVAL immediately.       |
| FOLLOW_FOOD    | 5     | Below this: break out of COORDINATING to forage.       |
| LEAD_FOOD      | 15    | Below this while leading: broadcast CANCEL, give up.   |
| SAFE_FOOD      | 20    | Target for foraging; below this: enter FORAGING.       |
| FORK_FOOD      | 30    | Minimum food required before forking is attempted.     |

---

## 10. Elevation requirements

| Current level | Players | linemate | deraumere | sibur | mendiane | phiras | thystame |
|---------------|---------|----------|-----------|-------|----------|--------|----------|
| 1             | 1       | 1        | 0         | 0     | 0        | 0      | 0        |
| 2             | 2       | 1        | 1         | 1     | 0        | 0      | 0        |
| 3             | 2       | 2        | 0         | 1     | 0        | 2      | 0        |
| 4             | 4       | 1        | 1         | 2     | 0        | 1      | 0        |
| 5             | 4       | 1        | 2         | 1     | 3        | 0      | 0        |
| 6             | 6       | 1        | 2         | 3     | 0        | 1      | 0        |
| 7             | 6       | 2        | 2         | 2     | 2        | 2      | 1        |

---

## 11. Key timings at f=100

The server frequency `-f 100` means 1 food unit is consumed every ~1.33 server
ticks (approximately 0.07 real seconds per food unit). The following timings
are approximate:

- RALLY_TIMEOUT_TICKS = 150: a failed rally lasts at most ~10 real seconds.
- HAVE_BROADCAST_INTERVAL = 40: team status broadcast every ~2.8 real seconds.
- INVENTORY_POLL_INTERVAL = 20: inventory refresh every ~1.4 real seconds.
- FORK_COOLDOWN = 300: minimum gap between forks (~21 real seconds).
- lead_cooldown after timeout = 50 ticks before a new leadership attempt.

---

## 13. Road to level 7: problems encountered and solutions applied

This section documents the bugs that prevented reaching level 7 and the fixes
that resolved them.

---

### 13.1 Server bug: collectParticipants regressed lower-level players

**Problem.** The server function `collectParticipants` collected every player
on the tile regardless of their level, then `applySuccess` set all of them to
`initiator_level + 1`. A level-4 player standing on a tile where a level-6
incantation completed would be set to level 7   correct   but a level-8 player
on the same tile would also be overwritten to level 7, regressing them.

**Fix applied in `AiDispatcher.cpp`.** `collectParticipants` now only includes
players whose level is `<= initiator level`. Higher-level bystanders are
excluded from the participant list entirely.

```cpp
for (int id : _world.tileAt(pos).playerIds())
    if (isPlayerAlive(id) && _world.player(id).level() <= level)
        result.push_back(id);
```

This also makes the "boost" mechanic intentional: a level-5 player standing on
a tile where a level-6 incantation occurs is included as a participant and
elevated to level 7 at no extra cost.

---

### 13.2 Client bug: followers stuck in ELEVATING after a failed incantation

**Problem.** When an incantation failed on the server side, the initiator
received `ko` from its own `Incantation` callback. Non-initiating participants
(followers) also received a raw `ko` line. The dispatcher routed `ko` to
`CommandSender.on_response`, which popped the oldest pending callback. If the
follower had no command in flight (it was frozen during the incantation), the
`ko` consumed a future callback, corrupting the queue. Worse, the follower
never exited `ElevationStatus.IN_PROGRESS` and was stuck in ELEVATING
indefinitely.

**Fix applied in `elevation.py`.** A dedicated `make_ko_elevation_handler` is
registered for the `"ko"` prefix. It checks the elevation status:

```python
def make_ko_elevation_handler(state: ElevationState, sender=None):
    def handler(line: str) -> None:
        if state.status == ElevationStatus.IN_PROGRESS:
            state.status = ElevationStatus.FAILED
            if sender is not None and sender.in_flight > 0:
                sender.consume_response(line)
        elif sender is not None:
            sender.on_response(line)
    return handler
```

If elevation is in progress the handler marks it as FAILED and consumes the
callback only when there is one in flight (the Incantation command of the
initiator). If elevation is not in progress the `ko` is forwarded to the
normal callback queue so other commands (e.g. failed Set or Take) are handled
correctly.

---

### 13.3 Protocol bug: followers ignored rallies from higher-level leaders

**Problem.** `on_rally_received` in `coordination.py` contained the guard:

```python
if level != state.level:
    return
```

A level-5 follower receiving a RALLY broadcast from a level-6 leader would
reject it immediately. With the server's `<= initiator` fix, including a
level-5 player in a level-6 incantation would boost them to level 7 for free.
But the follower never heard the invitation.

The practical consequence: reaching level 7 requires 6 players at level 6
simultaneously. Only 4-5 AIs reliably reached level 6 before the mendiane
bottleneck (level-5→6 ritual requires 3 mendiane, density = 0.1 on a 10×10
map = ~10 stones total). The missing 1-2 participants were often AIs at level 5
who had all required stones but were simply excluded from the level-6 rally.

**Fix applied in `coordination.py`.**

```python
if level < state.level:
    return
```

A follower now accepts rallies from leaders at its own level **or higher**.
Additional guards prevent unwanted switching:

- If a READY has already been sent (`existing.ready_sent`), no rally switch
  occurs (the follower is committed).
- If the follower already has an active rally at a higher level, it does not
  downgrade to a lower-level rally (`if level < existing.level: return`).

---

### 13.4 Protocol bug: committed same-level leaders blocked higher-level rallies

**Problem.** A second guard in `on_rally_received`:

```python
if leader_holder[0] is not None:
    if leader_holder[0].has_rallied:
        return
```

prevented any leader who had already broadcast a RALLY from yielding to
another RALLY   regardless of the incoming leader's level. This caused split
rallies: two leaders at level 5 and level 6 would each accumulate 2-3
followers instead of one rally at level 6 collecting all 5 participants.

Observed at t≈130s: 1 level-6 leader + 1 level-6 follower + 1 level-5
follower + **1 level-5 leader who refused to yield** = only 4 participants
instead of 6.

**Fix applied in `coordination.py`.**

```python
if leader_holder[0] is not None:
    if level == state.level and leader_holder[0].has_rallied:
        # Committed to a same-level rally; don't yield (prevents deadlock).
        return
    # Higher-level leader takes priority: yield even if already committed.
    if leader_holder[0].has_rallied and sender.can_send:
        sender.send(f"Broadcast {format_msg(MSG_CANCEL)}",
                    callback=lambda r: None)
    leader_holder[0] = None
```

A leader who receives a RALLY from a **higher-level** leader now broadcasts
CANCEL to its own followers, clears its leader role, and joins the higher-level
leader as a follower. Same-level committed leaders are unaffected to prevent
mutual deadlock.

---

### 13.5 Observed result after all four fixes

With all fixes deployed (10×10 map, 20 AIs, f=100):

```
t≈57s   6 AIs reach level 5 simultaneously
t≈68s   5 of those 6 reach level 6 in a single incantation
         (a level-5 AI is included as participant and boosted)
t≈81s   5 AIs reach level 7 (lvl7+=5 at t=90s, lvl7+=7 total)
```

The level-5→6 ritual at t=57s collected 6 participants:
- 5 players at level 5 (the level-5 ritual requires only 4, so the 5th was
  included as a bonus)
- After elevation: 5 at level 6

The level-6→7 ritual at t=68s used the same tile and collected the remaining
level-5 player as a cross-level participant (boosted to 7 by the server fix).

---

## 12. File map

```
src/ia/
  main.py               entry point, connects and runs the event loop
  event_loop.py         readline loop, dispatcher wiring
  dispatcher.py         line router by string prefix
  sender.py             pipelined command queue (max 10 in-flight)
  line_buffer.py        TCP reassembly into complete lines
  connection.py         TCP socket creation
  handshake.py          initial server handshake (team name, slots, map size)
  player_state.py       PlayerState dataclass, orientation helpers
  fsm.py                FSM orchestrator (tick, evaluate, overrides)
  fsm_helpers.py        thresholds, poll_inventory, request_look, find_food_tile
  survival.py           CRITICAL_SURVIVAL state handler
  foraging.py           FORAGING state handler
  collecting.py         COLLECTING state handler
  leading_elevation.py  LEADING_ELEVATION state handler (leader role)
  coordination.py       COORDINATING state handler (follower role)
  scouting.py           SCOUTING state handler
  forking.py            FORKING state handler
  broadcast_protocol.py message format/parse (HAVE/RALLY/COMING/READY/CANCEL)
  have_broadcast.py     HAVE broadcast scheduling and team member tracking
  broadcast.py          raw "message K, text" line parser
  broadcast_direction.py  K direction to cardinal orientation mapping
  elevation.py          ElevationState, server response handlers
  stones.py             ELEVATION_REQUIREMENTS table, needed_stones()
  navigation.py         navigate_to, turns_to_face, torus-aware path planner
  look.py               Look response parser
  inventory.py          Inventory response parser
  torus.py              toroidal distance and wrap helpers
```
