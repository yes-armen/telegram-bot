#pragma once

#include "fake_data.h"

#include <iostream>

#include <Poco/URI.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>

static const std::string kTestApiKey = "abcdefgh";

class FakeHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest&,
                       Poco::Net::HTTPServerResponse& response) override {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.send();
    }
};

static bool ExpectMethod(Poco::Net::HTTPRequest& request, const std::string& method) {
    if (request.getMethod() != method) {
        std::cerr << "Invalid method: expected " << method;
        std::cerr << ", got " << request.getMethod() << "\n";
        return false;
    }
    return true;
}

static bool ExpectKey(Poco::Net::HTTPRequest& request, const std::string& key) {
    if (request.get("X-Yandex-API-Key", "") != key) {
        std::cerr << "Invalid api key: expected " << key << "\n";
        return false;
    }
    return true;
}

class DefaultHandler : public FakeHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override {
        Poco::URI uri{request.getURI()};
        auto good = (uri.getPath() == "/v1/forecast");
        good &= ExpectMethod(request, "GET");
        good &= ExpectKey(request, kTestApiKey);
        if (good) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            response.send() << fake_data::kMoscowWeather;
        } else {
            FakeHandler::handleRequest(request, response);
        }
    }
};

class LatLonHandler : public FakeHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override {
        auto good = ExpectMethod(request, "GET");
        good &= ExpectKey(request, kTestApiKey);
        Poco::URI uri{request.getURI()};
        auto params = uri.getQueryParameters();
        good &= (params.size() == 2);
        if (good) {
            std::ranges::sort(params);
            const auto& [lat, lat_v] = params[0];
            const auto& [lon, lon_v] = params[1];
            good &= (lat == "lat") && (lon == "lon");
            good &= (CheckValue(lat_v, 59.6) && CheckValue(lon_v, 30.2));
        }
        if (good) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            response.send() << fake_data::kSpbWeather;
        } else {
            FakeHandler::handleRequest(request, response);
        }
    }

private:
    static bool CheckValue(const std::string& actual, double expected) {
        try {
            return std::labs(std::stod(actual) - expected) < 1e-9;
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            return false;
        }
    }
};

class FakeHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    FakeHandlerFactory(std::string test) : test_{std::move(test)} {
    }

    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest&) {
        if (test_ == "error") {
            return new FakeHandler{};
        } else if (test_ == "moscow weather") {
            return new DefaultHandler{};
        } else if (test_ == "spb weather") {
            return new LatLonHandler{};
        } else {
            throw std::runtime_error{"Unknown test type"};
        }
    }

private:
    const std::string test_;
};

class FakeServer {
public:
    FakeServer(std::string test) : test_{std::move(test)} {
    }

    ~FakeServer() {
        Stop();
    }

    void Start() {
        Poco::Net::SocketAddress address{"localhost", 0};
        socket_ = std::make_unique<Poco::Net::ServerSocket>(address);
        auto* factory = new FakeHandlerFactory{test_};
        auto* params = new Poco::Net::HTTPServerParams;
        server_ = std::make_unique<Poco::Net::HTTPServer>(factory, *socket_, params);
        server_->start();
    }

    std::string GetUrl() const {
        Poco::URI uri;
        uri.setScheme("http");
        uri.setHost("localhost");
        uri.setPort(socket_->address().port());
        uri.setPath("/v1/forecast");
        return uri.toString();
    }

    void Stop() {
        if (server_) {
            server_->stop();
            server_.reset();
            socket_.reset();
        }
    }

private:
    std::unique_ptr<Poco::Net::ServerSocket> socket_;
    std::unique_ptr<Poco::Net::HTTPServer> server_;
    const std::string test_;
};
