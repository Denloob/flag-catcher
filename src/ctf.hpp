#pragma once

#include <cstdint>
#include <ctime>
#include <string>

struct CTF
{
    std::int64_t id{};
    std::time_t start{};
    std::time_t finish{};
    std::string title{};

    explicit CTF(std::int64_t id);

    std::int64_t get_duration_seconds() const;

    std::string to_text() const;
};
