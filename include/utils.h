/*!
 * miledger.
 * utils.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_UTILS_H
#define MILEDGER_UTILS_H

#include <QString>
#include <minter/api/explorer/explorer_results.h>
#include <minter/minter_tx_config.h>

namespace miledger {
namespace utils {

QString humanDecimal(const dev::bigdec18& num, size_t min = 2, size_t max = 8);
QString humanDecimal(const std::string& num, size_t min = 2, size_t max = 8);
QString getRandomString(size_t len = 12, const QString& prefix = "");
QString getError(const std::exception_ptr& eptr);

} // namespace utils
} // namespace miledger

struct enum_hasher {
    template<typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

QDebug operator<<(QDebug debug, const dev::bigint& v);
QDebug operator<<(QDebug debug, const std::string& v);
QDebug operator<<(QDebug debug, const dev::bigdec18& v);
QDebug operator<<(QDebug debug, const minter::explorer::coin_item& v);
QDebug operator<<(QDebug debug, const minter::explorer::coin_item_base& v);
QDebug operator<<(QDebug debug, const toolbox::data::bytes_data& v);

#endif // MILEDGER_UTILS_H
