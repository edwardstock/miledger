/*!
 * miledger.
 * io_container.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/net/io_container.h"

#include <QTextStream>

// BASE IO
miledger::net::io_container::io_container()
    : m_body() {
}

void miledger::net::io_container::set_body(const QString& data) {
    m_body = data;
    set_header({"Content-Length", miledger::net::to_string(m_body.length())});
}
void miledger::net::io_container::set_body(QString&& data) {
    m_body = std::move(data);
    set_header({"Content-Length", miledger::net::to_string(m_body.length())});
}
void miledger::net::io_container::set_header(miledger::net::kv&& key_value) {
    bool found = false;
    for (auto& kv : m_headers) {
        if (QString::compare(kv.first, key_value.first, Qt::CaseInsensitive)) {
            kv.second = key_value.second;
            found = true;
        }
    }

    if (!found) {
        return add_header(std::move(key_value));
    }
}
bool miledger::net::io_container::has_header(const QString& name) const {
    for (auto& h : m_headers) {
        if (QString::compare(h.first, name, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

std::optional<miledger::net::kv> miledger::net::io_container::find_header_pair(const QString& name) const {
    std::optional<miledger::net::kv> out;
    for (auto& h : m_headers) {
        if (QString::compare(h.first, name, Qt::CaseInsensitive)) {
            out = h;
            break;
        }
    }

    return out;
}
QString miledger::net::io_container::get_header_value(const QString& headerName) const {
    for (auto& h : m_headers) {
        if (QString::compare(h.first, headerName, Qt::CaseInsensitive)) {
            return h.second;
        }
    }

    return {};
}
bool miledger::net::io_container::cmp_header_value(const QString& header_name, const QString& comparable) const {
    if (!has_header(header_name))
        return false;
    return get_header_value(header_name) == comparable;
}
void miledger::net::io_container::add_header(const QString& name, const QString& value) {

    for (auto& h : m_headers) {
        if (QString::compare(h.first, name, Qt::CaseInsensitive)) {
            h.second = value;
            return;
        }
    }
    m_headers.emplace_back(name.toLower(), value);
}
void miledger::net::io_container::add_header(const miledger::net::kv& kv) {
    add_header(kv.first, kv.second);
}
void miledger::net::io_container::add_header(miledger::net::kv&& kv) {
    for (auto& h : m_headers) {
        if (QString::compare(h.first, kv.first, Qt::CaseInsensitive)) {
            h.second = std::move(kv.second);
            return;
        }
    }
    m_headers.push_back(std::move(kv));
}
void miledger::net::io_container::add_headers(const miledger::net::kv_vector& values) {
    m_headers.reserve(m_headers.size() + values.size());
    std::for_each(values.begin(), values.end(), [this](const miledger::net::kv& pair) {
        add_header(pair.first, pair.second);
    });
}

bool miledger::net::io_container::remove_header(const QString& name, bool icase) {
    bool removed = false;
    size_t i = 0;
    for (auto& h : m_headers) {
        if (icase && QString::compare(h.first, name, Qt::CaseInsensitive)) {
            m_headers.erase(m_headers.begin() + i);
            removed = true;
        } else if (h.first == name) {
            m_headers.erase(m_headers.begin() + i);
            removed = true;
        }

        i++;
    }

    return removed;
}

void miledger::net::io_container::clear_headers() {
    m_headers.clear();
}
size_t miledger::net::io_container::headers_size() const {
    return m_headers.size();
}

void miledger::net::io_container::set_headers(const miledger::net::icase_map_t& map) {
    m_headers.reserve(m_headers.size() + map.size());
    for (auto& h : map) {
        add_header(h.first, h.second);
    }
}
void miledger::net::io_container::set_headers(const miledger::net::icase_multimap_t& mmp) {
    m_headers.reserve(m_headers.size() + mmp.size());
    for (auto& h : mmp) {
        add_header(h.first, h.second);
    }
}
QString miledger::net::io_container::get_body() const {
    return m_body;
}
std::size_t miledger::net::io_container::get_body_size() const {
    return m_body.length();
}

void miledger::net::io_container::clear_body() {
    m_body.clear();
}

bool miledger::net::io_container::has_body() const {
    return !m_body.isEmpty();
}
bool miledger::net::io_container::has_headers() const {
    return !m_headers.empty();
}
const miledger::net::kv_vector& miledger::net::io_container::get_headers() const {
    return m_headers;
}
std::vector<QString> miledger::net::io_container::get_headers_glued() const {
    std::vector<QString> out(m_headers.size());

    QString tmp;
    QTextStream ss(&tmp);
    int i = 0;
    for (const auto& h : m_headers) {
        ss << h.first << ": " << h.second;
        out[i] = ss.readAll();
        tmp.clear();
        ss.reset();
        i++;
    }

    return out;
}
