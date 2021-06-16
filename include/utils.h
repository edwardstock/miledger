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
#include <minter/minter_tx_config.h>

namespace miledger {
namespace utils {

QString humanDecimal(const dev::bigdec18& num, size_t min = 2, size_t max = 8);
QString humanDecimal(const std::string& num, size_t min = 2, size_t max = 8);
QString getRandomString(size_t len = 12, const QString& prefix = "");
QString getError(const std::exception_ptr& eptr);

} // namespace utils
} // namespace miledger

#endif // MILEDGER_UTILS_H
