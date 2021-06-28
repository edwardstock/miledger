/*!
 * miledger.
 * utils.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#include "include/utils.h"

#include <QDebug>
#include <QDebugStateSaver>
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

QDebug operator<<(QDebug debug, const dev::bigint& v) {
    QDebugStateSaver saver(debug);
    debug.nospace() << QString::fromStdString(minter::utils::to_string(v));
    return debug;
}
QDebug operator<<(QDebug debug, const std::string& v) {
    QDebugStateSaver saver(debug);
    debug.nospace() << QString::fromStdString(v);
    return debug;
}
QDebug operator<<(QDebug debug, const dev::bigdec18& v) {
    QDebugStateSaver saver(debug);
    debug.nospace() << miledger::utils::humanDecimal(v, 2, 18);
    return debug;
}
QDebug operator<<(QDebug debug, const minter::explorer::coin_item& v) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "Coin{id=" << v.id << " symbol=" << QString::fromStdString(v.symbol) << " type=" << v.type << "}";
    return debug;
}
QDebug operator<<(QDebug debug, const minter::explorer::coin_item_base& v) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "Coin{id=" << v.id << " symbol=" << QString::fromStdString(v.symbol) << " type=" << v.type << "}";
    return debug;
}
QDebug operator<<(QDebug debug, const toolbox::data::bytes_data& v) {
    QDebugStateSaver saver(debug);
    debug.nospace() << QString::fromStdString(v.to_hex());
    return debug;
}
