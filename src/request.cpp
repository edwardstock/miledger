/*!
 * miledger.
 * request.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/net/request.h"

#include <iomanip>
#include <toolbox/strings.hpp>
#include <toolbox/strings/regex.h>

// REQUEST
miledger::net::base_request::base_request()
    : io_container(),
      m_ssl(false),
      m_method(method::get),
      m_proto("http"),
      m_host(""),
      m_port("80"),
      m_path("/") {
}

miledger::net::base_request::base_request(const QString& url)
    : io_container(),
      m_ssl(false),
      m_method(method::get),
      m_proto("http"),
      m_host(""),
      m_port("80"),
      m_path("/") {
    parse_url(url);
}

miledger::net::base_request::base_request(const QUrl& url)
    : io_container(),
      m_ssl(false),
      m_method(method::get),
      m_proto("http"),
      m_host(""),
      m_port("80"),
      m_path("/") {
    parse_url(url.toString());
}

miledger::net::base_request::base_request(const QString& url, uint16_t port)
    : io_container(),
      m_ssl(false),
      m_method(method::get),
      m_proto("http"),
      m_host(""),
      m_path("/") {

    m_port = QString::number(port);
    parse_url(url);
}

miledger::net::base_request::base_request(const QString& url, miledger::net::base_request::method method)
    : io_container(),
      m_ssl(false),
      m_method(method),
      m_proto("http"),
      m_host(""),
      m_port("80"),
      m_path("/") {
    parse_url(url);
}

void miledger::net::base_request::parse_url(const QString& url) {
    QString urlParseRegex =
        R"(([a-zA-Z]+)\:\/\/([a-zA-Z0-9\.\-_]+)?\:?([0-9]{1,5})?(\/[a-zA-Z0-9\/\+\-\.\%\/_]*)?\??([a-zA-Z0-9\-_\+\=\&\%\.]*))";

    if (!toolbox::strings::matches_pattern(urlParseRegex.toStdString(), url.toStdString())) {
        return;
    }

    auto res = toolbox::strings::find_pattern(std::regex(urlParseRegex.toStdString()), url.toStdString());

    m_proto = QString::fromStdString(res[1]);
    m_host = QString::fromStdString(res[2]);
    QString port = QString::fromStdString(res[3]);
    m_path = QString::fromStdString(res[4]);
    QString paramsString = QString::fromStdString(res[5]);

    if (toolbox::strings::equals_icase(m_proto.toStdString(), "https")) {
        m_port = "443";
        m_ssl = true;
    } else if (toolbox::strings::equals_icase(m_proto.toStdString(), "ftp")) {
        m_port = "20";
        m_ssl = false;
    }

    if (!port.isEmpty()) {
        m_port = port;
    }

    if (!paramsString.isEmpty()) {
        auto kvs = toolbox::strings::split(paramsString.toStdString(), '&');
        for (auto& item : kvs) {
            auto kv = toolbox::strings::split_pair(item, '=');
            m_params.push_back({QString::fromStdString(kv.first), QString::fromStdString(kv.second)});
        }
    }
}

void miledger::net::base_request::parse_query(const QString& query_string) {
    QString query = query_string;
    if (query[0] == '?') {
        query = query.chopped(1);
    }

    std::vector<std::string> pairs = toolbox::strings::split(query.toStdString(), "&");

    for (const auto& param : pairs) {
        auto res = toolbox::strings::split_pair(param, "=");
        add_query(std::pair<QString, QString>(
            QString::fromStdString(std::move(res.first)),
            QString::fromStdString(std::move(res.second))));
    }
}

miledger::net::base_request::method miledger::net::base_request::method_from_string(const QString& method_name) {

    if (QString::compare(method_name, "POST", Qt::CaseInsensitive)) {
        return miledger::net::base_request::method::post;
    } else if (QString::compare(method_name, "PUT", Qt::CaseInsensitive)) {
        return method::put;
    } else if (QString::compare(method_name, "DELETE", Qt::CaseInsensitive)) {
        return method::delete_;
    } else if (QString::compare(method_name, "HEAD", Qt::CaseInsensitive)) {
        return method::head;
    }

    return method::get;
}

QString miledger::net::base_request::method_to_string(miledger::net::base_request::method methodName) {
    QString out;

    switch (methodName) {
    case method::post:
        out = "POST";
        break;
    case method::put:
        out = "PUT";
        break;
    case method::delete_:
        out = "DELETE";
        break;
    case method::head:
        out = "HEAD";
        break;
    case method::get:
        out = "GET";
        break;

    default:
        out = "UnsupportedMethod";
    }

    return out;
}

void miledger::net::base_request::set_method(miledger::net::base_request::method method) {
    this->m_method = method;
}

void miledger::net::base_request::add_query(miledger::net::kv&& keyValue) {
    m_params.push_back(std::move(keyValue));
}

void miledger::net::base_request::add_query(kvd&& keyValue) {
    auto tmp = std::move(keyValue);
    QString ss;
    ss.setNum(tmp.second);

    add_query({tmp.first, ss});
}

void miledger::net::base_request::add_query(miledger::net::kvf&& keyValue) {
    auto tmp = std::move(keyValue);
    QString nums;
    nums.setNum(tmp.second, 'g', 7);

    add_query({tmp.first, nums});
}

void miledger::net::base_request::use_ssl(bool use) {
    m_ssl = use;
}

QString miledger::net::base_request::get_url_string() const {
    std::stringstream ss;

    ss << m_proto.toStdString() << "://";
    ss << m_host.toStdString();
    if (m_port != "80" && m_port != "443") {
        ss << ":" << m_port.toStdString();
    }

    if (!m_path.isEmpty()) {
        ss << m_path.toStdString();
    } else {
        ss << "/";
    }

    if (has_query()) {
        ss << get_query_string().toStdString();
    }

    return QString::fromStdString(ss.str());
}

QUrl miledger::net::base_request::get_url() const {
    return QUrl(get_url_string());
}

miledger::net::base_request::method miledger::net::base_request::get_method() const {
    return m_method;
}

QString miledger::net::base_request::get_method_str() const {
    return method_to_string(get_method());
}

bool miledger::net::base_request::has_query() const {
    return !m_params.empty();
}

bool miledger::net::base_request::has_query(const QString& key, bool icase) const {
    const auto& cmp = [icase](const QString& lhs, const QString& rhs) {
        return QString::compare(lhs, rhs, icase ? Qt::CaseInsensitive : Qt::CaseSensitive);
    };

    for (const auto& param : m_params) {
        if (cmp(param.first, key)) {
            return true;
        }
    }

    return false;
}

QString miledger::net::base_request::get_query_value(const QString& key, bool icase) const {
    const auto& cmp = [icase](const QString& lhs, const QString& rhs) {
        return QString::compare(lhs, rhs, icase ? Qt::CaseInsensitive : Qt::CaseSensitive);
    };
    for (const auto& param : m_params) {
        if (cmp(param.first, key)) {
            return param.second;
        }
    }

    return QString();
}

std::optional<miledger::net::kv> miledger::net::base_request::find_query(const QString& key, bool icase) const {
    std::optional<miledger::net::kv> out;
    const auto& cmp = [icase](const QString& lhs, const QString& rhs) {
        return QString::compare(lhs, rhs, icase ? Qt::CaseInsensitive : Qt::CaseSensitive);
    };
    for (const auto& param : m_params) {
        if (cmp(param.first, key)) {
            out = param;
            break;
        }
    }

    return out;
}

void miledger::net::base_request::set_query(const QString& key, const QString& value, bool icase) {
    const auto& cmp = [icase](const QString& lhs, const QString& rhs) {
        return QString::compare(lhs, rhs, icase ? Qt::CaseInsensitive : Qt::CaseSensitive);
    };
    for (auto& param : m_params) {
        if (cmp(param.first, key)) {
            param.second = value;
        }
    }
}

void miledger::net::base_request::set_query(const miledger::net::kv& kv, bool icase) {
    set_query(kv.first, kv.second, icase);
}

bool miledger::net::base_request::remove_query_array(const QString& key, size_t index, bool icase) {
    const auto& cmp = [icase](const QString& lhs, const QString& rhs) {
        return QString::compare(lhs, rhs, icase ? Qt::CaseInsensitive : Qt::CaseSensitive);
    };

    size_t foundCount = 0;
    size_t erasedItems = 0;

    auto it = m_params.begin();
    while (it != m_params.end()) {
        if (cmp(it->first, key)) {
            if (foundCount == index) {
                it = m_params.erase(it);
                erasedItems++;
            } else {
                ++it;
            }
            foundCount++;
        } else {
            ++it;
        }
    }

    return erasedItems > 0;
}

bool miledger::net::base_request::remove_query(const QString& key, bool icase) {
    const auto& cmp = [icase](const QString& lhs, const QString& rhs) {
        return QString::compare(lhs, rhs, icase ? Qt::CaseInsensitive : Qt::CaseSensitive);
    };

    int foundCount = 0;

    auto it = m_params.begin();
    while (it != m_params.end()) {
        if (cmp(it->first, key)) {
            it = m_params.erase(it);
            foundCount++;
        } else {
            ++it;
        }
    }

    return foundCount > 0;
}

void miledger::net::base_request::clear_queries() {
    m_params.clear();
}

size_t miledger::net::base_request::queries_size() const {
    return m_params.size();
}

std::vector<QString> miledger::net::base_request::get_query_array(const QString& key, bool icase) const {
    std::vector<QString> out;

    const auto& cmp = [icase](const QString& lhs, const QString& rhs) {
        return QString::compare(lhs, rhs, icase ? Qt::CaseInsensitive : Qt::CaseSensitive);
    };

    for (const auto& param : m_params) {
        if (cmp(param.first, key)) {
            out.push_back(param.second);
        }
    }

    return out;
}

QString miledger::net::base_request::get_query_string() const {
    std::string combined;
    if (!m_params.empty()) {
        std::stringstream ss;
        std::vector<std::string> enc;
        for (auto& p : m_params) {
            ss << p.first.toStdString() << "=" << p.second.toStdString();
            enc.push_back(ss.str());
            ss.str("");
            ss.clear();
        }
        std::string glued = toolbox::strings::glue("&", enc);
        combined += "?" + glued;
    }

    return QString::fromStdString(combined);
}

const miledger::net::kv_vector& miledger::net::base_request::get_query_list() const {
    return m_params;
}

QString miledger::net::base_request::get_host() const {
    return m_host;
}

void miledger::net::base_request::set_host(const QString& hostname) {
    m_host = hostname;
}

uint16_t miledger::net::base_request::get_port() const {
    const int port = m_port.toInt();
    return (uint16_t) port;
}

void miledger::net::base_request::set_port(uint16_t portNumber) {
    m_port = QString::number(portNumber);
}

QString miledger::net::base_request::get_port_str() const {
    return m_port;
}

QString miledger::net::base_request::get_proto_name() const {
    return m_proto;
}

void miledger::net::base_request::set_proto_name(const QString& protocolName) {
    m_proto = protocolName;
}

QString miledger::net::base_request::get_path() const {
    if (m_path.isEmpty()) {
        return "/";
    }
    return m_path;
}

QString miledger::net::base_request::get_path_with_query() const {
    return get_path() + get_query_string();
}

void miledger::net::base_request::set_path(const QString& path) {
    if (m_path.length() == 0) {
        m_path = "/";
    } else if (m_path.length() > 0 && m_path[0] != '/') {
        m_path = "/" + m_path;
    }

    if (path.length() > 1 && path[0] != '/') {
        m_path = "/" + path;
    } else {
        m_path = path;
    }
}

void miledger::net::base_request::add_path(const QString& path) {
    if (path.length() == 0 || (path.length() == 1 && path[0] == '/')) {
        return;
    }

    if (m_path.length() == 0) {
        m_path = "/";
    }

    if (path[0] == '/') {
        if (m_path[m_path.length() - 1] == '/') {
            m_path += path.chopped(1);
        } else {
            m_path += path;
        }
    } else {
        if (m_path[m_path.length() - 1] != '/') {
            m_path += '/' + path;
        } else {
            m_path += path;
        }
    }
}

// void miledger::net::base_request::add_path(const std::string& path) {
//     add_path(QString::fromStdString(path));
// }
//
// void miledger::net::base_request::add_path(std::string&& p) {
//     QString path = QString::fromStdString(std::move(p));
//     if (path.length() == 0 || (path.length() == 1 && path[0] == '/')) {
//         return;
//     }
//
//     if (m_path.length() == 0) {
//         m_path = "/";
//     }
//
//     if (path[0] == '/') {
//         if (m_path[m_path.length() - 1] == '/') {
//             m_path += path.chopped(1);
//         } else {
//             m_path += path;
//         }
//     } else {
//         if (m_path[m_path.length() - 1] != '/') {
//             m_path += '/' + path;
//         } else {
//             m_path += path;
//         }
//     }
// }

bool miledger::net::base_request::is_ssl() const {
    return m_ssl;
}

QNetworkRequest miledger::net::request::to_qt_request() const {
    QNetworkRequest req;
    req.setUrl(get_url());
    for (const auto& h : m_headers) {
        req.setRawHeader(h.first.toUtf8(), h.second.toUtf8());
    }
    return req;
}
