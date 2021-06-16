/*!
 * miledger.
 * exchange_calculator.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_EXCHANGE_CALCULATOR_H
#define MILEDGER_EXCHANGE_CALCULATOR_H

#include "include/net/explorer_repo.h"
#include "utils.h"

#include <QObject>
#include <minter/api/explorer/explorer_results.h>
#include <minter/api/gate/gate_results.h>
#include <minter/minter_tx_config.h>
#include <rxcpp/rx-subscriber.hpp>
#include <sstream>
#include <toolbox/strings/decimal_formatter.h>

namespace miledger {

struct EstimateResult {
    dev::bigdec18 amount;
    dev::bigdec18 fee;
    minter::gate::estimate_swap_from swap_from;
    minter::explorer::pool_route route;
    QString error_message;

    QString formatAmountToTarget(const std::string& coin) const {
        toolbox::strings::decimal_formatter fmt(minter::utils::to_string(amount));
        fmt.set_max_precision(8);

        return QString("≈ %1 %2").arg(
            QString::fromStdString(fmt.format()), QString::fromStdString(coin));
    }

    QString formatTargetRate(const QString& amountInput, const minter::explorer::coin_item_base& sourceCoin) {
        dev::bigdec18 amountVal(amountInput.toStdString());
        dev::bigdec18 rate = amount / amountVal;
        return QString("%1 %2")
            .arg(miledger::utils::humanDecimal(rate), QString::fromStdString(sourceCoin.symbol));
    }
    QString formatSourceRate(const QString& amountInput, const minter::explorer::coin_item_base& targetCoin) {
        dev::bigdec18 amountVal(amountInput.toStdString());
        dev::bigdec18 rate = amountVal / amount;
        return QString("%1 %2")
            .arg(miledger::utils::humanDecimal(rate), QString::fromStdString(targetCoin.symbol));
    }

    unsigned getPoolsExtraCoinsSize() {
        return route.coins.size() < 3 ? 0 : route.coins.size() - 2;
    }

    bool exchangeViaPools() {
        return swap_from == minter::gate::estimate_swap_from::pool;
    }

    QString formatSwapType() {
        if (swap_from == minter::gate::estimate_swap_from::pool) {
            std::stringstream ss;
            for (size_t i = 0; i < route.coins.size(); i++) {
                ss << route.coins.at(i).symbol;
                if (i < route.coins.size() - 1) {
                    ss << " > ";
                }
            }
            return QString("%1 %2").arg(QObject::tr("Pools: "), QString::fromStdString(ss.str()));
        } else {
            std::stringstream ss;
            ss << route.coins[0].symbol;
            ss << " > ";
            ss << route.coins[route.coins.size() - 1].symbol;
            return QString("%1 %2").arg(QObject::tr("Reserves: "), QString::fromStdString(ss.str()));
        }
    }
};

class ExchangeCalculator : public QObject {
    Q_OBJECT

public:
    ExchangeCalculator(
        minter::explorer::coin_item* from,
        minter::explorer::coin_item* to,
        dev::bigdec18* amount,
        bool buyCoins)
        : fromCoin(from),
          toCoin(to),
          amount(amount),
          buy(buyCoins) {
    }

    rxcpp::observable<EstimateResult> calculate() const {
        namespace exp = minter::explorer;
        namespace gt = minter::gate;
        return explorerRepo.get_pool_estimate(*fromCoin, *toCoin, *amount, buy ? miledger::repo::pool_swap_type::buy : miledger::repo::pool_swap_type::sell)
            .map([&](exp::pool_route route) {
                EstimateResult res;
                if (!route.is_ok()) {
                    throw std::runtime_error(QString("[%1] %2")
                                                 .arg(QString::number(route.error.code), QString::fromStdString(route.error.message))
                                                 .toStdString());
                } else {
                    if (buy) {
                        res.amount = route.amount_in;
                    } else {
                        res.amount = route.amount_out;
                    }

                    res.swap_from = route.swap_type;
                    res.route = route;

                    // workaround for bancor coins
                    if (!res.exchangeViaPools()) {
                        res.route.coins.push_back(*fromCoin);
                        res.route.coins.push_back(*toCoin);
                    }
                    return res;
                }
            });
    }

private:
    minter::explorer::coin_item* fromCoin;
    minter::explorer::coin_item* toCoin;
    dev::bigdec18* amount;
    bool buy;
    miledger::repo::explorer_repo explorerRepo;
};

} // namespace miledger

#endif // MILEDGER_EXCHANGE_CALCULATOR_H
