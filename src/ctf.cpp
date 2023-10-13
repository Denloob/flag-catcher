#include "ctf.hpp"
#include "dpp/utility.h"
#include <chrono>
#include <cstdint>
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <string>
#include <string_view>

constexpr std::string_view api_url{"https://ctftime.org/api/v1/events/"};

std::int64_t string_date_to_timestamp(const std::string &date)
{
    std::tm tm{};
    if (!strptime(date.c_str(), "%Y-%m-%dT%H:%M:%S%z", &tm))
        return 0;

    auto time = std::mktime(&tm);
    std::chrono::system_clock::time_point time_point =
        std::chrono::system_clock::from_time_t(time);

    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        time_point.time_since_epoch());

    return duration.count();
}

bool parse_json_endpoint(const std::string &endpoint, Json::Value *json,
                         std::string *errors = nullptr)
{
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    std::string responseData;

    curl_easy_setopt(
        curl, CURLOPT_WRITEFUNCTION,
        +[](void *contents, size_t size, size_t nmemb, void *userp)
        {
            size_t totalSize = size * nmemb;
            auto *responseData = static_cast<std::string *>(userp);
            responseData->append(static_cast<char *>(contents), totalSize);
            return totalSize;
        });

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

    CURLcode res{curl_easy_perform(curl)};
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        *errors = std::string{"cURL request error: "} + curl_easy_strerror(res);
        return false;
    }

    long http_code{};
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (res != CURLE_OK)
    {
        *errors = std::string{"cURL getinfo error: "} + curl_easy_strerror(res);
        return false;
    }

    if (http_code == 404)
    {
        *errors = "404: CTF Not Found.";
        return false;
    }

    if (http_code != 200)
    {
        *errors = "Couldn't get CTF information. HTTP code " +
                  std::to_string(http_code) + '.';
        return false;
    }

    Json::CharReaderBuilder builder;
    Json::CharReader *reader{builder.newCharReader()};

    return reader->parse(responseData.c_str(),
                         responseData.c_str() + responseData.size(), json,
                         errors);
}

CTF::CTF(std::int64_t id) : id{id}
{

    std::string endpoint{api_url};
    endpoint += std::to_string(id);
    endpoint += '/';

    std::string errors;
    Json::Value root;
    if (!parse_json_endpoint(endpoint, &root, &errors))
        throw CTFCreationException(errors);

    start = string_date_to_timestamp(root["start"].asString());
    finish = string_date_to_timestamp(root["finish"].asString());
    title = root["title"].asString();
    url = root["url"].asString();
    ctftime_url = root["ctftime_url"].asString();
}

std::int64_t CTF::get_duration_seconds() const
{
    return finish - start;
}

constexpr auto seconds_in_minute{60};
constexpr auto minutes_in_hour{60};
constexpr auto hours_in_day{24};

constexpr std::int64_t seconds_to_minutes(std::int64_t seconds)
{
    return seconds / seconds_in_minute;
}

constexpr std::int64_t seconds_to_hours(std::int64_t seconds)
{
    return seconds / seconds_in_minute / minutes_in_hour;
}

constexpr std::int64_t seconds_to_days(std::int64_t seconds)
{
    return seconds / seconds_in_minute / minutes_in_hour / hours_in_day;
}

std::string quantity_with_name(std::int64_t quantity, std::string_view word)
{
    std::string s;

    s += std::to_string(quantity);
    s += ' ';
    s += word;

    if (quantity > 1)
        s += 's';

    return s;
}

std::string seconds_to_human_string(std::int64_t duration)
{
    std::stringstream stream;

    if (auto days = seconds_to_days(duration); days > 0)
    {
        stream << quantity_with_name(days, "day");
    }

    auto minutes = seconds_to_minutes(duration) % minutes_in_hour;
    if (auto hours = seconds_to_hours(duration) % hours_in_day; hours > 0)
    {
        stream << (minutes > 0 ? ", " : " and ")
               << quantity_with_name(hours, "hour");
    }

    if (minutes > 0)
    {
        stream << " and " << quantity_with_name(minutes, "minute");
    }

    return stream.str();
}

std::string CTF::to_text() const
{
    using dpp::utility::time_format;

    std::stringstream stream;

    stream << "# " << title << '\n'
           << "CTFtime: <" << ctftime_url << ">\n"
           << "Website: <" << url << ">\n\n";

    stream << "Status: ";

    std::int64_t current_time = std::time(nullptr);

    if (current_time > finish)
    {
        stream << "Over. Ended "
               << dpp::utility::timestamp(finish, time_format::tf_relative_time)
               << '('
               << dpp::utility::timestamp(start, time_format::tf_short_date)
               << " - "
               << dpp::utility::timestamp(finish, time_format::tf_short_date)
               << ")\n";

        return stream.str();
    }

    if (current_time > start)
    {
        stream << "🔴 Live! Ends in "
               << dpp::utility::timestamp(finish, time_format::tf_relative_time)
               << " ("
               << dpp::utility::timestamp(finish, time_format::tf_long_datetime)
               << ")\n";

        return stream.str();
    }

    stream << "Starts in "
           << dpp::utility::timestamp(start, time_format::tf_relative_time)
           << " ("
           << dpp::utility::timestamp(start, time_format::tf_long_datetime)
           << ")\n"
           << "Will last `"
           << seconds_to_human_string(this->get_duration_seconds()) << "`.\n"
           << "Ends on "
           << dpp::utility::timestamp(finish, time_format::tf_long_datetime);

    return stream.str();
}

CTFCreationException::CTFCreationException(const std::string &message)
    : std::runtime_error{message}
{
}
