/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for ResourceNameResolver
*/

#include <criterion/criterion.h>

#include "server/game/Constants.hpp"
#include "server/game/ResourceNameResolver.hpp"

using zappy::server::game::ResourceNameResolver;
using zappy::server::game::ResourceType;

/* 1. every valid name resolves to its matching ResourceType */

Test(resource_name_resolver, valid_names_resolve)
{
    cr_assert(ResourceNameResolver::resolve("food") == ResourceType::FOOD);
    cr_assert(
        ResourceNameResolver::resolve("linemate") == ResourceType::LINEMATE);
    cr_assert(
        ResourceNameResolver::resolve("deraumere") == ResourceType::DERAUMERE);
    cr_assert(ResourceNameResolver::resolve("sibur") == ResourceType::SIBUR);
    cr_assert(
        ResourceNameResolver::resolve("mendiane") == ResourceType::MENDIANE);
    cr_assert(ResourceNameResolver::resolve("phiras") == ResourceType::PHIRAS);
    cr_assert(
        ResourceNameResolver::resolve("thystame") == ResourceType::THYSTAME);
}

/* 2. unknown, empty, miscased or padded names return nullopt */

Test(resource_name_resolver, unknown_names_return_nullopt)
{
    cr_assert(!ResourceNameResolver::resolve("stone").has_value());
    cr_assert(!ResourceNameResolver::resolve("").has_value());
    cr_assert(!ResourceNameResolver::resolve("FOOD").has_value());
    cr_assert(!ResourceNameResolver::resolve("food ").has_value());
}

/* 3. resolving the same name twice yields the same value */

Test(resource_name_resolver, idempotent)
{
    cr_assert(ResourceNameResolver::resolve("food")
        == ResourceNameResolver::resolve("food"));
}
