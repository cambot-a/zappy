##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## SCOUTING state handler: default exploration when nothing else to do
##

from fsm_helpers import request_look
from player_state import PlayerState, apply_forward, apply_right
from sender import CommandSender

# Turn right every N forward steps to avoid walking in a straight line.
SCOUT_TURN_INTERVAL = 6


def tick_scouting(state: PlayerState, sender: CommandSender,
                  scout_counter: list[int]) -> None:
    """Execute one FSM tick in the SCOUTING state.

    Explores the map with a loose spiral (forward + periodic right turn)
    while keeping the Look result fresh. A new Look is requested whenever
    the previous one has been consumed.

    Priority actions:
    1. No Look result: request one.
    2. Roam: forward every tick, right turn every SCOUT_TURN_INTERVAL steps.

    Args:
        state: Mutable player state.
        sender: Command sender.
        scout_counter: Single-element list used as a mutable step counter.
    """
    if not sender.can_send:
        return

    if not state.last_look:
        request_look(state, sender)
        return

    scout_counter[0] += 1
    if scout_counter[0] % SCOUT_TURN_INTERVAL == 0:
        sender.send("Right",
                    callback=lambda r: None)
    else:
        sender.send("Forward",
                    callback=lambda r: None)
    state.last_look = []   # invalidate so a fresh Look is requested next tick
    request_look(state, sender)
