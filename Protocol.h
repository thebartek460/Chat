#pragma once
#include <string>
// USUNIÊTO: #include <json.hpp> - to powodowa³o b³êdy!

namespace ChatShared {

    struct Message {
        std::string type; // "login", "join", "message", "private"
        std::string from;
        std::string to;   // kana³ lub nick
        std::string text;
    };

    // Zmieniamy typ zwracany z 'nlohmann::json' na zwyk³y 'std::string'
    inline std::string messageToJson(const Message& msg) {
        // Rêczne sklejanie tekstu, który wygl¹da jak JSON
        // Np.: {"type":"login","from":"Jan","to":"","text":""}

        std::string json = "{";
        json += "\"type\":\"" + msg.type + "\",";
        json += "\"from\":\"" + msg.from + "\",";
        json += "\"to\":\"" + msg.to + "\",";
        json += "\"text\":\"" + msg.text + "\"";
        json += "}";

        return json;
    }

} // namespace ChatShared