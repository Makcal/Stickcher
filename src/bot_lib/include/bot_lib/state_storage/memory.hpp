#ifndef INCLUDE_bot_lib_state_storage_memory
#define INCLUDE_bot_lib_state_storage_memory

#include "bot_lib/chat_id_type.hpp"
#include "bot_lib/state.hpp"

#include <unordered_map>
#include <utility>

namespace tg_stater {

template <concepts::State StateT_>
class MemoryStateStorage {
    std::unordered_map<ChatIdT, StateT_> states;

  public:
    using StateT = StateT_;

    StateT* get(const ChatIdT id) {
        auto it = states.find(id);
        return it == states.end() ? nullptr : &it->second;
    }

    decltype(auto) operator[](const ChatIdT id) {
        return get(id);
    }

    void erase(const ChatIdT id) {
        if (auto it = states.find(id); it != states.end())
            states.erase(it);
    }

    StateT& put(const ChatIdT id, const StateT& state) {
        return states.insert_or_assign(id, state).first->second;
    }

    StateT& put(const ChatIdT id, StateT&& state) {
        return states.insert_or_assign(id, std::move(state)).first->second;
    }
};
} // namespace tg_stater

#endif // INCLUDE_bot_lib_state_storage_memory
