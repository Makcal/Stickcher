#ifndef INCLUDE_bot_lib_state_storage_memory
#define INCLUDE_bot_lib_state_storage_memory

#include "bot_lib/tg_types.hpp"
#include "bot_lib/state.hpp"

#include <unordered_map>
#include <utility>

namespace tg_stater {

template <concepts::State StateT_>
class MemoryStateStorage {
    std::unordered_map<ChatUserIdT, StateT_> states;

  public:
    using StateT = StateT_;

    StateT* operator[](const ChatUserIdT& key) {
        auto it = states.find(key);
        return it == states.end() ? nullptr : &it->second;
    }

    void erase(const ChatUserIdT& key) {
        if (auto it = states.find(key); it != states.end())
            states.erase(it);
    }

    StateT& put(const ChatUserIdT& key, const StateT& state) {
        return states.insert_or_assign(key, state).first->second;
    }

    StateT& put(const ChatUserIdT& key, StateT&& state) {
        return states.insert_or_assign(key, std::move(state)).first->second;
    }
};
} // namespace tg_stater

#endif // INCLUDE_bot_lib_state_storage_memory
