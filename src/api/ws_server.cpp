/*!
 * miledger.
 * web_server.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/api/ws_server.h"

#include "include/api/ws_messages.h"
#include "include/settings.h"

#include <chrono>
#include <minter/ledger/errors.h>
#include <restinio/router/easy_parser_router.hpp>
#include <restinio/router/express.hpp>

miledger::WsServer::WsServer(miledger::ConsoleApp* app, QObject* parent)
    : QObject(parent)
    , m_app(app)
    , m_server(nullptr)
    , m_isRunning(false)
    , m_isStarting(false)
    , m_isStopping(false) {

    connect(this, &miledger::WsServer::stopServer, this, &miledger::WsServer::stop);
    connect(this, &miledger::WsServer::startServer, this, &miledger::WsServer::run);
}

miledger::WsServer::~WsServer() {
    stop();
    delete m_server;
}

void miledger::WsServer::run() {
    if (m_isStarting.load(std::memory_order_acquire)) {
        qDebug() << "- server already starting...";
        return;
    }
    if (m_isRunning) {
        return;
    }
    m_isStarting = true;
    qDebug() << "Starting server...";

    m_server = new server_t{
        restinio::external_io_context(m_ctx),
        settings_t{}
            .port(Settings::get().getUint16(Settings::KEY_SERVER_PORT, 8081))
            .address(Settings::get().getString(Settings::KEY_SERVER_ADDRESS, "127.0.0.1").toStdString())
            .request_handler(requestHandler())
            .read_next_http_message_timelimit(std::chrono::seconds(30))
            .write_http_response_timelimit(std::chrono::seconds(30))
            .handle_request_timeout(std::chrono::seconds(30))
            .cleanup_func(std::bind(&WsServer::cleanupHandler, this))};

    restinio::asio_ns::post(m_ctx, [this] {
        m_server->open_sync();
        qDebug() << "Starting server... [DONE]";
    });
    connect(m_app, &ConsoleApp::deviceStateChanged, [this](dev_state state) {
        std::for_each(m_registry.begin(), m_registry.end(), [this, state](std::pair<uint64_t, rws::ws_handle_t> it) {
            miledger::ws_message res(ws_message::type_t::event_device_state_changed, DeviceServer::stateToString(state));

            sendMessage(it.first, res);
        });
    });
    m_isStarting = false;

    m_isRunning = true;
    io::error_code ec;
    m_ctx.run(ec);
    if (ec) {
        qDebug() << "Error while running io_context" << QString::fromStdString(ec.message());
    }
    m_isRunning = false;
}
void miledger::WsServer::stop() {
    if (!isRunning()) {
        return;
    }
    if (m_isStopping.load(std::memory_order_acquire)) {
        qDebug() << "- server already stopping...";
        return;
    }
    m_isStopping = true;
    qDebug() << "Stopping server...";
    if (m_server) {
        m_server->close_sync();
        delete m_server;
        m_server = nullptr;
    }

    m_ctx.stop();

    m_isStopping = false;
    qDebug() << "Stopping server... [DONE]";
}

bool miledger::WsServer::isRunning() const {
    return m_isRunning.load(std::memory_order_relaxed);
}

void miledger::WsServer::cleanupHandler() {
    m_registry.clear();
}

void miledger::WsServer::sendStatusMessage(uint64_t recipient, miledger::ws_message::type_t type, const std::string& message) {
    miledger::ws_message msg(type);
    msg.value = message;
    sendMessage(recipient, msg);
}

void miledger::WsServer::sendErrorMessage(uint64_t recipient, const std::string& error) {
    miledger::ws_message msg;
    msg.value = error;
    sendMessage(recipient, msg);
}

void miledger::WsServer::sendErrorMessage(uint64_t recipient, const std::string& error, nlohmann::json payload) {
    miledger::ws_message msg;
    msg.value = error;
    msg.payload = payload;
    sendMessage(recipient, msg);
}

void miledger::WsServer::sendMessage(uint64_t recipient, const miledger::ws_message& message) {
    if (!m_registry.count(recipient)) {
        return;
    }

    auto handle = m_registry.at(recipient);
    handle->send_message(message.to_ws_message());
}

restinio::request_handling_status_t miledger::WsServer::handleHttpRequest(
    miledger::ws_message::type_t type,
    std::shared_ptr<restinio::request_t> req,
    const restinio::router::route_params_t&) const {
    miledger::ws_message res;
    switch (type) {
    case ws_message::action_get_device_state:
        res = miledger::ws_message(ws_message::type_t::result_get_device_state, m_app->dev.getStateString());
        break;
    case ws_message::action_get_address: {
        if (!m_app->dev.canInteract()) {
            nlohmann::json pl;
            pl["state"] = m_app->dev.getStateString();
            res.value = fmt::format("Can't proceed request: device is in invalid state: {0}", m_app->dev.getStateString(), pl);
            break;
        }
        try {
            minter::address_t address = m_app->dev.getAddress(true).as_blocking().first();
            res = miledger::ws_message(ws_message::type_t::result_get_address, address.to_string());
        } catch (const std::exception& e) {
            res.value = std::string(e.what());
        }
    } break;
    case ws_message::action_sign_tx: {
        if (!m_app->dev.canInteract()) {
            nlohmann::json pl;
            pl["state"] = m_app->dev.getStateString();
            res.payload = pl;
            res.value = fmt::format("Can't proceed request: device is in invalid state: {0}", m_app->dev.getStateString());
            break;
        }
        miledger::net::request tmp("http://localhost");
        tmp.parse_query(QString::fromStdString(std::string(req->header().query())));

        if (!tmp.has_query("tx")) {
            res.value = "GET parameter tx is required";
            break;
        }
        std::string tx_val = tmp.get_query_value("tx").toStdString();
        if (tx_val.empty()) {
            res.value = "GET parameter tx has empty value. Value must be a hex-string with 32 bytes of raw unsigned tx data";
            break;
        }
        if (tx_val.size() != 64) {
            res.value = "GET parameter tx has invalid raw tx length. Value must be a hex-string with 32 bytes of raw unsigned tx data";
            break;
        }

        tb::bytes_data rawTx(tx_val);
        minter::signature signature;
        try {
            signature = m_app->dev.signTx(rawTx);

            ws_message signRes(ws_message::type_t::result_sign_tx);
            signRes.payload["r"] = tb::bytes_to_hex(signature.r.data(), signature.r.size());
            signRes.payload["s"] = tb::bytes_to_hex(signature.s.data(), signature.s.size());
            signRes.payload["v"] = tb::bytes_to_hex(signature.v.data(), signature.v.size());
            res = signRes;

        } catch (const minter::exchange_error& e) {
            ws_message msg(ws_message::type_t::event_user_action_result);
            msg.value = e.codeString();
            msg.payload["status_code"] = e.code();
            res = msg;
        } catch (const std::exception& e) {
            const std::string err(e.what());
            res.value = err;
        }

    } break;
    default: {

    } break;
    }

    if (res.type == ws_message::type_t::event_error) {
        return req->create_response(restinio::status_bad_request())
            .set_body(res.to_string())
            .done();
    } else {
        return req->create_response()
            .set_body(res.to_string())
            .done();
    }

    return restinio::request_accepted();
}

void miledger::WsServer::handleRequestMessage(uint64_t recipient, const miledger::ws_message& message) {
    switch (message.type) {
    case ws_message::type_t::action_get_address: {
        if (!m_app->dev.canInteract()) {
            nlohmann::json pl;
            pl["state"] = m_app->dev.getStateString();
            sendErrorMessage(recipient, fmt::format("Can't proceed request: device is in invalid state: {0}", m_app->dev.getStateString()), pl);
            return;
        }
        m_app->dev.getAddress(true)
            .observe_on(RxQt::get().ioThread())
            .subscribe_on(RxQt::get().ioThread())
            .subscribe(
                [this, recipient](minter::address_t address) {
                    miledger::ws_message res(ws_message::type_t::result_get_address, address.to_string());
                    sendMessage(recipient, res);
                },
                [this, recipient](std::exception_ptr eptr) {
                    miledger::ws_message res;
                    res.value = miledger::utils::getError(eptr).toStdString();
                    sendMessage(recipient, res);
                });
        return;
    }
    case ws_message::type_t::action_get_device_state: {
        miledger::ws_message res(ws_message::type_t::result_get_device_state, m_app->dev.getStateString());
        sendMessage(recipient, res);
        return;
    }

    case ws_message::type_t::action_sign_tx: {
        if (!m_app->dev.canInteract()) {
            nlohmann::json pl;
            pl["state"] = m_app->dev.getStateString();
            sendErrorMessage(recipient, fmt::format("Can't proceed request: device is in invalid state: {0}", m_app->dev.getStateString()), pl);
            return;
        }

        if (message.value.empty()) {
            sendErrorMessage(recipient, "Unable to sign tx: empty value. Value must be a hex-string with 32 bytes of raw unsigned tx data");
            return;
        }
        if (message.value.size() != 64) {
            sendErrorMessage(recipient, "Unable to sign tx: invalid raw tx length. Value must be a hex-string with 32 bytes of raw unsigned tx data");
            return;
        }
        tb::bytes_data rawTx(message.value);
        minter::signature signature;
        try {
            sendStatusMessage(recipient, ws_message::type_t::event_user_action_required);
            signature = m_app->dev.signTx(rawTx);

            ws_message msg(ws_message::type_t::event_user_action_result);
            msg.value = "success";
            msg.payload["status_code"] = CODE_SUCCESS;
            sendMessage(recipient, msg);

            ws_message signRes(ws_message::type_t::result_sign_tx);
            signRes.payload["r"] = tb::bytes_to_hex(signature.r.data(), signature.r.size());
            signRes.payload["s"] = tb::bytes_to_hex(signature.s.data(), signature.s.size());
            signRes.payload["v"] = tb::bytes_to_hex(signature.v.data(), signature.v.size());
            sendMessage(recipient, signRes);

        } catch (const minter::exchange_error& e) {
            ws_message msg(ws_message::type_t::event_user_action_result);
            msg.value = e.codeString();
            msg.payload["status_code"] = e.code();
            sendMessage(recipient, msg);
            return;
        } catch (const std::exception& e) {
            const std::string err(e.what());
            sendErrorMessage(recipient, err);
            return;
        }
    } break;

    default:
        break;
        // ignore
    }
}

std::unique_ptr<router_t> miledger::WsServer::requestHandler() {
    auto router = std::make_unique<router_t>();

    router->http_get("/app", [this](std::shared_ptr<restinio::request_t> req, auto) mutable {
        if (req->header().connection() == restinio::http_connection_header_t::upgrade) {
            auto handler = rws::upgrade<traits_t>(
                *req,
                rws::activation_t::immediate,
                [this](rws::ws_handle_t wsh, std::shared_ptr<rws::message_t> m) {
                    switch (m->opcode()) {
                    case rws::opcode_t::text_frame:
                    case rws::opcode_t::binary_frame:
                    case rws::opcode_t::continuation_frame: {
                        miledger::ws_message msg;
                        bool success;
                        msg = miledger::ws_message::parse_message(m, &success);
                        if (!success) {
                            wsh->send_message(msg.to_ws_message());
                            break;
                        }

                        handleRequestMessage(wsh->connection_id(), msg);
                    } break;
                    case rws::opcode_t::ping_frame: {
                        auto resp = *m;
                        resp.set_opcode(rws::opcode_t::pong_frame);
                        wsh->send_message(resp);
                        break;
                    }
                    case rws::opcode_t::connection_close_frame:
                        m_registry.erase(wsh->connection_id());
                        break;
                    case restinio::websocket::basic::opcode_t::pong_frame:
                    case restinio::websocket::basic::opcode_t::unknown_frame:
                        break;
                    }
                });
            m_registry.emplace(handler->connection_id(), handler);
            return restinio::request_accepted();
        }
        return restinio::request_rejected();
    });

    namespace epr = restinio::router::easy_parser_router;

    for (const auto& type : ws_message::action_types) {
        router->http_get("/" + ws_message::type_to_string(type), [type, this](std::shared_ptr<restinio::request_t> req, restinio::router::route_params_t params) {
            return handleHttpRequest(type, req, params);
        });
    }

    router->http_get("/status", [](std::shared_ptr<restinio::request_t> req, auto) {
        return req->create_response().set_body("{\"status\":\"ok\"}").done();
    });

    return router;
}
