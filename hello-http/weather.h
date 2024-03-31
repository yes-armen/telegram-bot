#pragma once

#include <string>
#include <memory>
#include <optional>
#include <stdexcept>
#include "fake/fake_server.h"
#include "fake/fake_data.h"

struct Location {
    double lat;
    double lon;
};

struct WeatherForecast {
    double temp;
    double feels_like;
};

struct YandexAPIError : public std::runtime_error {
    YandexAPIError(int http_code, const std::string& details)
        : std::runtime_error{"api error: code=" + std::to_string(http_code) +
                             " details=" + details},
          http_code{http_code},
          details{details} {
    }

    int http_code;
    std::string details;
};

class IForecaster {
public:
    virtual ~IForecaster() = default;
    virtual WeatherForecast ForecastWeather(std::optional<Location> where = std::nullopt) = 0;
};

std::unique_ptr<IForecaster> CreateYandexForecaster(
    const std::string& api_key,
    const std::string& api_endpoint = "http://api.weather.yandex.ru/v1/forecast");
