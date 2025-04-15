/*#include "db.hpp"*/
#include "bot_lib/bot.hpp"
#include "bot_lib/handler/event.hpp"
#include "bot_lib/handler/handler.hpp"
#include "bot_lib/handler/type.hpp"
#include "bot_lib/state_storage/common.hpp"
#include "bot_lib/state_storage/memory.hpp"

/*#include <sqlite_orm/sqlite_orm.h>*/
#include <tgbot/Bot.h>

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
    auto help = [](const Message& m, const Api& bot, const StateStorage&) {
        bot.sendMessage(m.chat->id, "This is Selecter bot.");
    };

    tg_stater::Setup<State, Dep>::Stater<Handler<Events::AnyMessage{}, default_, HandlerTypes::NoState{}>,
                                         Handler<Events::AnyMessage{}, always, HandlerTypes::AnyState{}>,
                                         Handler<Events::Message{}, select>,
                                         Handler<Events::Message{}, confirm>
                                         // Handler<Events::Command{&help_cmd[0]}, help>
                                         >
        stater{};
    stater.start(std::move(bot));
}
