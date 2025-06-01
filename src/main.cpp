#include <M5EPD.h>
#include "crawler.h"
#include "transport_api.h"
#include <time.h>

M5EPD_Canvas top(&M5.EPD);
M5EPD_Canvas canvas(&M5.EPD);

Crawler* crawler = NULL;

const String Altstetten = "Altstetten";
const String Zurich = "Zürich";
const String Lenzburg = "Lenzburg";
const String Winterthur = "Winterthur";

const char* ntpServer = "pool.ntp.org";

API::Connection* conn1 = NULL;
API::Connection* conn2 = NULL;
API::Connection* conn3 = NULL;

void setup() {

    // SD card, SPI Serial Peripheral Interface...
    M5.begin(false, true,
               true, false,
               false);
    M5.EPD.SetRotation(90);
    M5.EPD.Clear(true);
    M5.RTC.begin();               
    M5.SHT30.Begin();
    //M5.GT911

    top.createCanvas(540, 100);
    top.setTextSize(3);        

    canvas.createCanvas(540, 960);
    canvas.fillCanvas(0);

    crawler = new Crawler();

    configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", ntpServer);
}

time_t lastupdate;

void loop() {
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo)) Serial.println("error");
    time_t unixTimestamp = mktime(&timeinfo);

    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);

        // CAREFUL WITH RATE LIMITING
        // LIMITED TO 1000 per day
    {
        String addr = API::build_connection_str(Altstetten, Zurich);
        String payload;
        if (!conn1 && crawler->fetch(addr,&payload)) {
            lastupdate = unixTimestamp;
            conn1 = API::parse_connections(payload);

            if (conn1) {
                API::print_connection(conn1);
                //delete conn;
            }

        } 
    }
    {
        String addr = API::build_connection_str(Altstetten, Lenzburg);
        String payload;
        if (!conn2 && crawler->fetch(addr,&payload)) {
            lastupdate = unixTimestamp;
            conn2 = API::parse_connections(payload);

            if (conn2) {
                API::print_connection(conn2);
                //delete conn;
            }

        } 
    }
    {
        String addr = API::build_connection_str(Altstetten, Winterthur);
        String payload;
        if (!conn3 && crawler->fetch(addr,&payload)) {
            lastupdate = unixTimestamp;
            conn3 = API::parse_connections(payload);

            if (conn3) {
                API::print_connection(conn3);
                //delete conn;
            }
        } 
    }

    {
        M5.SHT30.UpdateData();
        float hum = M5.SHT30.GetRelHumidity();
        float temp = M5.SHT30.GetTemperature();
        char temp_str[10], hum_str[10];

        dtostrf(temp, 2, 2, temp_str);
        dtostrf(hum, 2, 2, hum_str);
        
        top.fillCanvas(0);
        top.drawString("Temperature:" + String(temp_str) + "'C", 5, 5);
        top.drawString("Humidity:" + String(hum_str) + "%", 0, 55);

        top.pushCanvas(0, 0, UPDATE_MODE_DU4);
    }

    {
        conn1->canvas->pushCanvas(0, 100, UPDATE_MODE_DU4);
        for (int i = 0; i < conn1->conns.size() && i < 4; i++) {
            // missed
            if (conn1->conns.at(i)->departureTimestamp + conn1->conns.at(i)->delay*60 < unixTimestamp + 60*5) {
                Serial.printf("erasing it %d size %d", i, conn1->conns.size());
                delete conn1->conns.at(i);
                conn1->conns.erase(conn1->conns.begin()+i);                
                Serial.printf("erasing it %d size %d", i, conn1->conns.size());
                i-=1;
                continue;
            }
            conn1->conns.at(i)->canvas->pushCanvas(0, 100+50+80*i, UPDATE_MODE_DU4);

        }
        int size1 = (conn1->conns.size() < 4) ? conn1->conns.size() : 4;

        int start = 150+80*size1;
        conn2->canvas->pushCanvas(0, start, UPDATE_MODE_DU4);

        int size2 = (conn2->conns.size() < 2) ? conn2->conns.size() : 2;
        for (int i = 0; i < size2; i++) {
            if (conn2->conns.at(i)->departureTimestamp + conn2->conns.at(i)->delay*60 < unixTimestamp + 60*5) {
                delete conn2->conns.at(i);
                conn2->conns.erase(conn2->conns.begin()+i);                
                i-=1;
                continue;
            }
            conn2->conns.at(i)->canvas->pushCanvas(0, start+50+80*i, UPDATE_MODE_DU4);
        }

        size2 = (conn2->conns.size() < 2) ? conn2->conns.size() : 2;

        start += 80*size2 + 50;
        conn3->canvas->pushCanvas(0, start, UPDATE_MODE_DU4);

        int size3 = (conn3->conns.size() < 2) ? conn3->conns.size() : 2;
        for (int i = 0; i < size2; i++) {
            if (conn3->conns.at(i)->departureTimestamp + conn3->conns.at(i)->delay*60 < unixTimestamp + 60*5) {
                delete conn3->conns.at(i);
                conn3->conns.erase(conn3->conns.begin()+i);                
                i-=1;
                continue;
            }
            conn3->conns.at(i)->canvas->pushCanvas(0, start+50+80*i, UPDATE_MODE_DU4);
        }
    }

    // UPDATE
    if (conn1->conns.size() < 4 && lastupdate + 2*60 < unixTimestamp) {
        Serial.println("reloading conn1");
        delete conn1;
        conn1 = NULL;
    }
    else if (conn2->conns.size() < 2 && lastupdate + 2*60 < unixTimestamp) {
        Serial.println("reloading conn2");
        delete conn2;
        conn2 = NULL;
    } 
    else if (conn3->conns.size() < 2 && lastupdate + 2*60 < unixTimestamp) {
        Serial.println("reloading conn3");
        delete conn3;
        conn3 = NULL;
    }

    delay(1000 * 60);
}