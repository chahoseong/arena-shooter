// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Custom trace channels for this project.
 *
 * Keep in sync with [/Script/Engine.CollisionProfile] in Config/DefaultEngine.ini.
 * Nothing links the channel index to its configured name at compile time, so
 * reordering the channels there silently repoints these macros.
 *
 * Deliberately has no includes: the macros expand at the use site, where the
 * ECollisionChannel enum is already visible.
 */

#define ArenaShooter_TraceChannel_Weapon	ECC_GameTraceChannel1
