/*#include "db.hpp"*/
#include <tg_stater/bot.hpp>
#include <tg_stater/handler/event.hpp>
#include <tg_stater/handler/handler.hpp>
#include <tg_stater/handler/type.hpp>
#include <tg_stater/state_storage/common.hpp>
#include <tg_stater/state_storage/memory.hpp>

/*#include <sqlite_orm/sqlite_orm.h>*/
#include <tgbot/Bot.h>

#include <format>
#include <fstream>
#include <ostream>
#include <print>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace TgBot {
class Message;
class Api;
} // namespace TgBot

struct Confirmation {
    std::vector<std::string> v;
};

struct Selection {
    std::vector<std::string> v;
};

using State = std::variant<Confirmation, Selection>;

int main() {
    using tg_stater::Handler, tg_stater::Events, tg_stater::HandlerTypes;
    using TgBot::Api, TgBot::Message;
    using StateStorage = tg_stater::StateProxy<tg_stater::MemoryStateStorage<State>>;

    std::string token;
    std::ifstream{".env"} >> token;
    TgBot::Bot bot{token};

    struct Dep {
        mutable int x = 0;
        void f() const {
            x++;
        }
    };
    auto default_ = [](const Message& m, const Api& bot, const StateStorage& ss) {
        std::println("{}\n{}", m.from->id, m.chat->id);
        bot.sendMessage(m.chat->id, "Enter items. Type \"done\" when finish.");
        ss.put(Selection{});
    };
    auto always = [](const Message& /*m*/, const Dep& d) { d.f(); };
    auto select = [](Selection& state, const Message& m, const Api& bot, const StateStorage& ss) {
        if (m.text != "done")
            state.v.push_back(m.text);
        std::ostringstream output;
        for (const auto& s : state.v)
            std::println(output, "- {}", s);
        bot.sendMessage(m.chat->id, "You selected:\n" + output.str());
        if (m.text == "done") {
            bot.sendMessage(m.chat->id, "Confirm with \"Yes\"");
            ss.put(Confirmation{std::move(state.v)});
        }
    };
    auto confirm = [](Confirmation& state, const Message& m, const Api& bot, const StateStorage& ss, const Dep& d) {
        bot.sendMessage(m.chat->id, m.text == "Yes" ? "Selected " + std::to_string(state.v.size()) : "All again");
        bot.sendMessage(m.chat->id, "Enter items. Type \"done\" when finish." + std::to_string(d.x));
        ss.put(Selection{});
    };
    static constexpr const char help_cmd[] = "help"; // NOLINT(*-c-arrays)
    static constexpr const char d_cmd[] = "d";       // NOLINT(*-c-arrays)
    auto help = [](const Message& m, const Api& bot) { bot.sendMessage(m.chat->id, "This is Selecter bot."); };
    auto d = [](const Message& m, const Api& bot, const Dep& d) {
        bot.sendMessage(m.chat->id, std::format("{} messages", d.x));
    };

    // NOLINTBEGIN(*-decay)
    tg_stater::Setup<State, Dep>::Stater<Handler<Events::Message{}, default_, HandlerTypes::NoState{}>,
                                         Handler<Events::AnyMessage{}, always, HandlerTypes::AnyState{}>,
                                         Handler<Events::Message{}, select>,
                                         Handler<Events::Message{}, confirm>,
                                         Handler<Events::Command{help_cmd}, help, HandlerTypes::AnyState{}>,
                                         Handler<Events::Command{help_cmd}, help, HandlerTypes::AnyState{}>,
                                         Handler<Events::UnknownCommand{}, help, HandlerTypes::AnyState{}>,
                                         Handler<Events::Command{d_cmd}, d, HandlerTypes::AnyState{}>>
        stater{};
    // NOLINTEND(*-decay)
    stater.start(std::move(bot));
}
