#include "SQLiteCpp/SQLiteCpp.h"
#include "ctf.hpp"
#include "db.hpp"
#include "dpp/dpp.h"
#include <string_view>

namespace CTF
{
DB::DB(std::string_view file_name)
    : database(file_name, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
    SQLite::Statement query(database, "CREATE TABLE IF NOT EXISTS ctf_data ("
                                      "id INTEGER PRIMARY KEY,"
                                      "channel_id INTEGER,"
                                      "ctf_id INTEGER,"
                                      "status INTEGER,"
                                      "start INTEGER,"
                                      "finish INTEGER,"
                                      "title TEXT,"
                                      "url TEXT,"
                                      "ctftime_url TEXT,"
                                      "team_name TEXT NULL,"
                                      "team_password TEXT NULL,"
                                      "team_link TEXT NULL,"
                                      "team_other_info TEXT NULL"
                                      ")");
    query.exec();
}
void DB::insert_ctf(const CTF &ctf, dpp::snowflake message_id,
                    dpp::snowflake channel_id) const
{

    SQLite::Statement insert(
        database,
        "INSERT INTO ctf_data (id, channel_id, ctf_id, status, start, "
        "finish, "
        "title, url, ctftime_url, "
        "team_name, team_password, team_link, team_other_info) "
        "VALUES "
        "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    insert.bind(1, static_cast<std::int64_t>(message_id));
    insert.bind(2, static_cast<std::int64_t>(channel_id));
    insert.bind(3, ctf.id);
    insert.bind(4, static_cast<int>(ctf.get_status()));
    insert.bind(5, ctf.start);
    insert.bind(6, ctf.finish);
    insert.bind(7, ctf.title);
    insert.bind(8, ctf.url);
    insert.bind(9, ctf.ctftime_url);
    insert.bind(10, ctf.team.name ? ctf.team.name.value().c_str() : nullptr);
    insert.bind(11, ctf.team.password ? ctf.team.password.value().c_str()
                                      : nullptr);
    insert.bind(12, ctf.team.url ? ctf.team.url.value().c_str() : nullptr);
    insert.bind(13, ctf.team.other_info ? ctf.team.other_info.value().c_str()
                                        : nullptr);

    insert.exec();
}

void DB::remove_ctf(dpp::snowflake id) const
{
    SQLite::Statement query(database, "DELETE FROM ctf_data WHERE id = ?");
    query.bind(1, static_cast<std::int64_t>(id));

    query.exec();
}

CTF get_ctf_from_query(const SQLite::Statement &query)
{
    CTF ctf;

    ctf.id = query.getColumn(2);
    ctf.start = query.getColumn(4);
    ctf.finish = query.getColumn(5);
    ctf.title = query.getColumn(6).getString();
    ctf.url = query.getColumn(7).getString();
    ctf.ctftime_url = query.getColumn(8).getString();

    if (query.getColumn(9).isText())
        ctf.team.name = query.getColumn(9).getString();
    if (query.getColumn(10).isText())
        ctf.team.password = query.getColumn(10).getString();
    if (query.getColumn(11).isText())
        ctf.team.url = query.getColumn(11).getString();
    if (query.getColumn(12).isText())
        ctf.team.other_info = query.getColumn(12).getString();

    return ctf;
}

CTF DB::get_ctf(std::int64_t id) const
{
    SQLite::Statement query(database, "SELECT * FROM ctf_data WHERE id = ?");
    query.bind(1, id);

    if (!query.executeStep())
        throw CreationException("CTF not found in the database.");

    return get_ctf_from_query(query);
}

void DB::remove_deleted_ctfs(dpp::cluster &bot) const
{
    SQLite::Statement query(database, "SELECT * FROM ctf_data");
    while (query.executeStep())
    {
        const std::int64_t id{query.getColumn(0)};
        const std::int64_t channel_id{query.getColumn(1)};

        bot.message_get(id, channel_id,
                        [this, id](const dpp::confirmation_callback_t &cc)
                        {
                            if (!cc.is_error())
                                return;

                            SQLite::Statement delete_query(
                                database, "DELETE FROM ctf_data WHERE id = ?");
                            delete_query.bind(1, id);
                            delete_query.exec();
                        });
    }
}

void update_ctf(dpp::cluster &bot, std::int64_t id, std::int64_t channel_id,
                const CTF &ctf)
{
    bot.message_get(
        id, channel_id,
        [ctf, &bot](const dpp::confirmation_callback_t &cc)
        {
            if (cc.is_error())
                return;

            auto ctf_embed = ctf.to_embed();

            auto message = cc.get<dpp::message>();
            message.embeds = {ctf_embed};

            bot.message_edit(message);

            constexpr std::string_view message_link_prefix{
                "https://discord.com/channels/"};
            auto message_link{
                std::string{message_link_prefix} + message.guild_id.str() +
                '/' + message.channel_id.str() + '/' + message.id.str()};

            auto notification_embed =
                dpp::embed()
                    .set_title("**" + ctf.title + "** is starting now!")
                    .set_description("[**Event Details**](" + message_link +
                                     ')')
                    .set_color(ctf_embed.color);
            bot.message_create(
                dpp::message(message.channel_id, notification_embed));
        });
}

/**
 * @warning Never to pass `field_name` user-copntrolled string.
 *          Will lead to an SQL injection.
 */
void DB::update_ctf_type(dpp::cluster &bot, CTF::CTF::Status current_status,
                         std::string_view field_name) const
{
    using Status = CTF::CTF::Status;

    SQLite::Statement query(database,
                            "SELECT * FROM ctf_data WHERE status = ? AND " +
                                std::string{field_name} + " < ?");
    query.bind(1, static_cast<int>(current_status));
    query.bind(2, std::time(nullptr));

    while (query.executeStep())
    {
        CTF ctf{get_ctf_from_query(query)};
        std::int64_t message_id{query.getColumn(0)};
        std::int64_t channel_id{query.getColumn(1)};

        update_ctf(bot, message_id, channel_id, ctf);
    }

    SQLite::Statement update(
        database, "UPDATE ctf_data SET status = ? WHERE status = ? AND " +
                      std::string{field_name} + " < ?");
    update.bind(1, static_cast<int>(current_status) + 1);
    update.bind(2, static_cast<int>(current_status));
    update.bind(3, std::time(nullptr));
    update.exec();
}

void DB::remove_finished_ctfs() const
{
    SQLite::Statement query(database, "DELETE FROM ctf_data WHERE status = ?");
    query.bind(1, static_cast<int>(CTF::CTF::Status::over));

    query.exec();
}

void DB::update_ctfs(dpp::cluster &bot) const
{
    update_ctf_type(bot, CTF::CTF::Status::soon, "start");
    update_ctf_type(bot, CTF::CTF::Status::live, "finish");
}

void DB::cleanup_ctfs(dpp::cluster &bot) const
{
    remove_finished_ctfs();
    remove_deleted_ctfs(bot);
}
} // namespace CTF
