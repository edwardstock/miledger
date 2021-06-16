/*!
 * miledger.
 * utils.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#include "include/utils.h"

#include <QRandomGenerator>
#include <minter/tx/utils.h>
#include <toolbox/strings/decimal_formatter.h>

QString miledger::utils::humanDecimal(const dev::bigdec18& num, size_t min, size_t max) {
    return humanDecimal(minter::utils::to_string(num), min, max);
}
QString miledger::utils::humanDecimal(const std::string& num, size_t min, size_t max) {
    toolbox::strings::decimal_formatter fmt(num);
    fmt.set_max_precision(max);
    fmt.set_min_precision(min);

    return QString::fromStdString(fmt.format());
}

QString miledger::utils::getRandomString(size_t len, const QString& prefix) {
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

    QString randomString;
    randomString.append(prefix);
    for (size_t i = 0; i < len; ++i) {
        int index = QRandomGenerator::global()->generate() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}
QString miledger::utils::getError(const std::exception_ptr& eptr) {
    try {
        std::rethrow_exception(eptr);
    } catch (const std::exception& e) {
        return QString(e.what());
    }
}
