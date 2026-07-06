/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Unit tests for the runtime feature flags registry
*/

#include <criterion/criterion.h>
#include <utility>
#include <vector>

#include "server/config/FeatureFlags.hpp"

using zappy::server::config::FeatureFlag;
using zappy::server::config::FeatureFlags;

Test(feature_flags, default_state_all_disabled)
{
    FeatureFlags flags;
    cr_assert_not(flags.isEnabled(FeatureFlag::EVENTS));
    cr_assert_not(flags.isEnabled(FeatureFlag::BIOMES));
    cr_assert_not(flags.isEnabled(FeatureFlag::ADMIN));
    cr_assert_not(flags.isEnabled(FeatureFlag::PROFILE));
}

Test(feature_flags, enable_then_is_enabled)
{
    FeatureFlags flags;
    flags.enable(FeatureFlag::EVENTS);
    cr_assert(flags.isEnabled(FeatureFlag::EVENTS));
    cr_assert_not(flags.isEnabled(FeatureFlag::BIOMES));
    cr_assert_not(flags.isEnabled(FeatureFlag::ADMIN));
    cr_assert_not(flags.isEnabled(FeatureFlag::PROFILE));
}

Test(feature_flags, disable_after_enable)
{
    FeatureFlags flags;
    flags.enable(FeatureFlag::EVENTS);
    flags.disable(FeatureFlag::EVENTS);
    cr_assert_not(flags.isEnabled(FeatureFlag::EVENTS));
}

Test(feature_flags, set_with_boolean)
{
    FeatureFlags flags;
    flags.set(FeatureFlag::BIOMES, true);
    cr_assert(flags.isEnabled(FeatureFlag::BIOMES));
    flags.set(FeatureFlag::BIOMES, false);
    cr_assert_not(flags.isEnabled(FeatureFlag::BIOMES));
}

Test(feature_flags, enable_is_idempotent)
{
    FeatureFlags flags;
    flags.enable(FeatureFlag::ADMIN);
    flags.enable(FeatureFlag::ADMIN);
    cr_assert(flags.isEnabled(FeatureFlag::ADMIN));
}

Test(feature_flags, disable_on_disabled_is_noop)
{
    FeatureFlags flags;
    flags.disable(FeatureFlag::PROFILE);
    cr_assert_not(flags.isEnabled(FeatureFlag::PROFILE));
}

Test(feature_flags, from_name_roundtrip)
{
    cr_assert(FeatureFlags::fromName("events") == FeatureFlag::EVENTS);
    cr_assert(FeatureFlags::fromName("biomes") == FeatureFlag::BIOMES);
    cr_assert(FeatureFlags::fromName("admin") == FeatureFlag::ADMIN);
    cr_assert(FeatureFlags::fromName("profile") == FeatureFlag::PROFILE);
}

Test(feature_flags, from_name_unknown_is_nullopt)
{
    cr_assert_not(FeatureFlags::fromName("bogus").has_value());
    cr_assert_not(FeatureFlags::fromName("").has_value());
    cr_assert_not(FeatureFlags::fromName("EVENTS").has_value());
}

Test(feature_flags, to_name)
{
    cr_assert_eq(FeatureFlags::toName(FeatureFlag::EVENTS), "events");
    cr_assert_eq(FeatureFlags::toName(FeatureFlag::BIOMES), "biomes");
    cr_assert_eq(FeatureFlags::toName(FeatureFlag::ADMIN), "admin");
    cr_assert_eq(FeatureFlags::toName(FeatureFlag::PROFILE), "profile");
}

Test(feature_flags, snapshot_reflects_current_state)
{
    FeatureFlags flags;
    flags.enable(FeatureFlag::EVENTS);
    flags.enable(FeatureFlag::PROFILE);

    const std::vector<std::pair<FeatureFlag, bool>> snap = flags.snapshot();
    cr_assert_eq(snap.size(), 4U);
    cr_assert(snap[0] == std::make_pair(FeatureFlag::EVENTS, true));
    cr_assert(snap[1] == std::make_pair(FeatureFlag::BIOMES, false));
    cr_assert(snap[2] == std::make_pair(FeatureFlag::ADMIN, false));
    cr_assert(snap[3] == std::make_pair(FeatureFlag::PROFILE, true));
}
