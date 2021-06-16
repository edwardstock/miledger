/*!
 * wsserver.
 * Request.h
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef ML_REQUEST_H
#define ML_REQUEST_H

#include "io_container.h"

#include <QNetworkRequest>
#include <QString>
#include <QUrl>
#include <memory>
#include <queue>
#include <sstream>
#include <unordered_map>

namespace miledger {
namespace net {

enum class http_status : unsigned {
    unknown = 0,

    continue_ = 100,
    switching_protocols = 101,

    processing = 102,

    ok = 200,
    created = 201,
    accepted = 202,
    non_authoritative_information = 203,
    no_content = 204,
    reset_content = 205,
    partial_content = 206,
    multi_status = 207,
    already_reported = 208,
    im_used = 226,

    multiple_choices = 300,
    moved_permanently = 301,
    found = 302,
    see_other = 303,
    not_modified = 304,
    use_proxy = 305,
    temporary_redirect = 307,
    permanent_redirect = 308,

    bad_request = 400,
    unauthorized = 401,
    payment_required = 402,
    forbidden = 403,
    not_found = 404,
    method_not_allowed = 405,
    not_acceptable = 406,
    proxy_authentication_required = 407,
    request_timeout = 408,
    conflict = 409,
    gone = 410,
    length_required = 411,
    precondition_failed = 412,
    payload_too_large = 413,
    uri_too_long = 414,
    unsupported_media_type = 415,
    range_not_satisfiable = 416,
    expectation_failed = 417,
    misdirected_request = 421,
    unprocessable_entity = 422,
    locked = 423,
    failed_dependency = 424,
    upgrade_required = 426,
    precondition_required = 428,
    too_many_requests = 429,
    request_header_fields_too_large = 431,
    connection_closed_without_response = 444,
    unavailable_for_legal_reasons = 451,
    client_closed_request = 499,

    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503,
    gateway_timeout = 504,
    http_version_not_supported = 505,
    variant_also_negotiates = 506,
    insufficient_storage = 507,
    loop_detected = 508,
    not_extended = 510,
    network_authentication_required = 511,
    network_connect_timeout_error = 599
};

enum class http_method {
    get,
    post,
    put,
    delete_,
    head
};

class base_request : public miledger::net::io_container {
public:
    using status = http_status;
    /// \brief Http methods
    using method = http_method;

    base_request();
    explicit base_request(const QString& url);
    explicit base_request(const QUrl& url);
    explicit base_request(const QString& url, uint16_t port);
    base_request(const QString& url, base_request::method method);
    virtual ~base_request() = default;

    /// \brief Convert string method name to wss::web::Request::Method
    /// \param method_name case insensitive string
    /// \return if method string will not be recognized, method will return wss::web::Request::Method::GET
    static method method_from_string(const QString& method_name);

    /// \brief Convert method to uppercase string
    /// \param methodName
    /// \return http method name
    static QString method_to_string(method methodName);

    /// \brief Set request http method
    /// \param method
    void set_method(method method);

    /// \brief Explicitly mark to use ssl stream, event if parsed url does not have https protocol
    /// \param use
    void use_ssl(bool use);

    /// \brief Return existed url
    /// \return url string or empty string if did not set
    QString get_url_string() const;

    /// \brief Return existed url
    /// \return url string or empty string if did not set
    QUrl get_url() const;

    /// \brief Return hostname
    /// \return pure hostname without protocol and port: google.com, facebook.com etc
    QString get_host() const;

    /// \brief Set hostname for url
    /// \param hostname example: google.com
    void set_host(const QString& hostname);

    /// \brief Retrun port number
    /// \return Default: 80 if url has http://, if url protocol https without explicit port number - 443,
    /// otherwise - custom port number
    uint16_t get_port() const;

    /// \brief Set port number for url
    /// \param portNumber 16 bit value, range is 0 - 65535
    void set_port(uint16_t portNumber);

    /// \brief Return port number as string
    /// \return the same but string value
    QString get_port_str() const;

    /// \brief Protocol name
    /// \return simple name url started from: http, ftp, or what was passed
    QString get_proto_name() const;

    /// \brief Set protocol name, like http or https or ftp
    /// \param protocolName
    void set_proto_name(const QString& protocolName);

    /// \brief Url path (not a query params!)
    /// \return for example: "search" for url "https://google.com/search"
    QString get_path() const;

    /// \brief Url path with prepared query params
    /// \return for example: "search" for url "https://google.com/search?q=1&c=2.0&etc=bla"
    QString get_path_with_query() const;

    /// \brief Set url path (without query!)
    /// \param path for example: "/api/v1/get-my-money" for url "https://google.com/api/v1/get-my-money"
    void set_path(const QString& path);

    /// \brief Add to existing path new path or just set it
    /// \param path for example: was "/api/v1/", your'e adding "/user/create", result will: /api/v1/user/create
    void add_path(const QString& path);

    //    void add_path(const std::string& path);
    //    void add_path(std::string&& path);

    /// \brief Return Http method name
    /// \return
    method get_method() const;

    /// \brief Return http method name as string
    /// \return string method name, ie GET, POST, etc
    QString get_method_str() const;

    /// \brief Whether call will be with requested with ssl stream or not
    /// \return true if ssl is used
    bool is_ssl() const;

    /// \brief parse query string to vector<KeyValue>. String must not contains hostname or protocol, only query string.
    /// Example: ?id=1&param=2&someKey=3
    /// Warning! Keys represented as arrays, will not be recognized as arrays, they will stored as multiple values, of one keys,
    /// and if you will try to get param only by key using getParam(const QString&), method will return only first found value, not all.
    /// \param query_string
    void parse_query(const QString& query_string);

    /// \brief Check for at least one query parameter has set
    /// \return
    bool has_query() const;

    /// \brief Add query param key-value wss::web::KeyValue. Key can be array! Just set std::pair<QString,QString>("arr[]", "v0")
    /// \param keyValue pair of strings
    void add_query(kv&& keyValue);

    /// \brief Add query param key-value (value double type)
    /// \param keyValue pair of string=>uint64_t
    void add_query(kvd&& keyValue);

    /// \brief Add query param key-value (value double type)
    /// \param keyValue pair of string=>uint64_t
    void add_query(kvf&& keyValue);

    /// \brief Check for parameter exists
    /// \param key query parameter name
    /// \param icase search case sensititvity
    /// \return true if parameter exists
    bool has_query(const QString& key, bool icase = true) const;

    /// \brief Return value of query parameter
    /// \param key query parameter name
    /// \param icase search case sensititvity
    /// \return empty string of parameter did not set
    QString get_query_value(const QString& key, bool icase = true) const;

    /// \brief
    /// \param key
    /// \param icase
    /// \return
    std::optional<miledger::net::kv> find_query(const QString& key, bool icase = true) const;

    /// \brief Set query param with value
    /// \param key query parameter name
    /// \param value parameter value
    void set_query(const QString& key, const QString& value, bool icase = true);

    /// \brief Set query param by key-value pair
    /// \param kv key-value pair
    /// \param icase search case sensititvity
    void set_query(const miledger::net::kv& kv, bool icase = true);

    /// \brief Remove query array paramter by it name and index
    /// Example: to delete first item of: a[]=1&a[]=2&a[]=3
    /// Use: remove_query_array_item("a[]", 0)
    /// OR
    /// remove_query_array_item("A[]", 0, true) to ignore input string case
    /// \param key param name ( !!! with braces: [] )
    /// \param index array index
    /// \param icase search case sensititvity
    /// \return true if was removed, false otherwise
    bool remove_query_array(const QString& key, size_t index = 0, bool icase = true);

    /// \brief Remove query paramter by it name
    /// \param key param name
    /// \param icase use case insensitive search
    /// \return true if was removed, false otherwise
    bool remove_query(const QString& key, bool icase = true);

    /// \brief Remove all existent queries
    void clear_queries();

    /// \brief Get size of total queries
    /// \return
    size_t queries_size() const;

    /// \brief Return multiple entries if param is an array.
    /// For example k[]=k&m[]=k&m=3, will return:
    /// std::vector<QString> out(3);
    /// \param key
    /// \param icase
    /// \return vector of string pairs
    std::vector<QString> get_query_array(const QString& key, bool icase = true) const;

    /// \brief Build passed url with query parameters
    /// \return url with parameters. if url did not set, will return empty string without parameters
    QString get_query_string() const;

    /// \brief Return vector of passed parameters
    /// \return simple vector with pairs of strings
    const kv_vector& get_query_list() const;

    // Use with carefully
    void parse_url(const QString& url);

private:
    bool m_ssl;
    http_method m_method;
    QString m_proto;
    QString m_host;
    QString m_port;
    QString m_path;
    /// \brief like multimap but vector of pairs
    kv_vector m_params;
};

class request : public miledger::net::base_request {
public:
    request()
        : base_request() {
    }
    explicit request(const QString& url)
        : base_request(url) {
    }
    explicit request(const QUrl& url)
        : base_request(url) {
    }
    explicit request(const QString& url, uint16_t port)
        : base_request(url, port) {
    }
    request(const QString& url, base_request::method method)
        : base_request(url, method) {
    }

    QNetworkRequest to_qt_request() const;

    //    void set_body(const QString& body) override;
    //    void set_body(QString&& body) override;
    //    void set_body(const miledger::net::request_body& body);
    //    void set_body(miledger::net::request_body&& body);
};

} // namespace net
} // namespace miledger

#endif // ML_REQUEST_H
