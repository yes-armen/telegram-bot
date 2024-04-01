# bot

In this task you need to write a telegram bot using the POCO library.
What exactly this bot will do is not so important. The assignment will evaluate the design and
implementation of a client for communicating with telegram servers.

In this task you have to create .h and .cpp files in the telegram directory,
and also test your code yourself by filling out the tests in test/test_api.cpp

## Plan

0. Solve the problem [hello-http](.../hello-http).
1. Get a token to work with the API.
2. Access the API from the console using the curl utility.
3. Read the documentation for the relevant classes from POCO.
4. Install dependencies, make sure everything builds and works.
5. Think about what classes you will need to write. What kind of interface will they have?
6. Add tests that work with FakeServer.
7. Write the bot logic.

## Introducing the HTTP API

he HTTP protocol for communicating with servers is described
on [this]( https://core.telegram.org/bots/api ) page.

To use the HTTP API
you need to [get a token]( https://core.telegram.org/bots#6-botfather ).

After you have received the token, you need to check that the HTTP API
works. To do this, we will pull the methods with the `curl` command. More for us
you will need the `json_pp` utility.

* [Method]( https://core.telegram.org/bots/api#getme ) `/getMe`
returns information about your bot. It is convenient to use for
token verification.


```
~/C/s/bot (master|…) $ curl https://api.telegram.org/bot<YOUR_TOKEN_HERE>/getMe
{"ok":true,"result":{"id":384306257,"is_bot":true,"first_name":"shad-cpp-test","username":"shad_shad_test_test_bot"}}
```

* [Method]( https://core.telegram.org/bots/api#getting-updates ) `/getUpdates` returns messages sent to your bot.

- For this method to return something meaningful, you need to send
message to your bot.
- Pay attention to the update_id parameter.
- Why does the command return the same message when repeated?
launch? How telegram servers understand what your bot has processed
messages?

```

~/C/s/bot (master|…) $ curl -s https://api.telegram.org/bot<YOUR_TOKEN_HERE>/getUpdates | json_pp
{
   "ok" : true,
   "result" : [
      {
         "message" : {
            "date" : 1510493105,
            "entities" : [
               {
                  "length" : 6,
                  "offset" : 0,
                  "type" : "bot_command"
               }
            ],
            "from" : {
               "is_bot" : false,
               "username" : "darth_slon",
               "id" : 104519755,
               "first_name" : "Fedor"
            },
            "chat" : {
               "username" : "darth_slon",
               "type" : "private",
               "first_name" : "Fedor",
               "id" : 104519755
            },
            "text" : "/start",
            "message_id" : 1
         },
         "update_id" : 851793506
      }
   ]
}
```

 * [Метод](https://core.telegram.org/bots/api#sendmessage)
   `/sendMessage` позволяет послать сообщение.

```
curl -s -H "Content-Type: application/json" -X POST -d '{"chat_id": <CHAT_ID_FROM_GET_UPDATES>, "text": "Hi!"}' https://api.telegram.org/bot<YOUR_TOKEN_HERE>/sendMessage | json_pp
{
   "ok" : true,
   "result" : {
      "chat" : {
         "type" : "private",
         "id" : 104519755,
         "username" : "darth_slon",
         "first_name" : "Fedor"
      },
      "message_id" : 5,
      "from" : {
         "is_bot" : true,
         "first_name" : "shad-cpp-test",
         "id" : 384306257,
         "username" : "shad_shad_test_test_bot"
      },
      "date" : 1510500325,
      "text" : "Hi!"
   }
}
```

* Run all three examples with the -v flag to see raw HTTP requests and responses.

## Requirements for your Client

 * All client methods are strongly typed. Poco::Json is not exposed in the interface.

 * HTTP-API errors are translated into exceptions.

 * The client saves the current offset to a file and restores it after a restart.

 * The client does not know about the logic of your specific bot.

 * Constants can be configured. Reasonable default values are set for all parameters, so only the necessary parameters need to be provided to use the Client.

## What the bot should do

 * Request /random. The bot sends a random number in response to this message.

 * Request /weather. The bot responds in the chat Winter Is Coming.

 * Request /styleguide. The bot responds in the chat with a funny joke on the topic of code review.

 * Request /stop. The bot process terminates normally.

 * Request /crash. The bot process terminates abnormally. For example, it executes abort();.

*Please* do not add bots to the general course chat to avoid unnecessary spam.
To show off your bot, you can use [a special channel](https://t.me/joinchat/BjrYSxBxPwTTonfSoku7xQ)

### Handling the offset parameter and long poll

You need to understand how the offset and timeout parameters work in the getUpdates method.
During operation, the bot should save its state to a file so that after stopping or crashing, user requests are not processed a second time.

To test this functionality, use the /stop and /crash requests, as well as a supervisor for the poor:

```
$ while true; do ./bot; done
```

## Useful classes from the POCO library

 - Poco::Net::HTTPClientSession
 - Poco::Net::HTTPSClientSession
 - Poco::Net::HTTPRequest
 - Poco::Net::HTTPResponse
 - Poco::URI
 - Poco::JSON::Object
 - Poco::JSON::Parser

## Installing dependencies

 * On Ubuntu

```
sudo apt-get install libpoco-dev
```

 * On MacOS
```
brew install poco --build-from-source --cc=gcc-8
```
## Automated tests

Interacting with a real server in tests is not the best idea for many reasons.
Instead, we provide you with the FakeServer class and ask you to write functional tests that check a set of scenarios.

FakeServer pretends to be a server and responds to all requests with pre-prepared responses.

Your test code should look something like this:

```
TEST_CASE("Single getMe") {
    FakeServer fake("Single getMe");
    fake.Start();

    // Your code here. Doing requests to fake.GetUrl();

    fake.StopAndCheckExpectations();
}
```

You need to check 4 scenarios in total. See the full description in the adjacent file (TESTING_SCENARIOS.md).

**The test server supports only a subset of the API. Your requests should look exactly as described in the scenario.**
