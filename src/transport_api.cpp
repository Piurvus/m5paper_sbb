#include "transport_api.h"
#include "json_parser.h"
#include <M5EPD.h>
#include <ctime>

#define OK(f) do { if (f) { return NULL; } } while(0)
#define OK_CONN(f) do { if (err = f) { Serial.printf("errcode is %d for %s\n", err, #f);} } while(0)

String API::build_connection_str(
    const String &from, const String &to) {

    String url = API::api_base;
    url += "connections";

    url += "?from=" + from;
    url += "&to=" + to;

    return url;
}

API::Connection* API::parse_connections(const String &json) {
    char buf[64];

    jparse_ctx_t jparse_ctx;
    OK(json_parse_start(&jparse_ctx, json.c_str(), json.length()));

    char from_str[64];
    OK(json_obj_get_object(&jparse_ctx, "from"));
    OK(json_obj_get_string(&jparse_ctx, "name", from_str, sizeof(from_str)));
    OK(json_obj_leave_object(&jparse_ctx));

    char to_str[64];
    OK(json_obj_get_object(&jparse_ctx, "to"));
    OK(json_obj_get_string(&jparse_ctx, "name", to_str, sizeof(to_str)));
    OK(json_obj_leave_object(&jparse_ctx));

    int conns = -1;
    int err;
    OK(json_obj_get_array(&jparse_ctx, "connections", &conns));

    Connection* conn = new Connection();
    conn->from = String(from_str);
    conn->to = String(to_str);
    conn->from.replace("\\u00fc", "u");
    conn->to.replace("\\u00fc", "u");


    M5EPD_Canvas *canvas = new M5EPD_Canvas(&M5.EPD);
    canvas->createCanvas(540, 50);
    canvas->fillCanvas(0);
    canvas->setTextSize(4);        
    sprintf(buf, "%s", conn->to.c_str());
    canvas->fillRect(0, 0, 540, 5, 0xffffffff);
    canvas->drawString(buf, 5, 10);

    conn->canvas = canvas;

    if (conns > 0) {
        Serial.println("there are connections");

        for (int i = 0; i < conns; i++) {
            
            if (0 == json_arr_get_object(&jparse_ctx, i)) {
                int64_t timestamp = 0;
                int delay = 0;
                char platform[16] = "";
                char duration[32] = "";
                int transfers = 0;
            
                // from 
                if (0 == json_obj_get_object(&jparse_ctx, "from")) {
                    OK_CONN(json_obj_get_int64(&jparse_ctx, "departureTimestamp", &timestamp));
                    OK_CONN(json_obj_get_int(&jparse_ctx, "delay", &delay));
                    if (delay > 60) delay=0; // null causes issues..
                    OK_CONN(json_obj_get_string(&jparse_ctx, "platform", platform, sizeof(platform)));

                    OK_CONN(json_obj_leave_object(&jparse_ctx));
                }

                OK_CONN(json_obj_get_string(&jparse_ctx, "duration", duration, sizeof(duration)));
                OK_CONN(json_obj_get_int(&jparse_ctx, "transfers", &transfers));

                OK_CONN(json_arr_leave_object(&jparse_ctx));
                
                Conn_info* conn_info = new Conn_info();
                conn_info->delay= delay;
                conn_info->platform= String(platform);
                conn_info->departureTimestamp = timestamp;

                // Try to parse duration
                int mins, hours, non, dur = 0;
                if (sscanf(duration, "%02dd%02d:%02d:%02d", &non, &hours, &mins, &non)) {
                    dur = mins + 60*hours;
                } else {
                    Serial.printf("duration is %s\n", duration);
                }

                conn_info->duration = dur;
                conn_info->transfers = transfers;

                // Build Canvas
                time_t time_t_timestamp = static_cast<time_t>(timestamp);
                struct tm* timeinfo = gmtime(&time_t_timestamp);

                canvas = new M5EPD_Canvas(&M5.EPD);
                
                canvas->createCanvas(540, 80);
                canvas->fillCanvas(0);

                canvas->drawLine(0, 0, 540, 0, 0xffffffff);
                canvas->drawLine(0, 1, 540, 1, 0xffffffff);

                canvas->setTextSize(3);        
                sprintf(buf, "Departure: %02d:%02d +(%d)", timeinfo->tm_hour+2, timeinfo->tm_min, delay);
                canvas->drawString(buf, 5, 5);
                sprintf(buf, "%d#", transfers);
                canvas->drawString(buf, 420, 5);

                sprintf(buf, "Duration: %d min. ", dur);
                canvas->drawString(buf, 5, 40);
                sprintf(buf, "Gl. %s", platform);
                canvas->drawString(buf, 420, 40);

                conn_info->canvas = canvas;

                conn->conns.push_back(conn_info);
            }
        }
    }

    json_parse_end(&jparse_ctx);
    return conn;
}

void API::print_connection(const API::Connection* conn) {
    Serial.println("Connection:");
    Serial.printf("from: %s\n", conn->from.c_str());
    Serial.printf("to: %s\n", conn->to.c_str());

    for (auto c : conn->conns) {
        Serial.println("*************************");
        time_t time_t_timestamp = static_cast<time_t>(c->departureTimestamp);
        struct tm* timeinfo = gmtime(&time_t_timestamp);
        Serial.printf("Departure time: %d:%2d\n", timeinfo->tm_hour, timeinfo->tm_min);
        Serial.printf("takes %d minutes\n", c->duration);
        Serial.printf("delay is %d\n", delay);

        Serial.printf("transfers is %d\n", c->transfers);
        Serial.printf("platform is %s\n", c->platform.c_str());
    }
    Serial.println("*************************");
}


M5EPD_Canvas* API::build_conn_canvas(const API::Connection*) {
    return NULL;
}
