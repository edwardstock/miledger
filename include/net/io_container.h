/*!
 * httb.
 * io_container.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef ML_IO_CONTAINER_H
#define ML_IO_CONTAINER_H

#include "include/optional.hpp"

#include <QCryptographicHash>
#include <QDataStream>
#include <QPair>
#include <QString>
#include <unordered_map>
#include <vector>

namespace miledger {
namespace net {

/// \brief Simple std::pair<QString, QString>
using kv = QPair<QString, QString>;
using kvd = QPair<QString, uint64_t>;
using kvf = QPair<QString, double>;

/// \brief Simple vector of pairs wss::web::KeyValue
using kv_vector = std::vector<kv>;

inline QString to_string(size_t n) {
    return QString::number(n);
}

inline bool case_insensitive_equal(const QString& str1, const QString& str2) noexcept {
    return str1.size() == str2.size() && QString::compare(str1, str2, Qt::CaseInsensitive);
}
class icase_equal_t {
public:
    bool operator()(const QString& str1, const QString& str2) const noexcept {
        return case_insensitive_equal(str1, str2);
    }
};

// Based on https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/2595226#2595226
class icase_hash_t {
public:
    std::size_t operator()(const QString& str) const noexcept {
        QByteArray hash = QCryptographicHash::hash(
            QByteArray::fromRawData((const char*) str.utf16(), str.length() * 2),
            QCryptographicHash::Md5);
        Q_ASSERT(hash.size() == 16);

        QDataStream stream(&hash, QIODeviceBase::OpenModeFlag::ReadOnly);
        qint64 a, b;
        stream >> a >> b;
        return a ^ b;
    }
};

using icase_multimap_t = std::unordered_multimap<QString,
                                                 QString,
                                                 icase_hash_t,
                                                 icase_equal_t>;

using icase_map_t = std::unordered_map<QString,
                                       QString,
                                       icase_hash_t,
                                       icase_equal_t>;

class io_container {
public:
    io_container();

    /// \brief Set request body data
    /// \param data string data for request/response
    virtual void set_body(const QString& data);

    /// \brief Move request body data
    /// \param data string data for request/response
    virtual void set_body(QString&& data);

    /// \brief Set header. Overwrites if already contains
    /// \param key_value std::pair<QString, QString>
    void set_header(miledger::net::kv&& key_value);

    /// \brief Adds from map new headers values, if some key exists, value will overwrited
    /// \see add_header(const KeyValue&)
    /// \param map unorderd_map
    void set_headers(const miledger::net::icase_map_t& map);

    /// \brief Adds from map new headers values, if some key exists, value will overwrited
    /// \see add_header(const KeyValue&)
    /// \param mmp
    void set_headers(const miledger::net::icase_multimap_t& mmp);

    /// \brief Check for header keys exists
    /// \param name header name. Searching is case insensitive
    /// \return true is key exists
    bool has_header(const QString& name) const;

    /// \brief Search for header and return row as pair: wss::web::KeyValue
    /// \param name string. Searching is case insensitive
    /// \return pair wss::web::KeyValue
    optns::optional<miledger::net::kv> find_header_pair(const QString& name) const;

    /// \brief Search for header and return it value
    /// \param headerName string. Searching is case insensitive
    /// \return empty string if not found, otherwise copy of origin value
    QString get_header_value(const QString& headerName) const;

    /// \brief Search for header and compare it value with comparable string
    /// \param header_name string. Searching is case insensitive
    /// \param comparable string to compare with
    /// \return true if header found and equals to comparable, false otherwise
    bool cmp_header_value(const QString& header_name, const QString& comparable) const;

    /// \brief Add header with separate key and value strings. If header exists, value will be ovewrited.
    /// \param key string. Value will be writed in original case
    /// \param value any string
    void add_header(const QString& key, const QString& value);

    /// \brief Add header with pair of key and value. If header exists, value will be ovewrited by new value.
    /// \see wss::web::KeyValue
    /// \param kv
    void add_header(const miledger::net::kv& kv);

    /// \brief Move input pair wss::web::KeyValue to header map. If header exists, value will be ovewrited.
    /// \param kv
    /// \see wss::web::KeyValue
    void add_header(kv&& kv);

    /// \brief Add headers collection of key and value. If some header exists, value wil be overwrited by new value
    /// \@see KeyValueVector
    /// \param values
    void add_headers(const kv_vector& values);

    /// \brief Remove header by it's name
    /// \param name header name in any case you want
    /// \param icase true - case insensitive search
    /// \return true if was removed, false - if not
    bool remove_header(const QString& name, bool icase = true);

    /// \brief Remove all existent headers
    void clear_headers();

    /// \brief Get size of headers
    /// \return
    size_t headers_size() const;

    /// \brief Get copy of request/response body
    /// \return Copy of body
    virtual QString get_body() const;

    /// \brief Check for body is not empty
    /// \return true if !body.empty()
    virtual bool has_body() const;

    /// \brief
    /// \return Return body length in bytes
    virtual std::size_t get_body_size() const;

    virtual void clear_body();

    /// \brief Check for header map has at least one value
    /// \return true if map not empty
    bool has_headers() const;

    /// \brief Return copy of header map
    /// \see wss::web::KeyValueVector
    /// \see wss::web::keyValue
    /// \return simple vector of pairs std::vector<KeyValue>
    const miledger::net::kv_vector& get_headers() const;

    /// \brief Glue headers and return list of its.
    /// \return vector of strings:
    /// Example:
    /// {
    ///     "Content-Type: application/json",
    ///     "Connection: keep-alive"
    /// }
    std::vector<QString> get_headers_glued() const;

protected:
    miledger::net::kv_vector m_headers;
    QString m_body;
};

} // namespace net
} // namespace miledger

#endif // HTTB_IO_CONTAINER_H
