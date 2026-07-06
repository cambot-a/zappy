##
## EPITECH PROJECT, 2026
## local_zappy
## File description:
## FORKING state handler: lay an egg to expand team slots
##

from player_state import PlayerState
from sender import CommandSender

# Player must have more food than this to consider forking.
FORK_FOOD_THRESHOLD = 20

# Player must be at least this level before forking is useful.
FORK_MIN_LEVEL = 2

# Ticks to wait before forking again after a successful Fork command.
FORK_COOLDOWN = 300


def can_fork(state: PlayerState, cooldown: int) -> bool:
    """Return True when the player should lay an egg right now.

    Conditions:
    - Cooldown has expired (no recent fork).
    - Food is well above the safe threshold (spare resources).
    - Player is at least FORK_MIN_LEVEL (level 1 players are newly connected).

    Args:
        state: Current player state.
        cooldown: Remaining cooldown ticks; 0 means ready to fork again.
    """
    if cooldown > 0:
        return False
    if state.inventory.get("food", 0) <= FORK_FOOD_THRESHOLD:
        return False
    return state.level >= FORK_MIN_LEVEL


def tick_forking(state: PlayerState, sender: CommandSender,
                 cooldown: list[int]) -> None:
    """Execute one FSM tick in the FORKING state.

    Sends a single Fork command and immediately sets the cooldown so the
    player does not keep forking every tick.

    Args:
        state: Mutable player state.
        sender: Command sender.
        cooldown: Single-element list holding the remaining cooldown counter.
    """
    if not sender.can_send:
        return
    sender.send("Fork", callback=lambda r: None)
    cooldown[0] = FORK_COOLDOWN
