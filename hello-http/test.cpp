#include <catch.hpp>

#include <fake/fake_server.h>
#include <weather.h>

TEST_CASE("Get current weather") {
    FakeServer server{"moscow weather"};
    server.Start();

    auto obj = CreateYandexForecaster(kTestApiKey, server.GetUrl());
    auto forecast = obj->ForecastWeather();

    REQUIRE(forecast.temp == -1);
    REQUIRE(forecast.feels_like == -5);
}

TEST_CASE("Lat-lon test") {
    FakeServer server{"spb weather"};
    server.Start();

    auto obj = CreateYandexForecaster(kTestApiKey, server.GetUrl());
    auto forecast = obj->ForecastWeather(Location{59.6, 30.2});

    REQUIRE(forecast.temp == 2);
    REQUIRE(forecast.feels_like == -6);
}

TEST_CASE("error") {
    FakeServer server{"error"};
    server.Start();

    auto obj = CreateYandexForecaster(kTestApiKey, server.GetUrl());
    REQUIRE_THROWS_AS(obj->ForecastWeather(), YandexAPIError);
}
