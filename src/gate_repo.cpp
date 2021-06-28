/*!
 * miledger.
 * gate_repo.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/net/gate_repo.h"

#include "include/miledger-config.h"
#include "include/utils.h"

#include <minter/api/gate/gate_results.h>
#include <minter/tx/tx.h>
#include <minter/tx/tx_builder.h>
#include <minter/tx/tx_send_coin.h>

using namespace minter::gate;

dev::bigdec18 miledger::repo::get_tx_fee_by_type(minter::gate::price_commissions tx_fees, minter::tx_type_val type) {
    dev::bigdec18 out;
    switch (type) {
    case minter::tx_type_val::send_coin:
        out = minter::utils::humanize_value(tx_fees.send);
        break;
    case minter::tx_type_val::sell_coin:
        out = minter::utils::humanize_value(tx_fees.sell_bancor);
        break;
    case minter::tx_type_val::sell_all_coins:
        out = minter::utils::humanize_value(tx_fees.sell_all_bancor);
        break;
    case minter::tx_type_val::buy_coin:
        out = minter::utils::humanize_value(tx_fees.buy_bancor);
        break;
    case minter::tx_type_val::create_coin:
        out = minter::utils::humanize_value(tx_fees.create_coin);
        break;
    case minter::tx_type_val::declare_candidacy:
        out = minter::utils::humanize_value(tx_fees.declare_candidacy);
        break;
    case minter::tx_type_val::delegate:
        out = minter::utils::humanize_value(tx_fees.delegate);
        break;
    case minter::tx_type_val::unbond:
        out = minter::utils::humanize_value(tx_fees.unbond);
        break;
    case minter::tx_type_val::redeem_check:
        out = minter::utils::humanize_value(tx_fees.redeem_check);
        break;
    case minter::tx_type_val::set_candidate_on:
        out = minter::utils::humanize_value(tx_fees.set_candidate_on);
        break;
    case minter::tx_type_val::set_candidate_off:
        out = minter::utils::humanize_value(tx_fees.set_candidate_off);
        break;
    case minter::tx_type_val::create_multisig:
        out = minter::utils::humanize_value(tx_fees.create_multisig);
        break;
    case minter::tx_type_val::multisend:
        out = minter::utils::humanize_value(tx_fees.multisend_base);
        break;
    case minter::tx_type_val::edit_candidate:
        out = minter::utils::humanize_value(tx_fees.edit_candidate);
        break;

    case minter::tx_type_val::set_halt_block:
        out = minter::utils::humanize_value(tx_fees.set_halt_block);
        break;
    case minter::tx_type_val::recreate_coin:
        out = minter::utils::humanize_value(tx_fees.recreate_coin);
        break;
    case minter::tx_type_val::edit_coin_owner:
        out = minter::utils::humanize_value(tx_fees.edit_ticker_owner);
        break;
    case minter::tx_type_val::edit_multisig:
        out = minter::utils::humanize_value(tx_fees.edit_multisig);
        break;
    case minter::tx_type_val::edit_candidate_public_key:
        out = minter::utils::humanize_value(tx_fees.edit_candidate_public_key);
        break;

    case minter::tx_type_val::add_liquidity:
        out = minter::utils::humanize_value(tx_fees.add_liquidity);
        break;
    case minter::tx_type_val::remove_liquidity:
        out = minter::utils::humanize_value(tx_fees.remove_liquidity);
        break;
    case minter::tx_type_val::sell_swap_pool:
        out = minter::utils::humanize_value(tx_fees.sell_pool_base);
        break;
    case minter::tx_type_val::buy_swap_pool:
        out = minter::utils::humanize_value(tx_fees.buy_pool_base);
        break;
    case minter::tx_type_val::sell_all_swap_pool:
        out = minter::utils::humanize_value(tx_fees.sell_all_pool_base);
        break;
    case minter::tx_type_val::edit_candidate_commission:
        out = minter::utils::humanize_value(tx_fees.edit_candidate_commission);
        break;
    case minter::tx_type_val::mint_token:
        out = minter::utils::humanize_value(tx_fees.mint_token);
        break;
    case minter::tx_type_val::burn_token:
        out = minter::utils::humanize_value(tx_fees.burn_token);
        break;
    case minter::tx_type_val::create_token:
        out = minter::utils::humanize_value(tx_fees.create_token);
        break;
    case minter::tx_type_val::recreate_token:
        out = minter::utils::humanize_value(tx_fees.recreate_token);
        break;
    case minter::tx_type_val::vote_commission:
        out = minter::utils::humanize_value(tx_fees.vote_commission);
        break;
    case minter::tx_type_val::vote_update:
        out = minter::utils::humanize_value(tx_fees.vote_update);
        break;
    case minter::tx_type_val::create_swap_pool:
        out = minter::utils::humanize_value(tx_fees.create_swap_pool);
        break;

    default:
        out = dev::bigdec18("1");
    }
    return out;
}

QString miledger::repo::tx_init_data::calc_fee_text(minter::tx_type_val tx_type, size_t payload_len) const {
    auto payloadFee = minter::utils::humanize_value(dev::bigint(payload_len) * tx_fees.payload_byte);
    auto gasDecimal = dev::bigdec18(gas);
    if (gas_representing_coin.id == minter::def_coin_id) {
        auto fee = get_tx_fee_by_type(tx_fees, tx_type) * gasDecimal + payloadFee;
        return QString("%1 %2")
            .arg(
                miledger::utils::humanDecimal(fee),
                QString::fromStdString(gas_representing_coin.symbol));
    } else {
        auto fee = get_tx_fee_by_type(tx_fees, tx_type) * gasDecimal + payloadFee;
        return QString("%1 %2 (%3 %4)")
            .arg(
                miledger::utils::humanDecimal((fee * gas_base_coin_rate)),
                QString(MINTER_DEFAULT_COIN),
                miledger::utils::humanDecimal(fee),
                QString::fromStdString(gas_representing_coin.symbol));
    }
}

QString miledger::repo::tx_init_data::calc_fee_swap_text(minter::tx_type_val tx_type, unsigned int extra_coins_count) const {
    dev::bigdec18 poolCoinFee("0");
    if (tx_type == minter::tx_type_val::buy_swap_pool) {
        poolCoinFee = minter::utils::humanize_value(tx_fees.buy_pool_delta) * dev::bigdec18(extra_coins_count);
    } else if (tx_type == minter::tx_type_val::sell_swap_pool) {
        poolCoinFee = minter::utils::humanize_value(tx_fees.sell_pool_delta) * dev::bigdec18(extra_coins_count);
    } else if (tx_type == minter::tx_type_val::sell_all_swap_pool) {
        poolCoinFee = minter::utils::humanize_value(tx_fees.sell_all_pool_delta) * dev::bigdec18(extra_coins_count);
    }

    if (gas_representing_coin.id == minter::def_coin_id) {
        auto fee = get_tx_fee_by_type(tx_fees, tx_type) * dev::bigdec18(gas) + poolCoinFee;
        return QString("%1 %2")
            .arg(
                miledger::utils::humanDecimal(fee),
                QString::fromStdString(gas_representing_coin.symbol));
    } else {
        auto fee = get_tx_fee_by_type(tx_fees, tx_type) * dev::bigdec18(gas) + poolCoinFee;
        return QString("%1 %2 (%3 %4)")
            .arg(
                miledger::utils::humanDecimal((fee * gas_base_coin_rate)),
                QString(MINTER_DEFAULT_COIN),
                miledger::utils::humanDecimal(fee),
                QString::fromStdString(gas_representing_coin.symbol));
    }
}

miledger::repo::gate_repo::gate_repo() {
}

TASK_RES_ROOT(gas_value)
miledger::repo::gate_repo::get_min_gas() {
    auto req = create_request();
    req.add_path("min_gas_price");

    return MAKE_TASK_ROOT(gas_value, req);
}

TASK_RES_ROOT(tx_count_value)
miledger::repo::gate_repo::get_tx_count(const minter::address_t& address) {
    auto req = create_request();
    req.add_path("nonce");
    req.add_path(QString::fromStdString(address.to_string()));

    return MAKE_TASK_ROOT(tx_count_value, req);
}

TASK_RES_ROOT(minter::gate::price_commissions)
miledger::repo::gate_repo::get_price_commissions() {
    auto req = create_request();
    req.add_path("price_commissions");

    return MAKE_TASK_ROOT(price_commissions, req);
}

TASK_RES_ROOT(commission_value)
miledger::repo::gate_repo::get_tx_commission_value(const dev::bytes_data& tx_sign) {
    auto req = create_request();
    req.add_path("estimate_tx_commission");
    req.add_path(QString::fromStdString(tx_sign.to_hex()));

    return MAKE_TASK_ROOT(commission_value, req);
}

TASK_RES_ROOT(minter::gate::commission_value)
miledger::repo::gate_repo::get_base_tx_fee() {
    minter::privkey_t pk("1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC");
    minter::address_t addr(pk);
    auto tx_builder = minter::new_tx();
    tx_builder->set_chain_id(MINTER_CHAIN_ID);
    tx_builder->set_gas_coin_id(minter::def_coin_id);
    tx_builder->set_gas_price(dev::bigint("1"));
    auto data = tx_builder->tx_send_coin();
    data->set_coin_id(minter::def_coin_id);
    data->set_value("0");
    data->set_to(addr);
    auto tx = data->build();
    auto sign = tx->sign_single(pk);

    return get_tx_commission_value(sign);
}

TASK_RES_ROOT(exchange_buy_value)
miledger::repo::gate_repo::get_exchange_buy_currency(const std::string& coin_to_sell,
                                                     const dev::bigdec18& value_to_buy,
                                                     const std::string& coin_to_buy) {
    auto req = create_request();
    req.add_path("estimate_coin_buy");
    req.add_query({"coin_id_to_sell", QString::fromStdString(coin_to_sell)});
    req.add_query({"value_to_buy", QString::fromStdString(
                                       minter::utils::to_string(minter::utils::normalize_value(value_to_buy)))});
    req.add_query({"coin_id_to_buy", QString::fromStdString(coin_to_buy)});

    return MAKE_TASK_ROOT(exchange_buy_value, req);
}

TASK_RES_ROOT(exchange_sell_value)
miledger::repo::gate_repo::get_exchange_sell_currency(const std::string& coin_to_sell,
                                                      const dev::bigdec18& value_to_sell,
                                                      const std::string& coin_to_buy) {
    auto req = create_request();
    req.add_path("estimate_coin_sell");
    req.add_query({"coin_id_to_sell", QString::fromStdString(coin_to_sell)});
    req.add_query({"value_to_sell", QString::fromStdString(
                                        minter::utils::to_string(minter::utils::normalize_value(value_to_sell)))});
    req.add_query({"coin_id_to_buy", QString::fromStdString(coin_to_buy)});

    return MAKE_TASK_ROOT(exchange_sell_value, req);
}

TASK_RES_ROOT(tx_send_result)
miledger::repo::gate_repo::send_tx(const dev::bytes_data& tx_sign) {
    auto req = create_request();
    req.set_header({"content-type", "application/json; charset=UTF-8"});
    req.set_method(miledger::net::request::method::post);
    req.add_path("send_transaction");
    {
        nlohmann::json j;
        j["tx"] = tx_sign.to_hex();
        std::stringstream ss;
        ss << j;
        req.set_body(QString::fromStdString(ss.str()));
    }

    return MAKE_TASK_ROOT(tx_send_result, req);
}

TASK_RES_ROOT(miledger::repo::tx_init_data)
miledger::repo::gate_repo::get_tx_init_data(const minter::address_t& address) {
    using namespace minter::gate;

    return get_min_gas()
        .combine_latest([this](gas_value gas, tx_count_value nonce, price_commissions fees) {
            tx_init_data init_data;
            init_data.nonce = (nonce.count + dev::bigint("1"));
            init_data.gas = gas.gas;
            init_data.gas_coin = minter::def_coin_id;
            init_data.gas_representing_coin = fees.coin;
            init_data.gas_base_coin_rate = dev::bigdec18("1");
            init_data.tx_fees = fees;

            if (init_data.gas_representing_coin.id != minter::def_coin_id) {
                auto tx_fee = get_base_tx_fee().as_blocking().first();
                init_data.gas_base_coin_rate = minter::utils::humanize_value(tx_fee.value) / minter::utils::humanize_value(init_data.tx_fees.send);
            }

            return init_data;
        },
                        get_tx_count(address), get_price_commissions());
}

QUrl miledger::repo::gate_repo::gate_repo::get_base_url() const {
    return QUrl(QString(MINTER_GATE_API));
}
