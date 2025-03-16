/*#include "db.hpp"*/
#include "bot_lib/bot.hpp"
#include "bot_lib/handler.hpp"
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
} // namespace TgBot

struct Confirmation {
    std::vector<std::string> v;
};

struct Selection {
    std::vector<std::string> v;
};

using State = std::variant<Confirmation, Selection>;

int main() {
    using tg_stater::EnumStateHandler, tg_stater::NoStateHandler, tg_stater::AnyStateHandler,
        tg_stater::VariantStateHandler;
    using TgBot::Bot, TgBot::Message;
    using StateStorage = tg_stater::StateProxy<tg_stater::MemoryStateStorage<State>>;

    std::string token;
    std::ifstream{".env"} >> token;
    Bot bot{token};

    auto default_ = [](const Message& m, Bot& bot, const StateStorage& ss) {
        bot.getApi().sendMessage(m.chat->id, "Enter items. Type \"done\" when finish.");
        ss.put(Selection{});
    };
    auto always = [](const Message& m, Bot& bot, const StateStorage&) {
        bot.getApi().sendMessage(m.chat->id, "always");
    };
    auto select = [](Selection& state, const Message& m, Bot& bot, const StateStorage& ss) {
        if (m.text != "done")
            state.v.push_back(m.text);
        std::ostringstream output;
        for (const auto& s : state.v)
            std::println(output, "- {}", s);
        bot.getApi().sendMessage(m.chat->id, "You selected:\n" + output.str());
        if (m.text == "done") {
            bot.getApi().sendMessage(m.chat->id, "Confirm with \"Yes\"");
            ss.put(Confirmation{std::move(state.v)});
        }
    };
    auto confirm = [](Confirmation& state, const Message& m, Bot& bot, const StateStorage& ss) {
        bot.getApi().sendMessage(m.chat->id,
                                 m.text == "Yes" ? "Selected " + std::to_string(state.v.size()) : "All again");
        bot.getApi().sendMessage(m.chat->id, "Enter items. Type \"done\" when finish.");
        ss.put(Selection{});
    };

    tg_stater::DefaultStater<State,
                             NoStateHandler{default_},
                             AnyStateHandler{always},
                             VariantStateHandler{confirm},
                             VariantStateHandler{select}>
        stater{};
    stater.start(std::move(bot));
}
