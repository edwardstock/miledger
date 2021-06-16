/*!
 * miledger.
 * gate_repo.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_GATE_REPO_H
#define MILEDGER_GATE_REPO_H

#include "include/miledger-config.h"
#include "repository.h"

#include <minter/api/gate/gate_results.h>
#include <minter/tx/tx.h>
#include <rxcpp/rx-observable.hpp>

namespace miledger {
namespace repo {

static dev::bigdec18 get_tx_fee_by_type(minter::gate::price_commissions tx_fees, minter::tx_type_val type);

struct tx_init_data {
    dev::bigint nonce;
    dev::bigint gas = dev::bigint("1");
    dev::bigint gas_coin = minter::def_coin_id;
    minter::explorer::coin_item_base gas_representing_coin = minter::explorer::coin_item_base{
        minter::def_coin_id,
        std::string(MINTER_DEFAULT_COIN),
        minter::explorer::coin_type::coin};
    dev::bigdec18 gas_base_coin_rate = dev::bigdec18("1");
    minter::gate::price_commissions tx_fees;

    QString calc_fee_text(minter::tx_type_val tx_type, size_t payload_len = 0) const;
    QString calc_fee_swap_text(minter::tx_type_val tx_type, unsigned extra_coins_count = 0) const;
};

class gate_repo : public miledger::net::repository {
public:
    gate_repo();
    QUrl get_base_url() const override;

    TASK_RES_ROOT(minter::gate::gas_value)
    get_min_gas();
    TASK_RES_ROOT(minter::gate::tx_count_value)
    get_tx_count(const minter::address_t& address);
    TASK_RES_ROOT(minter::gate::price_commissions)
    get_price_commissions();

    TASK_RES_ROOT(minter::gate::commission_value)
    get_tx_commission_value(const dev::bytes_data& tx_sign);

    TASK_RES_ROOT(minter::gate::commission_value)
    get_base_tx_fee();

    TASK_RES_ROOT(minter::gate::exchange_buy_value)
    get_exchange_buy_currency(const std::string& coin_to_sell, const dev::bigdec18& value_to_buy, const std::string& coin_to_buy);
    TASK_RES_ROOT(minter::gate::exchange_sell_value)
    get_exchange_sell_currency(const std::string& coin_to_sell, const dev::bigdec18& value_to_sell, const std::string& coin_to_buy);
    TASK_RES_ROOT(minter::gate::tx_send_result)
    send_tx(const dev::bytes_data& tx_sign);

    /// \brief Get initial data for transaction construction: nonce and gas price
    /// \param address address to fetch values
    /// \param error pass bool value to check error
    /// \return non
    TASK_RES_ROOT(tx_init_data)
    get_tx_init_data(const minter::address_t& address);
};

} // namespace repo
} // namespace miledger

#endif // MILEDGER_GATE_REPO_H
