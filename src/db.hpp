#pragma once

#include "ctf.hpp"
#include "dpp/dpp.h"
#include "SQLiteCpp/SQLiteCpp.h"

namespace CTF
{
struct DB
{
    /**
     * @brief Opens a DB with the given file_name.
     *          Creates it if it doesn't exist and creates
     *          the required tables if necessary.
     *
     * @param file_name The name of the file where the db will be stored.
     */
    explicit DB(std::string_view file_name);

    /**
     * @brief Inserts a ctf into the database.
     *
     * @param ctf The CTF to insert.
     * @param id The ID of CTF's message.
     * @param channel_id The ID of the channel_id where the message was sent.
     */
    void insert_ctf(const CTF &ctf, dpp::snowflake message_id,
                    dpp::snowflake channel_id) const;

    /**
     * @brief Removes a CTF from the database.
     *
     * @param id The ID used during insertion (message_id).
     *
     * @see insert_ctf
     */
    void remove_ctf(dpp::snowflake id) const;

    /**
     * @brief Updates the status and the discord message of all the CTFs in the
     *          database.
     *
     * @param bot The bot (cluster) that sent and that will edit the CTF messages.
     */
    void update_ctfs(dpp::cluster &bot) const;

    /**
     * @brief Cleans up all the CTFs in the database that no longer needed.
     *          For example CTFs which are finished or CTFs whose message was
     *          deleted.
     *
     * @param bot The bot (cluster) to use for checking the messages.
     */
    void cleanup_ctfs(dpp::cluster &bot) const;

  private:
    SQLite::Database database;

    CTF get_ctf(std::int64_t id) const;

    /**
     * @brief Removes all the CTFs whose message was deleted.
     *
     * @param bot The bot (cluster) to use for checking the messages.
     */
    void remove_deleted_ctfs(dpp::cluster &bot) const;

    /**
     * @warning Never to pass `field_name` user-copntrolled string.
     *          Will lead to an SQL injection.
     */
    void update_ctf_type(dpp::cluster &bot, CTF::CTF::Status current_status,
                         std::string_view field_name) const;

    /**
     * @brief Removes all the finished CTFs from the database.
     */
    void remove_finished_ctfs() const;
};
} // namespace CTF
