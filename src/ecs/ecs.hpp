/*
** Agartha-Software, 2023
** C++evy
** File description:
** ecs namespace
*/

#pragma once

namespace cevy {
/**
 * @brief Holds the entity components system
 *
 * Access each classes for a complete description
 *
 * note: here classes are being forward declared
 * they will be expanded by their relevant files
 */
namespace ecs {
class App;
class Entity;
class Plugin;
class Scheduler;
class Commands;
class Command;
class EntityCommands;
class World;

template <typename T>
class Factory;

template <typename Content>
class Resource;
template <class... Containers>
class Query;

template <class... Containers>
class iterator;
} // namespace ecs
} // namespace cevy
