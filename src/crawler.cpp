#include "crawler.h"
#include "credentials.h"

Crawler::Crawler() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(network::WIFI_SSID, network::WIFI_PW);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("** connected to WiFi");
    Serial.println(WiFi.localIP());
}



Crawler::~Crawler() {

}


bool Crawler::fetch(String &addr, String* payload) {
    HTTPClient http;
    bool success = false;

    http.begin(addr); 
    int httpCode = http.GET();

    if(httpCode > 0) {
        if(httpCode == HTTP_CODE_OK) {
            *payload = http.getString();
            success = true;
            goto done;
        } else {
            Serial.printf("[HTTP] httpcode is %d\n", httpCode);

            if (429 == httpCode) {
                Serial.println("TOO MANY REQUEST.. backoff");
                delay(60000 * 5);
            }
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

done:
    http.end();
    return success;
}