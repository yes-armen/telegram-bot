# hello-http

This task was discussed at the POCO seminar. It is recommended to solve it before starting the [third homework assignment](../bot).

Implement the CreateYandexForecaster function, which returns the current weather using the [Yandex.Weather API](https://tech.yandex.com/weather/doc/dg/concepts/about-docpage/). This function takes an API access key and a URL to make requests to (used in tests).

The interfaces are provided in the `weather.h` file, implement everything necessary in weather.cpp. The implementation of the IForecaster interface allows specifying latitude and longitude to use when making a request. Otherwise, do not specify these parameters in the request (Yandex.Weather then substitutes values for Moscow).

If the server returns an error in response to the request, throw a YandexAPIError exception.

To test, you can make a request to the API in main.cpp (first you need to obtain an API key). The corresponding binary is called hello_http.

## Installing Dependencies

* On Ubuntu
```
sudo apt-get install libpoco-dev
```

* On MacOS
```
brew install poco
```

Switch the project build to the system clang.
