#pragma once

#include <optional>
#include <string>

struct Team
{
    std::optional<std::string> name{};
    std::optional<std::string> password{};
    std::optional<std::string> url{};
    std::optional<std::string> other_info{};

    std::string to_string() const;
};
