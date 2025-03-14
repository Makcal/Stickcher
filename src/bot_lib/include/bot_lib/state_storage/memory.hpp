#ifndef INCLUDE_bot_lib_state_storage_memory
#define INCLUDE_bot_lib_state_storage_memory

#include "bot_lib/state.hpp"
#include "bot_lib/chat_id_type.hpp"

#include <functional>
#include <optional>
#include <unordered_map>

namespace tg_stater {

template <State StateT_>
class MemoryStateStorage {
    std::unordered_map<ChatIdT, StateT_> states;

  public:
    using StateT = StateT_;

    std::optional<std::reference_wrapper<StateT>> get(const ChatIdT id) {
        auto it = states.find(id);
        return it == states.end() ? std::nullopt : std::optional{std::ref(it->second)};
    }

    decltype(auto) operator[](const ChatIdT id) {
        return get(id);
    }

    void erase(const ChatIdT id) {
        if (auto it = states.find(id); it != states.end()) {
            states.erase(it);
        }
    }

    std::reference_wrapper<StateT> put(const ChatIdT id, const StateT& state) {
        return states[id] = state;
    }
};
} // namespace tg_stater

#endif // INCLUDE_bot_lib_state_storage_memory
