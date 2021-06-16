/*!
 * miledger.
 * repository.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_REPOSITORY_H
#define MILEDGER_REPOSITORY_H

#include "request.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QThread>
#include <cpr/callback.h>
#include <cpr/cpr.h>
#include <fmt/format.h>
#include <map>
#include <minter/api/explorer/explorer_results.h>
#include <minter/api/gate/gate_results.h>
#include <rxcpp/rx-observable.hpp>
#include <rxcpp/rx-subscriber.hpp>
#include <utility>

namespace miledger {
namespace net {

class repository : public QObject {
public:
    virtual QUrl get_base_url() const = 0;
    miledger::net::request create_request() const {
        return miledger::net::request(get_base_url());
    }

    template<class T>
    rxcpp::observable<T> defer_task(miledger::net::request&& req) const {
        return rxcpp::observable<>::create<T>([req](rxcpp::subscriber<T> emitter) {
            cpr::Response resp;
            cpr::Session session;
            session.SetUrl(cpr::Url(req.get_url_string().toStdString()));
            cpr::Header headers;

            //            qDebug() << "Request url: " << req.get_url_string();

#ifdef _MSC_VER
            //@todo: use force winssl
            session.SetVerifySsl(cpr::VerifySsl(false));
#endif

            for (const auto& h : req.get_headers()) {
                headers.insert(std::pair<std::string, std::string>(h.first.toStdString(), h.second.toStdString()));
            }

            switch (req.get_method()) {
            case miledger::net::request::method::get: {
                curl_easy_setopt(session.GetCurlHolder()->handle, CURLOPT_NOSIGNAL, 1);
                resp = session.Get();
            } break;
            case miledger::net::request::method::post:
                session.SetBody(cpr::Body(req.get_body().toStdString()));
                curl_easy_setopt(session.GetCurlHolder()->handle, CURLOPT_NOSIGNAL, 1);
                resp = session.Post();
                break;
            default:
                emitter.on_error(
                    std::make_exception_ptr(
                        std::runtime_error("Unsupported HTTP method")));
                // nothing yet
                break;
            }

            if (resp.error && resp.text.empty()) {
                emitter.on_error(std::make_exception_ptr(
                    std::runtime_error(fmt::format("Unable to proceed request {0}: [{1}] {2}", req.get_url_string().toStdString(), resp.error.code, resp.error.message))));
                return;
            }

            nlohmann::json val;

            try {
                val = nlohmann::json::parse(resp.text);
            } catch (const std::exception& e) {
                emitter.on_error(std::make_exception_ptr(std::runtime_error(e.what())));
                return;
            } catch (...) {
                emitter.on_error(std::current_exception());
                return;
            }

            T res = val.get<T>();

            emitter.on_next(res);
            emitter.on_completed();
        });
    }
};

/// \brief Functor to simplity creating response task.
/// \param type Just pass specified response type WITHOUT base minter::*::result
/// \param req Request object
#define MAKE_TASK(type, req) defer_task<result<type>>(std::move(req))
#define MAKE_TASK_ROOT(type, req) defer_task<type>(std::move(req))

/// \brief Wrapper for result shared pointer
#define TASK_RES(type) rxcpp::observable<result<type>>
#define TASK_RES_ROOT(type) rxcpp::observable<type>

} // namespace net
} // namespace miledger

#endif // MILEDGER_REPOSITORY_H
