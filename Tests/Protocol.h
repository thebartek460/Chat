#pragma once
#pragma once
#include <string>

namespace ChatShared {

    // Struktura reprezentuj¹ca wiadomoœæ
    struct Message {
        std::string type; // Typ wiadomoœci: "login", "message", "join_room"
        std::string from; // Nick nadawcy
        std::string to;   // "all" albo konkretny nick
        std::string text; // Has³o (login) lub treœæ wiadomoœci / numer pokoju
    };

    // Zamiana wiadomoœci na "JSON-like" string
    inline std::string messageToJson(const Message& msg) {
        std::string json = "{";
        json += "\"type\":\"" + msg.type + "\",";
        json += "\"from\":\"" + msg.from + "\",";
        json += "\"to\":\"" + msg.to + "\",";
        json += "\"text\":\"" + msg.text + "\"";
        json += "}";
        return json;
    }

    // Pomocnicza funkcja do parsowania pola z JSON-like string
    inline std::string extractField(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return "";

        pos = json.find(":", pos);
        if (pos == std::string::npos) return "";

        size_t startQuote = json.find("\"", pos);
        if (startQuote == std::string::npos) return "";

        size_t endQuote = json.find("\"", startQuote + 1);
        if (endQuote == std::string::npos) return "";

        return json.substr(startQuote + 1, endQuote - startQuote - 1);
    }

} // namespace ChatShared
