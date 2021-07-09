/*!
 * miledger.
 * ws_messages.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_WS_MESSAGES_H
#define MILEDGER_WS_MESSAGES_H

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <restinio/string_view.hpp>
#include <restinio/websocket/message.hpp>
#include <unordered_map>

namespace miledger {

class ws_message {
public:
    enum type_t {
        // simple device state when it changed. this event triggers when state is changed or to new client who didn't receive it
        event_device_state_changed = (uint8_t) 0x00,
        // if client send message to sign transaction, ledger ask for user action (accepted or rejected)
        event_user_action_required,
        // result of user action: accepted or rejected
        event_user_action_result,

        // common error
        event_error,

        // request for getting ledger address
        action_get_address,
        // result with requested address
        result_get_address,

        // request for signing raw transaction
        action_sign_tx,
        // result with signed transaction
        result_sign_tx,

        // get current device state
        action_get_device_state,
        // result with device state
        result_get_device_state,

    };

    const static std::unordered_map<type_t, std::string> map;
    const static std::vector<type_t> action_types;

    static type_t type_from_string(const std::string& type);
    static std::string type_to_string(type_t t);

    static ws_message parse_message(const std::string& body, bool* success) {
        ws_message out;

        try {
            nlohmann::json j = nlohmann::json::parse(body);

            if (j.find("type") != j.end()) {
                out.type = type_from_string(j.at("type").get<std::string>());
            }
            if (j.find("value") != j.end()) {
                if (j.at("value").is_string()) {
                    out.value = j.at("value").get<std::string>();
                }
            }
            if (j.find("payload") != j.end()) {
                if (j.at("payload").is_object()) {
                    out.payload = j.at("payload").get<std::unordered_map<std::string, std::string>>();
                }
            }

            *success = true;
        } catch (const std::exception& e) {
            out.value = fmt::format("Unable to parse request message: {0}", std::string(e.what()));
            *success = false;
        }

        return out;
    }

    static ws_message parse_message(std::shared_ptr<restinio::websocket::basic::message_t> message, bool* success) {
        return parse_message(message->payload(), success);
    }

    ws_message()
        : type(type_t::event_error)
        , value("Unknown error")
        , payload() {
    }

    ws_message(type_t type)
        : type(type)
        , value()
        , payload() {
    }

    ws_message(type_t type, const std::string& value)
        : type(type)
        , value(value)
        , payload() {
    }

    ws_message(type_t type, const std::string& value, std::unordered_map<std::string, std::string> payload)
        : type(type)
        , value(value)
        , payload(payload) {
    }

    std::string to_string() const {
        nlohmann::json j;
        j["type"] = type_to_string(type);
        if (value.empty()) {
            j["value"] = nullptr;
        } else {
            j["value"] = value;
        }
        j["payload"] = payload;
        j["payload"]["type_code"] = type;

        return j.dump();
    }

    restinio::websocket::basic::message_t to_ws_message() const {
        restinio::websocket::basic::message_t out;
        out.set_opcode(restinio::websocket::basic::opcode_t::text_frame);
        out.set_payload(to_string());
        return out;
    }

    type_t type;
    std::string value;
    nlohmann::json payload;
};

} // namespace miledger

// clang-format off
NLOHMANN_JSON_SERIALIZE_ENUM(miledger::ws_message::type_t, {
    {miledger::ws_message::type_t::event_device_state_changed, "event_device_state_changed"},
    {miledger::ws_message::type_t::event_user_action_required, "event_user_action_required"},
    {miledger::ws_message::type_t::event_user_action_result, "event_user_action_result"},
    {miledger::ws_message::type_t::event_error, "event_error"},
    {miledger::ws_message::type_t::action_get_address, "action_get_address"},
    {miledger::ws_message::type_t::result_get_address, "result_get_address"},
    {miledger::ws_message::type_t::action_sign_tx, "action_sign_tx"},
    {miledger::ws_message::type_t::result_sign_tx, "result_sign_tx"},
    {miledger::ws_message::type_t::action_get_device_state, "action_get_device_state"},
    {miledger::ws_message::type_t::result_get_device_state, "result_get_device_state"},
    })
// clang-format on

#endif // MILEDGER_WS_MESSAGES_H
