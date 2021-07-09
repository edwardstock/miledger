/*!
 * miledger.
 * ws_messages.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/api/ws_messages.h"

#include <restinio/string_view.hpp>

const std::unordered_map<miledger::ws_message::type_t, std::string> miledger::ws_message::map = {
    {miledger::ws_message::event_device_state_changed, "event_device_state_changed"},
    {miledger::ws_message::event_user_action_required, "event_user_action_required"},
    {miledger::ws_message::event_user_action_result, "event_user_action_result"},
    {miledger::ws_message::event_error, "event_error"},
    {miledger::ws_message::action_get_address, "action_get_address"},
    {miledger::ws_message::result_get_address, "result_get_address"},
    {miledger::ws_message::action_sign_tx, "action_sign_tx"},
    {miledger::ws_message::result_sign_tx, "result_sign_tx"},
    {miledger::ws_message::action_get_device_state, "action_get_device_state"},
    {miledger::ws_message::result_get_device_state, "result_get_device_state"},
};

const std::vector<miledger::ws_message::type_t> miledger::ws_message::action_types = {
    miledger::ws_message::action_get_address,
    miledger::ws_message::action_sign_tx,
    miledger::ws_message::action_get_device_state};

miledger::ws_message::type_t miledger::ws_message::type_from_string(const std::string& type) {
    auto res = std::find_if(map.begin(), map.end(), [&type](std::pair<ws_message::type_t, std::string> it) {
        if (it.second == type) {
            return true;
        }
        return false;
    });
    if (res != map.end()) {
        return res->first;
    }
    throw std::runtime_error(fmt::format("unknown message type: {0}", type));
}

std::string miledger::ws_message::type_to_string(miledger::ws_message::type_t t) {
    if (!map.count(t)) {
        return "<unknown>";
    }

    return map.at(t);
}
