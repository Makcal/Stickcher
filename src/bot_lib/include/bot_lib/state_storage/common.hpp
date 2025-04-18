#ifndef INCLUDE_bot_lib_state_storage_common
#define INCLUDE_bot_lib_state_storage_common

#include "bot_lib/tg_types.hpp"
#include "bot_lib/state.hpp"

#include <concepts>
#include <cstddef>
#include <format>
#include <functional>
#include <optional>
#include <ostream>
#include <utility>

namespace tg_stater {

struct StateKey {
    // If there is no chat, fallback to userId.
    // If the bot needs to distinguish users in a chat, the developer can implement it by themself.
    ChatUserIdT chatId;
    std::optional<ThreadIdT> threadId = std::nullopt;

    bool operator==(StateKey o) const {
        return chatId == o.chatId && threadId == o.threadId;
    }
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

    [[nodiscard]] decltype(auto) get() const {
        return std::as_const(storage).get(key);
    }

    [[nodiscard]] decltype(auto) operator[]() const {
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

template<>
struct std::hash<tg_stater::StateKey> {
    std::size_t operator()(const tg_stater::StateKey& key) const {
        std::size_t h1 = hash<decltype(key.chatId)>{}(key.chatId);
        std::size_t h2 = hash<decltype(key.threadId)>{}(key.threadId);
        return h1 ^ (h2 << 1U);
    }
};

template<>
struct std::formatter<tg_stater::StateKey> {
    template<class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}')
            throw std::format_error("Only empty format args are allowed for StateKey.");
        return it;
    }

    template<class FmtContext>
    FmtContext::iterator format(const tg_stater::StateKey& key, FmtContext& ctx) const
    {
        std::ostringstream out;
        out << "{chatId=" << key.chatId;
        if (key.threadId) {
            out << ", threadId=" << *key.threadId;
        }
        out << '}';
        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

#endif // INCLUDE_bot_lib_state_storage_common
