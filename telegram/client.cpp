#include "client.h"
#include <unordered_map>
#include <fstream>

Poco::Dynamic::Var telegram::Client::ProduceRequest(
    const std::string &key, const std::string &method, const Poco::JSON::Object &body_params,
    const std::unordered_map<std::string, std::string> &query_params,
    const std::string &http_method) {
    const std::string path = "/" + key + "/" + method;
    Poco::URI uri(path);
    Poco::JSON::Stringifier str;

    for (auto &[k, v] : query_params) {
        uri.addQueryParameter(k, v);
    }

    Poco::Net::HTTPRequest req(http_method, uri.getPathAndQuery(),
                               Poco::Net::HTTPMessage::HTTP_1_1);

    req.add("Content-Type", "application/json");
    std::ostringstream oss;
    str.condense(body_params, oss);
    req.add("Content-Length", std::to_string(oss.str().size()));

    auto &req_body = session_->sendRequest(req);
    str.condense(body_params, req_body);

    Poco::Net::HTTPResponse res;
    std::istream &is = session_->receiveResponse(res);

    if (auto status = res.getStatus(); status != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK) {
        throw std::runtime_error("error1");
    }

    Poco::JSON::Parser parser;
    return parser.parse(is);
}

telegram::Client::GetMeAnswer telegram::Client::GetMe() {
    auto resp = ProduceRequest(api_key_, "getMe");

    auto o = resp.extract<Poco::JSON::Object::Ptr>();
    auto res = o->getObject("result");
    auto ido = res->get("id");
    auto id = ido.convert<uint64_t>();

    return {id};
}

std::vector<telegram::Client::Update> telegram::Client::FetchUpdates(
    std::optional<int64_t> timeout, std::optional<int64_t> offset) {

    std::unordered_map<std::string, std::string> q;

    if (timeout.has_value()) {
        q.emplace("timeout", std::to_string(*timeout));
    }
    if (offset.has_value()) {
        q.emplace("offset", std::to_string(*offset));
    }

    auto resp = ProduceRequest(api_key_, "getUpdates", Poco::JSON::Object{}, q);
    std::vector<Update> result;
    auto object = resp.extract<Poco::JSON::Object::Ptr>();
    auto arr = object->getArray("result");
    for (auto &var : *arr) {
        Update update;
        auto entry_ptr = var.extract<Poco::JSON::Object::Ptr>();
        auto message = entry_ptr->get("message");
        if (message.isEmpty()) {
            continue;
        }
        auto message_ptr = message.extract<Poco::JSON::Object::Ptr>();
        auto tmp = entry_ptr->get("update_id");
        update.update_id = tmp.convert<int64_t>();
        auto tmp1 = message_ptr->getObject("chat");
        auto id_var = tmp1->get("id");
        update.chat_id = id_var.convert<int64_t>();
        auto tmp2 = message_ptr->get("message_id");
        update.message_id = tmp2.convert<int64_t>();
        auto tmp3 = message_ptr->get("text");
        if (!tmp3.isEmpty()) {
            update.message = tmp3.convert<std::string>();
        }

        result.push_back(std::move(update));
    }

    std::sort(result.begin(), result.end(),
              [](auto &&l, auto &&r) { return l.update_id < r.update_id; });

    return result;
}

void telegram::Client::SendMessage(const std::string &message, int64_t chat_id,
                                   std::optional<int64_t> reply_to_message_id) {
    Poco::JSON::Object params;

    params.set("text", message);
    params.set("chat_id", chat_id);

    if (reply_to_message_id.has_value()) {
        params.set("reply_to_message_id", *reply_to_message_id);
    }

    ProduceRequest(api_key_, "sendMessage", params, {}, Poco::Net::HTTPRequest::HTTP_POST);
}

telegram::Client::Client(const std::string &api_endpoint, const std::string &api_key)
    : api_key_(api_key) {

    Poco::URI uriobj(api_endpoint);
    if (api_endpoint.starts_with("https")) {
        session_ =
            std::make_unique<Poco::Net::HTTPSClientSession>(uriobj.getHost(), uriobj.getPort());
    } else {
        session_ =
            std::make_unique<Poco::Net::HTTPClientSession>(uriobj.getHost(), uriobj.getPort());
    }
}

telegram::Client::~Client() {
}
