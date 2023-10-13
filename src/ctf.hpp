#pragma once

#include <cstdint>
#include <ctime>
#include <dpp/message.h>
#include <stdexcept>
#include <string>

struct CTF
{
    std::int64_t id{};
    std::time_t start{};
    std::time_t finish{};
    std::string title{};
    std::string url{};
    std::string ctftime_url{};

    explicit CTF(std::int64_t id);

    std::int64_t get_duration_seconds() const;

    std::string to_text() const;
    dpp::embed to_embed() const;
};

struct CTFCreationException : public std::runtime_error
{
    explicit CTFCreationException(const std::string &message);
};
