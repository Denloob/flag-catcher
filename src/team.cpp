#include "team.hpp"

std::string Team::to_string() const
{
    std::string s;

    if (name)
        s += "Name: `" + name.value() + "`\n";

    if (password)
        s += "Password: `" + password.value() + "`\n";

    if (url)
        s += "URL: [link](" + url.value() + ")\n";

    if (other_info)
        s += other_info.value() + "\n";

    return s;
}
