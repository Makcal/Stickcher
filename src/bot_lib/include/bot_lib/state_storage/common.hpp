#ifndef INCLUDE_bot_lib_state_storage_common
#define INCLUDE_bot_lib_state_storage_common

#include "bot_lib/chat_id_type.hpp"
#include "bot_lib/state.hpp"

#include <functional>
#include <optional>

namespace tg_stater {

template <typename T, typename StateT>
concept StateStorage = State<StateT> && requires(T& s, ChatIdT id, const StateT& state) {
    typename StateT;
    { s[id] } -> std::same_as<std::optional<std::reference_wrapper<StateT>>>;
    { s.get(id) } -> std::same_as<std::optional<std::reference_wrapper<StateT>>>; // same as s[id]
    { s.erase(id) } -> std::same_as<void>;
    { s.put(id, state) } -> std::same_as<std::reference_wrapper<StateT>>;
};

template <typename StateT, StateStorage<StateT> StorageT>
class StateProxy {
    std::reference_wrapper<StorageT> storage;
    ChatIdT chatId;

  public:
    explicit StateProxy(StorageT& storage, ChatIdT chatId) : storage{storage}, chatId{chatId} {}

    decltype(auto) get() const {
        return storage.get().get(chatId);
    }

    decltype(auto) operator[]() const {
        return get();
    }

    void erase() const {
        storage.get().erase(chatId);
    }

    decltype(auto) put(const StateT& state) const {
        storage.get().put(chatId, state);
    }
};
template <typename StorageT>
StateProxy(StorageT&, ChatIdT) -> StateProxy<typename StorageT::StateT, StorageT>;

} // namespace tg_stater
#endif // INCLUDE_bot_lib_state_storage_common
