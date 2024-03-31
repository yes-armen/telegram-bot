#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <sstream>
#include <exception>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

namespace telegram {
class Client {
public:
    Client(const std::string &api_endpoint, const std::string &api_key);
    ~Client();

    struct GetMeAnswer {
        uint64_t id;
    };

    GetMeAnswer GetMe();

    struct Update {
        int64_t update_id;
        int64_t chat_id;
        int64_t message_id;
        std::string message;
    };

    std::vector<Update> FetchUpdates(std::optional<int64_t> timeout = std::nullopt,
                                     std::optional<int64_t> offset = std::nullopt);

    void SendMessage(const std::string &message, int64_t chat_id,
                     std::optional<int64_t> reply_to_message_id = std::nullopt);

private:
    Poco::Dynamic::Var ProduceRequest(
        const std::string &key, const std::string &method,
        const Poco::JSON::Object &body_params = Poco::JSON::Object{},
        const std::unordered_map<std::string, std::string> &query_params = {},
        const std::string &http_method = Poco::Net::HTTPRequest::HTTP_GET);

    const std::string api_key_;
    std::unique_ptr<Poco::Net::HTTPClientSession> session_;
};
}  // namespace telegram
