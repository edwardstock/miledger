/*!
 * miledger.
 * web_server.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_WS_SERVER_H
#define MILEDGER_WS_SERVER_H
#include "include/api/ws_messages.h"
#include "include/console_app.h"
#include "include/miledger-config.h"

#include <QObject>
#include <QThread>
#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>
#include <unordered_map>

namespace rws = restinio::websocket::basic;
namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

using traits_t = restinio::traits_t<
    restinio::asio_timer_manager_t,
    restinio::single_threaded_ostream_logger_t,
    router_t>;

using ws_registry_t = std::unordered_map<std::uint64_t, rws::ws_handle_t>;
using server_t = restinio::http_server_t<traits_t>;
using settings_t = restinio::server_settings_t<traits_t>;

namespace io = restinio::asio_ns;

namespace miledger {

class WsServer : public QObject {
    Q_OBJECT

public:
    WsServer(miledger::ConsoleApp* app, QObject* parent = nullptr);
    ~WsServer();

    virtual bool isRunning() const;

signals:
    void stopped();
    void stopServer();
    void startServer();

public slots:
    void run();
    void stop();

private:
    miledger::ConsoleApp* m_app;
    io::io_context m_ctx;
    server_t* m_server = nullptr;
    ws_registry_t m_registry;
    std::atomic_bool m_isRunning;
    std::atomic_bool m_isStarting;
    std::atomic_bool m_isStopping;

    void cleanupHandler();
    std::unique_ptr<router_t> requestHandler();
    restinio::request_handling_status_t handleHttpRequest(ws_message::type_t type, std::shared_ptr<restinio::request_t> req, const restinio::router::route_params_t& params) const;
    void handleRequestMessage(uint64_t recipient, const miledger::ws_message& message);

    void sendMessage(uint64_t recipient, const miledger::ws_message& message);
    void sendStatusMessage(uint64_t recipient, ws_message::type_t type, const std::string& message = "");
    void sendErrorMessage(uint64_t recipient, const std::string& error);
    void sendErrorMessage(uint64_t recipient, const std::string& error, nlohmann::json payload);
};

} // namespace miledger

#endif // MILEDGER_WS_SERVER_H
