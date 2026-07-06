/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for Egg
*/

#include <criterion/criterion.h>

#include "server/game/Egg.hpp"

using zappy::server::game::Egg;
using zappy::server::game::EggState;
using zappy::server::game::Position;

/* construction */

Test(egg, defaults)
{
    const Egg e(5, "beta", Position(1, 2));
    cr_assert_eq(e.id(), 5);
    cr_assert_str_eq(e.team().c_str(), "beta");
    cr_assert(e.position() == Position(1, 2));
    cr_assert_eq(static_cast<int>(e.state()), static_cast<int>(EggState::WAITING));
}

/* state transition */

Test(egg, hatch)
{
    Egg e(5, "beta", Position(1, 2));
    e.setState(EggState::HATCHED);
    cr_assert_eq(static_cast<int>(e.state()), static_cast<int>(EggState::HATCHED));
}
