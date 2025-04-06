#ifndef INCLUDE_bot_lib_state_storage_common
#define INCLUDE_bot_lib_state_storage_common

#include "bot_lib/tg_types.hpp"
#include "bot_lib/state.hpp"

#include <concepts>
#include <functional>
#include <optional>
#include <utility>

namespace tg_stater {

// NOLINTNEXTLINE (*-pro-type-member-init)
struct StateKey {
    ChatUserIdT chatId; // if there is no chat, fallback to userId
    std::optional<ThreadIdT> threadId = std::nullopt;
};

namespace concepts {

template <typename T, typename StateT>
concept StateStorage = State<StateT> && requires(T& s, const StateKey& key) {
    typename T::StateT;
    requires std::same_as<typename T::StateT, StateT>;

    { s[key] } -> std::same_as<StateT*>; // pointer here represents a non-const nullable reference (optional<StateT&>)
    { s.erase(key) } -> std::same_as<void>;
    { s.put(key, std::declval<const StateT&>()) } -> std::same_as<StateT&>;
    { s.put(key, std::declval<StateT&&>()) } -> std::same_as<StateT&>;
};

} // namespace concepts

template <typename StorageT_>
    requires concepts::StateStorage<StorageT_, typename StorageT_::StateT>
class StateProxy {
  public:
    using StateT = StorageT_::StateT;
    using StorageT = StorageT_;

  private:
    std::reference_wrapper<StorageT> storage;
    StateKey key;

  public:
    explicit StateProxy(StorageT& storage, const StateKey& key) : storage{storage}, key{key} {}

    [[nodiscard]] const StateKey& getKey() const {
        return key;
    }

    decltype(auto) get() const {
        return storage.get().get(key);
    }

    decltype(auto) operator[]() const {
        return get();
    }

    void erase() const {
        storage.get().erase(key);
    }

    decltype(auto) put(const StateT& state) const {
        return storage.get().put(key, state);
    }

    decltype(auto) put(StateT&& state) const {
        return storage.get().put(key, std::move(state));
    }
};

} // namespace tg_stater
#endif // INCLUDE_bot_lib_state_storage_common
