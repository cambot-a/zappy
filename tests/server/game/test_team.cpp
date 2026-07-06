/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for Team
*/

#include <criterion/criterion.h>

#include "server/game/Team.hpp"

using zappy::server::game::Team;

/* construction */

Test(team, defaults)
{
    const Team t("alpha", 3);
    cr_assert_str_eq(t.name().c_str(), "alpha");
    cr_assert_eq(t.slotsTotal(), 3);
}

/* capacity */

Test(team, add_slot_increments_total)
{
    Team t("alpha", 1);
    t.addSlot();
    cr_assert_eq(t.slotsTotal(), 2);
    t.addSlot();
    cr_assert_eq(t.slotsTotal(), 3);
}
