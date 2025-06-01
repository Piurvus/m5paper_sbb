#pragma once
#include <WiFi.h>
#include <HTTPClient.h>


class Crawler {
public:
    Crawler();
    ~Crawler();

    bool fetch(String &addr, String* payload);

    //void add_address(String &addr);

private:
    //std::vector<String> addresses;
};