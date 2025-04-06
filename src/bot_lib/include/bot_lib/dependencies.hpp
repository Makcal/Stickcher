#ifndef INCLUDE_bot_lib_handler_dependencies
#define INCLUDE_bot_lib_handler_dependencies

namespace tg_stater {

template <typename... Deps>
class Dependencies : public Deps... {};

} // namespace tg_stater

#endif // INCLUDE_bot_lib_handler_dependencies
