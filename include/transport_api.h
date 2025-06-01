#pragma once
#include <WString.h>
#include <vector>
#include <M5EPD.h>

namespace API {
    struct Conn_info {
        ~Conn_info() {
            delete this->canvas;
        }
        // from
        int64_t departureTimestamp;
        int delay;
        String platform;

        int duration;
        int transfers;

        M5EPD_Canvas *canvas;
    };
    struct Connection {
        ~Connection() {
            for (auto conn : this->conns) {
                delete conn;
            }
            delete canvas;
        }

        String from;
        String to;

        M5EPD_Canvas *canvas;
        std::vector<Conn_info*> conns;
    };
    void print_connection(const Connection* conn);

    const String api_base = "http://transport.opendata.ch/v1/";

    String build_connection_str(const String&, const String&); 
    Connection* parse_connections(const String&);

    M5EPD_Canvas* build_conn_canvas(const Connection*);


};