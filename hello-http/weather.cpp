#include "weather.h"

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

std::string GetJsonByURI(const std::string& uri, const std::string& api_key) {
    Poco::URI uri_obj(uri);
    Poco::Net::HTTPResponse http_response;
    Poco::Net::HTTPClientSession client_session(uri_obj.getHost(), uri_obj.getPort());

    std::string path(uri_obj.getPathAndQuery());

    Poco::Net::HTTPRequest http_request(Poco::Net::HTTPRequest::HTTP_GET, path,
                                        Poco::Net::HTTPMessage::HTTP_1_1);
    http_request.add("X-Yandex-API-Key", api_key);
    client_session.sendRequest(http_request);

    std::istream& is = client_session.receiveResponse(http_response);
    auto status = http_response.getStatus();
    if (status != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK) {
        throw YandexAPIError(static_cast<int>(status), http_response.getReason());
    }
    std::ostringstream ss;
    Poco::StreamCopier::copyStream(is, ss);
    auto json = ss.str();
    return json;
}

class YandexForecaster : public IForecaster {
public:
    YandexForecaster(const std::string& api_key, const std::string& api_endpoint)
        : api_key_(api_key), api_endpoint_(api_endpoint) {
    }
    YandexForecaster() = delete;

    WeatherForecast ForecastWeather(std::optional<Location> where) override {
        auto uri = api_endpoint_;

        if (where.has_value()) {
            auto lat = std::to_string(where->lat);
            auto lon = std::to_string(where->lon);
            uri += "?lat=" + lat + "&lon=" + lon;
        }
        auto json = GetJsonByURI(uri, api_key_);
        Poco::JSON::Parser parser;
        auto result = parser.parse(json);

        auto object = result.extract<Poco::JSON::Object::Ptr>();
        auto fact_weather = object->get("fact");
        auto fact_weather_ptr = fact_weather.extract<Poco::JSON::Object::Ptr>();
        auto feels_like = fact_weather_ptr->get("feels_like");
        auto temp = fact_weather_ptr->get("temp");

        WeatherForecast forecast;
        forecast.feels_like = std::stod(feels_like.toString());
        forecast.temp = std::stod(temp.toString());

        return forecast;
    }

private:
    const std::string api_key_, api_endpoint_;
};

std::unique_ptr<IForecaster> CreateYandexForecaster(const std::string& api_key,
                                                    const std::string& api_endpoint) {
    return std::make_unique<YandexForecaster>(api_key, api_endpoint);
}