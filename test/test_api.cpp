#include <fake/fake.h>

#include <catch.hpp>
#include "telegram/client.h"
#include <iostream>

TEST_CASE("Single getMe") {
    telegram::FakeServer fake{"Single getMe"};
    fake.Start();
    telegram::Client client(fake.GetUrl(), "bot123");

    client.GetMe();

    fake.StopAndCheckExpectations();
}

TEST_CASE("getMe error handling") {
    telegram::FakeServer fake{"getMe error handling"};
    fake.Start();

    telegram::Client client(fake.GetUrl(), "bot123");
    try {
        client.GetMe();
    } catch (const std::runtime_error &exception) {
    }
    try {
        client.GetMe();
    } catch (const std::runtime_error &exception) {
    }

    fake.StopAndCheckExpectations();
}

TEST_CASE("Single getUpdates and send messages") {
    telegram::FakeServer fake{"Single getUpdates and send messages"};
    fake.Start();
    telegram::Client client(fake.GetUrl(), "bot123");

    auto updates = client.FetchUpdates();
    client.SendMessage("Hi!", updates[0].chat_id);
    client.SendMessage("Reply", updates[1].chat_id, updates[1].message_id);
    client.SendMessage("Reply", updates[1].chat_id, updates[1].message_id);

    fake.StopAndCheckExpectations();
}

TEST_CASE("Handle getUpdates offset") {
    telegram::FakeServer fake{"Handle getUpdates offset"};
    fake.Start();

    telegram::Client client(fake.GetUrl(), "bot123");
    auto one = client.FetchUpdates(5);
    REQUIRE(one.size() == 2);
    auto max_upd_id = std::max(one[0].update_id, one[1].update_id);
    auto two = client.FetchUpdates(5, max_upd_id + 1);
    REQUIRE(two.empty());
    auto three = client.FetchUpdates(5, max_upd_id + 1);
    REQUIRE(three.size() == 1);

    fake.StopAndCheckExpectations();
}
