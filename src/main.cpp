#include "ctf.h"
#include "ctf.hpp"
#include "dpp/dpp.h"
#include <string_view>

std::string get_bot_token()
{
    if (auto env_token{std::getenv("BOT_TOKEN")})
        return env_token;

    std::string token;
    std::cin >> token;

    return token;
}

template <typename T, typename... Types>
std::optional<T> to_optional(const std::variant<Types...> &variant)
{
    if (std::holds_alternative<T>(variant))
    {
        return std::get<T>(variant);
    }
    else
    {
        return std::nullopt;
    }
}

template <typename T>
std::optional<T> to_optional_from_event(const dpp::slashcommand_t &event,
                                        const std::string &parameter_name)
{
    return to_optional<T>(event.get_parameter(parameter_name));
}

namespace slash_command
{
void create(const dpp::slashcommand_t &event)
{
    auto id = std::get<std::int64_t>(event.get_parameter("id"));

    CTF::Team team{
        .name = to_optional_from_event<std::string>(event, "team-name"),
        .password = to_optional_from_event<std::string>(event, "team-password"),
        .url = to_optional_from_event<std::string>(event, "team-link"),
        .other_info =
            to_optional_from_event<std::string>(event, "team-other-info"),
    };

    try
    {
        CTF::CTF ctf{id, team};

        event.reply(dpp::message(event.command.channel_id, ctf.to_embed()));
    }
    catch (CTF::CreationException &e)
    {
        event.reply(
            dpp::message("Error occurred while creating a CTF with ID = '" +
                         std::to_string(id) + "'\n" + e.what())
                .set_flags(dpp::m_ephemeral));
    }
}
} // namespace slash_command

int main(int argc, char **argv)
{
    dpp::cluster bot(get_bot_token());

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand(
        [](const dpp::slashcommand_t &event)
        {
            if (event.command.get_command_name() == "create")
                slash_command::create(event);
        });

    bool recreate_commands{};
    for (int i = 1; i < argc; i++)
    {
        if (std::string_view{argv[i]} == "--recreate-commands")
            recreate_commands = true;
    }

    bot.on_ready(
        [&bot, recreate_commands](const dpp::ready_t &)
        {
            if (dpp::run_once<struct register_bot_commands>() &&
                recreate_commands)
            {
                bot.global_bulk_command_create({
                    dpp::slashcommand("create", "Creates a CTF event",
                                      bot.me.id)
                        .add_option(dpp::command_option(
                            dpp::co_integer, "id",
                            "ID of the CTF on ctftime.org", true))
                        .add_option(
                            dpp::command_option(dpp::co_string, "team-name",
                                                "The name of your team."))
                        .add_option(dpp::command_option(
                            dpp::co_string, "team-password",
                            "The password required to join your "
                            "team."))
                        .add_option(dpp::command_option(dpp::co_string,
                                                        "team-link",
                                                        "The link one can use "
                                                        "to join your team."))
                        .add_option(dpp::command_option(
                            dpp::co_string, "team-other-info",
                            "Any information related to joining your "
                            "team.")),
                });
            }
        });

    bot.start(dpp::st_wait);
}
