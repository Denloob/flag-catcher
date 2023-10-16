#pragma once

#include "team.hpp"
#include <cstdint>
#include <ctime>
#include <dpp/message.h>
#include <stdexcept>
#include <string>

namespace CTF
{
struct CTF
{
    std::int64_t id{};
    std::time_t start{};
    std::time_t finish{};
    std::string title{};
    std::string url{};
    std::string ctftime_url{};
    Team team{};

    enum class Status
    {
        soon,
        live,
        over,
    };

    CTF(std::int64_t id, const Team &team = {});

    std::int64_t get_duration_seconds() const;

    std::string to_text() const;
    dpp::embed to_embed() const;

    std::string to_google_event() const;

    Status get_status() const;
};

struct CreationException : public std::runtime_error
{
    explicit CreationException(const std::string &message);
};
} // namespace CTF
