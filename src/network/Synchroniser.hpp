/*
** Agartha-Software, 2013
** Cevy
** File description:
** Synchroniser
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "App.hpp"
#include "CevyNetwork.hpp"
#include "Entity.hpp"
#include "Indirect.hpp"
#include "Plugin.hpp"
#include "Query.hpp"
#include "Scheduler.hpp"
#include "cevy.hpp"
#include "commands/Commands.hpp"
#include "commands/EntityCommands.hpp"
#include "ecs.hpp"
#include "network.hpp"

class cevy::Synchroniser : virtual public cevy::ecs::Plugin {
  public:
  class SummonStage : public ecs::core_stage::before<ecs::core_stage::PreUpdate> {};
  class SyncStage : public ecs::core_stage::after<SummonStage> {};
  struct SyncId {
    size_t id = unset;
    size_t type = 0;
    inline static const size_t unset = -1;
  };
  template <typename Block, typename... Component>
  class SyncBlock;
  using BlockType = std::type_index;
  using ComponentType = std::type_index;
  enum class Mode {
    Server,
    Client,
  };

  template <size_t N, typename... T>
  struct Spawnable {
    static const uint16_t value = N;
  };

  virtual void build_custom(cevy::ecs::App &app) = 0;

  Synchroniser(CevyNetwork &net) : _net(net){};

  Synchroniser(Synchroniser &&rhs) : Plugin(rhs), _net(rhs._net) {}

  // Synchroniser(Mode mode, const std::string &host = std::string(""))
  //     : mode(mode), _net(host, 4995){};
  ~Synchroniser(){};

  template <typename Block, typename... Component>
  void add_sync(cevy::ecs::App &app) {
    _blocks[BlockType(typeid(Block))] = ++_blockCount;
    using F = SyncBlock<Block, Component...>;
    app.add_class_system<F, SyncStage, cevy::ecs::Query<SyncId, Component...>>(
        F(mode, *this, _net));
    // ([&] {
    //     (_sync_map[typeid(Block)].push_back(typeid(Component)));
    // } (), ...);
  }

  template <typename S, typename... T>
  void add_spawnable(T... args) {
    _spawnCommands[S::value] = [args...](ecs::EntityCommands e) { e.insert(T(args)...); };
  }

  template <typename T>
  void summon(cevy::ecs::Commands &command) {
    auto e = command.spawn_empty();
    _spawnCommands[T::value](e);
    auto id = first_free();
    _occupancy[id] = true;
    e.insert(SyncId({id, T::value}));
    _net.sendSummon(id, T::value);
  }

  void dismiss(cevy::ecs::Commands &command, SyncId syncId) {
    auto target = syncId.id;
    std::function<void(ecs::Query<ecs::Entity, SyncId>)> deletor =
        [target, command](ecs::Query<ecs::Entity, SyncId> q) mutable {
          for (auto [e, id] : q) {
            if (id.id == target) {
              command.entity(e).despawn();
            }
          }
        };
    command.system(deletor);
    _occupancy[target] = false;
    _net.sendDismiss(target);
  }

  protected:
  void system_summon(cevy::ecs::Commands command) {
    while (true) {
      auto x = _net.recvSummon();
      if (!x)
        break;
      auto pair = x.value();
      auto e = command.spawn_empty();
      _spawnCommands[pair.second](e);
      e.insert(SyncId{pair.first, pair.second});
      _occupancy[pair.first] = true;
    };
    while (true) {
      auto x = _net.recvDismiss();
      if (!x)
        break;
      auto target = x.value();
      std::function<void(ecs::Query<ecs::Entity, SyncId>)> deletor =
          [target, command](ecs::Query<ecs::Entity, SyncId> q) mutable {
            for (auto [e, id] : q) {
              if (id.id == target) {
                command.entity(e).despawn();
              }
            }
          };
      command.system(deletor);
      _occupancy[target] = false;
    };
  }
  uint16_t first_free() {
    int i = -1;
    for (auto it : _occupancy) {
      ++i;
      if (!it)
        return i;
    }
    throw std::out_of_range("Trying to create more than 1023 synced entities");
  }

  CevyNetwork &_net;
  std::unordered_map<BlockType, uint8_t> _blocks;
  uint8_t _blockCount;
  std::unordered_map<uint8_t, std::function<void(ecs::EntityCommands)>> _spawnCommands;
  std::array<bool, 1024> _occupancy;
  // std::unordered_map<BlockType, std::vector<ComponentType>> _sync_map;
  Mode mode;

  protected:
  void build(cevy::ecs::App &app) override {
    app.add_stage<SummonStage>();
    app.add_stage<SyncStage>();
    std::function<void(ecs::Commands)> func = [this](cevy::ecs::Commands cmd) {
      this->system_summon(cmd);
    };
    app.add_class_system<std::function<void(ecs::Commands)>, SummonStage, cevy::ecs::Commands>(
        func);
    build_custom(app);
  }
};

template <typename Block, typename... Component>
class cevy::Synchroniser::SyncBlock {
  public:
  SyncBlock(Synchroniser::Mode mode, Synchroniser &sync, CevyNetwork &net)
      : mode(mode), _sync(sync), _net(sync._net){};

  private:
  Synchroniser &_sync;
  CevyNetwork &_net;

  template <typename T>
  uint16_t id(uint16_t sync_id) const {
    auto it = _sync._blocks.find(BlockType(typeid(T)));

    if (it != _sync._blocks.end()) {
      uint16_t block = it->second;
      block <<= 10;
      block &= (sync_id & ((1 << 10) - 1));
      return block;
    }
    return 0;
  }

  void system_send(cevy::ecs::Query<SyncId, Component...> &q) const {
    for (auto e : q) {
      auto sync_id = std::get<SyncId &>(e);
      if (sync_id.id == SyncId::unset)
        continue;
      std::vector<uint8_t> block;
      (
          [&] {
            constexpr size_t size = sizeof(Component);
            std::array<uint8_t, size> vec;
            std::memcpy(vec.data(), &std::get<Component &>(e), size);
            block.insert(block.end(), vec.begin(), vec.end());
          }(),
          ...);
      _net.sendState(id<Block>(sync_id.id), block);
    }
  }

  void system_recv(cevy::ecs::Query<SyncId, Component...> &q) const {
    for (auto e : q) {
      auto sync_id = std::get<SyncId &>(e);
      std::optional<std::vector<uint8_t>> block_ = _net.recvState(id<Block>(sync_id.id));
      if (!block_)
        return;
      std::vector<uint8_t> &block = block_.value();

      (
          [&] {
            constexpr size_t size = sizeof(Component);
            std::memcpy(&std::get<Component &>(e), block.data(), size);
            block.erase(block.begin(), block.begin() + size);
          }(),
          ...);
    }
  }
  Mode mode;

  public:
  void operator()(cevy::ecs::Query<SyncId, Component...> q) const {
    if (mode == Mode::Server)
      system_send(q);
    if (mode == Mode::Client)
      system_recv(q);
  };
};
