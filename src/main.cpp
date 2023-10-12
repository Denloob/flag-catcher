#include "dpp/dpp.h"
#include <cstdlib>
#include <string_view>

std::string get_bot_token()
{
    if (auto env_token{std::getenv("BOT_TOKEN")})
        return env_token;

    std::string token;
    std::cin >> token;

    return token;
}

int main(int argc, char **argv)
{
    dpp::cluster bot(get_bot_token());

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand(
        [](const dpp::slashcommand_t &event)
        {
            if (event.command.get_command_name() == "ping")
            {
                event.reply("Pong!");
            }
        });

    bot.on_ready(
        [&bot](const dpp::ready_t &)
        {
            if (dpp::run_once<struct register_bot_commands>())
            {
                bot.global_command_create(
                    dpp::slashcommand("ping", "Ping pong!", bot.me.id));
            }
        });

    bot.start(dpp::st_wait);
}
