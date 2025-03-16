#ifndef INCLUDE_bot_lib_state_storage_common
#define INCLUDE_bot_lib_state_storage_common

#include "bot_lib/chat_id_type.hpp"
#include "bot_lib/state.hpp"

#include <concepts>
#include <functional>
#include <utility>

namespace tg_stater {

namespace concepts {

template <typename T, typename StateT>
concept StateStorage = State<StateT> && requires(T& s, ChatIdT id) {
    typename T::StateT;
    requires std::same_as<typename T::StateT, StateT>;

    { s[id] } -> std::same_as<StateT*>; // pointer here represents a non-const nullable reference (optional<StateT&>)
    { s.get(id) } -> std::same_as<StateT*>; // same as s[id]
    { s.erase(id) } -> std::same_as<void>;
    { s.put(id, std::declval<const StateT&>()) } -> std::same_as<StateT&>;
    { s.put(id, std::declval<StateT&&>()) } -> std::same_as<StateT&>;
};

} // namespace concepts

template <typename StorageT>
    requires concepts::StateStorage<StorageT, typename StorageT::StateT>
class StateProxy {
    using StateT = StorageT::StateT;

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
        return storage.get().put(chatId, state);
    }

    decltype(auto) put(StateT&& state) const {
        return storage.get().put(chatId, std::move(state));
    }
};

} // namespace tg_stater
#endif // INCLUDE_bot_lib_state_storage_common
